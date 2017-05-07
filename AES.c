/*

 Copyright 2017 Nicholas W. Sayer

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*

This file uses the ATXmega AES subsystem for WDE for Orthrus. It
has two basic purposes:

1. It processes disk blocks with AES XEX mode given a BLOCKSIZE sized
nonce. It does this by presenting an interface to process BLOCKSIZE
blocks of the disk sector at a time and an initialization method to
prepare for each block's encryption or decryption.

2. It performs AES CMAC operations. These are used in the key derivation process
as whitening for the entropy generator output, and to derive the WDE key from
the contents of the two key blocks on the cards.

*/

#include <avr/io.h>
#include <stdlib.h>
#include <util/atomic.h>
#include "SCSI.h"
#include "AES.h"

// Why is this not defined?
#ifndef DMA_CH_TRIGSRC_AES_gc
#define DMA_CH_TRIGSRC_AES_gc (0x04<<0)
#endif

// This is XORed in with the bottom byte after the bit shift to
// perform a multiply-by-two over GF(128).
#define RB (0x87)

uint8_t tweak[BLOCKSIZE], key[KEYSIZE], dec_key[KEYSIZE], mode;

void init_aes(void) {
	clearKeys();
}

void clearKeys(void) {
	memset(key, 0, sizeof(key));
	memset(dec_key, 0, sizeof(key));
	// a reset will clear out the key memory of the hardware.
	AES.CTRL |= AES_RESET_bm;
	while(AES.CTRL & AES_RESET_bm);
}

// Decryption with our hardware requires
// using a different key than encryption.
// Fortunately, the hardware itself can derive
// the decryption key from the encryption key
// by processing a dummy block.
void generateDecryptKey(void) {
	// Encrypt mode
	AES.CTRL &= ~(AES_DECRYPT_bm);
	// Note that this presumes BLOCKSIZE == KEYSIZE. Look for
	// similar constructions throughout this file if you upgrade
	// to, say, AES-256, with better hardware.
	for(int i = 0; i < BLOCKSIZE; i++) {
		AES.KEY = key[i];
		AES.STATE = 0;
	}
	// GO!
	AES.CTRL |= AES_START_bm;
	// Wait for it to finish
	while(!(AES.STATUS & AES_SRIF_bm)) ;
	// Read out the decryption key
	for(int i = 0; i < BLOCKSIZE; i++) {
		dec_key[i] = AES.KEY;
		AES.STATE; // dummy read - if we don't, chaos ensues.
	}
}

/*
 * Do a single encryption.
 */
static void encrypt_ECB(uint8_t *block) {
	// encrypt mode
	AES.CTRL &= ~(AES_DECRYPT_bm);
	for(int i = 0; i < BLOCKSIZE; i++) {
		AES.KEY = key[i];
		AES.STATE = block[i];
	}
	// GO!
	AES.CTRL |= AES_START_bm;
	// Wait for it to finish
	while(!(AES.STATUS & AES_SRIF_bm)) ;
	// And copy it out
	for(int i = 0; i < BLOCKSIZE; i++) {
		block[i] = AES.STATE;
	}
}

// mode_in is zero for encrypt, non-zero for decrypt
void init_xex(uint8_t *nonce, size_t nonce_len, uint8_t mode_in) {
	// Save the mode for later.
	mode = mode_in;
	// Clear it out
	memset(tweak, 0, sizeof(tweak));
	memcpy(tweak, nonce, MIN(nonce_len, sizeof(tweak)));
	encrypt_ECB(tweak); // encrypt the tweak. We're now ready to go.
}

// This is used both by the CMAC code to build K1 and K2 and
// by the XEX code to build the next tweak value. It works
// because both cases require multiplying a constant by
// 2^n over GF(128), where n counts from 0 upwards. Each
// call to this method raises the exponent by one - in other
// words, it multiplies by 2 within GF(128).
//
// If you understand that, then you're better at this than I am.
static void galois_mult(uint8_t *block, size_t block_len) {
	uint8_t carry = 0;
	for(int i = block_len - 1; i >= 0; i--) {
		uint8_t next_carry = (block[i] & 0x80) != 0;
		block[i] <<= 1;
		block[i] |= carry?1:0;
		carry = next_carry;
	}
	// If the carry from the left shift is 1, then XOR RB in at the bottom
	if (carry)
		block[block_len - 1] ^= RB;
}

void process_xex_block(uint8_t *data) {
	if (mode) {
		AES.CTRL |= AES_DECRYPT_bm;
	} else {
		AES.CTRL &= ~(AES_DECRYPT_bm);
	}
	// on the way into AES, XOR the data with the tweak block.
	for(int i = 0; i < BLOCKSIZE; i++) {
		AES.STATE = data[i] ^ tweak[i];
		AES.KEY = mode?dec_key[i]:key[i];
	}
	AES.CTRL |= AES_START_bm;
	while(!(AES.STATUS & AES_SRIF_bm)) ; // wait for it
	// And on the way out of AES, XOR the data with the tweak again.
	// XEX... get it?
	for(int i = 0; i < BLOCKSIZE; i++) {
		data[i] = AES.STATE ^ tweak[i];
	}
	// now make the next tweak block.
	galois_mult(tweak, sizeof(tweak));
}


// perform an AES CMAC signature on the given buffer.
void CMAC(uint8_t *buf, size_t buf_length, uint8_t *sigbuf) {
	uint8_t kn[BLOCKSIZE];
	memset(kn, 0, BLOCKSIZE); // start with 0.
	encrypt_ECB(kn);

	// K1 is a galois field 128 multiplication-by-2 operation on "k0".
	galois_mult(kn, sizeof(kn));

	if (buf_length % BLOCKSIZE) {
		// We must now make K2, which is just the same thing again.
		galois_mult(kn, sizeof(kn));
	}

	memset(sigbuf, 0, BLOCKSIZE); // start with 0.
	for(int i = 0; i < (((buf_length - 1) / BLOCKSIZE) * BLOCKSIZE); i += BLOCKSIZE) {
		for(int j = 0; j < BLOCKSIZE; j++)
			sigbuf[j] ^= buf[i + j];
		encrypt_ECB(sigbuf); // Now roll the intermediate value through ECB.
	}

	// Now we do the last block.
	uint16_t start = ((buf_length - 1) / BLOCKSIZE) * BLOCKSIZE;
	for(int i = start; i < buf_length; i++)
		sigbuf[i - start] ^= buf[i];

	// If there's any empty bytes at the end, then xor the first "unused" one with
	// a trailer byte.
	if (buf_length - start < BLOCKSIZE)
		sigbuf[buf_length - start] ^= 0x80;

	// Now xor in kn.
	for(int i = 0; i < BLOCKSIZE; i++)
		sigbuf[i] ^= kn[i];

	encrypt_ECB(sigbuf); // Now roll THAT and the result is the answer.
}

