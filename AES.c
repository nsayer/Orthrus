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

1. It processes disk blocks with AES counter mode given a BLOCKSIZE-2 sized
nonce. It does this by creating a disk block size buffer of pre-cipher-text
for XORing with the incoming data. It creates the buffer using the DMA
system (channels 0 and 1 alternately) so that as much of the CPU is available
in the foreground as possible.

2. It performs AES CMAC operations. These are used in the key derivation process
as whitening for the entropy generator output, and to derive the WDE key from
the contents of the two key blocks on the cards. CMAC is done completely
in the foreground - no DMA.

*/

#include <avr/io.h>
#include <stdlib.h>
#include <util/atomic.h>
#include "SCSI.h"
#include "AES.h"

volatile uint8_t key[KEYSIZE], pre_ct[VIRTUAL_MEMORY_BLOCK_SIZE];

volatile static uint8_t nonce[BLOCKSIZE];
volatile uint16_t pre_ct_tail, pre_ct_head;

void init_aes(void) {
	AES.CTRL |= AES_RESET_bm;
}

ISR(AES_INT_vect) {
	for(int i = 0; i < BLOCKSIZE; i++) {
		pre_ct[pre_ct_head++] = AES.STATE;
	}
	if (pre_ct_head >= sizeof(pre_ct)) {
		AES.INTCTRL = 0; // disable interrupts
		return;
	}
	if (++nonce[sizeof(nonce) - 1] == 0) ++nonce[sizeof(nonce) - 2];
	for(int i = 0; i < BLOCKSIZE; i++) {
		AES.KEY = key[i];
		AES.STATE = nonce[i];
	}
	AES.CTRL |= AES_START_bm;
}

/*
 * Set up a, AES+DMA mechanism to generate a (disk) block's worth
 * of AES counter mode pre-ciphertext (the stuff you XOR with the
 */
void init_CTR(uint8_t* nonce_in, size_t nonce_length) {
	pre_ct_head = pre_ct_tail = 0;
	// Copy in the nonce, zero-filling first.
        memset((void*)nonce, 0, sizeof(nonce));
	// The last 2 bytes are the counter - force it to remain zero.
        memcpy((void*)nonce, nonce_in, MIN(nonce_length, sizeof(nonce) - 2));

	for(int i = 0; i < BLOCKSIZE; i++) {
		AES.KEY = key[i];
		AES.STATE = nonce[i];
	}
	AES.INTCTRL = AES_INTLVL_MED_gc; //
	AES.CTRL |= AES_START_bm;
}

/*
 * For CMAC, we don't use DMA - we just do a block interactively
 */
void encrypt_ECB(uint8_t *block) {
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

// This is a CMAC constant related to the key size.
#define RB (0x87)

// This is a helper method for CMAC.
static uint8_t leftShiftBlock(uint8_t* ptr, size_t block_length) {
	uint8_t carry = 0;
	for(int i = block_length - 1; i >= 0; i--) {
		uint8_t save_carry = (ptr[i] & 0x80)?1:0;
		ptr[i] <<= 1;
		ptr[i] |= carry;
		carry = save_carry;
	}
	return carry;
}

// perform an AES CMAC signature on the given buffer.
void CMAC(uint8_t *buf, size_t buf_length, uint8_t *sigbuf) {
	uint8_t kn[BLOCKSIZE];
	memset(kn, 0, BLOCKSIZE); // start with 0.
	encrypt_ECB(kn);

	// First, figure out k1. That's a left-shift of "k0",
	// and if the top bit of k0 was 1, we XOR with RB.
	uint8_t save_carry = (kn[0] & 0x80) != 0;
	leftShiftBlock(kn, BLOCKSIZE);
	if (save_carry)
		kn[BLOCKSIZE - 1] ^= RB;

	if (buf_length % BLOCKSIZE) {
		// We must now make k2, which is just the same thing again.
		save_carry = (kn[0] & 0x80) != 0;
		leftShiftBlock(kn, BLOCKSIZE);
		if (save_carry)
			kn[BLOCKSIZE - 1] ^= RB;

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

