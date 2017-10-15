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


#include <atmel_start.h>
#include <AES.h>

#define MIN(a,b) (((a)>(b))?(b):(a))

// This is XORed in with the bottom byte after the bit shift to
// perform a multiply-by-two over GF(128).
#define RB (0x87)

uint8_t __attribute__((section(".dtcm"))) tweak[BLOCKSIZE];
enum __attribute__((section(".dtcm"))) aes_action mode;

void setKey(uint8_t *key) {
	aes_sync_set_encrypt_key(&CRYPTOGRAPHY_0, key, AES_KEY_128);
	aes_sync_set_decrypt_key(&CRYPTOGRAPHY_0, key, AES_KEY_128);
}

void clearKeys(void) {
	unsigned char blankKey[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	setKey(blankKey);
}

__attribute__((noinline)) __attribute__((section(".itcm"))) void init_xex(uint8_t *nonce, size_t nonce_len, enum aes_action action) {
	mode = action;
	memset(tweak, 0, sizeof(tweak));
	memcpy(tweak, nonce, MIN(nonce_len, sizeof(tweak)));
	aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, tweak, tweak);
}

// This is used both by the CMAC code to build K1 and K2 and
// by the XEX code to build the next tweak value. It works
// because both cases require multiplying a constant by
// 2^n over GF(128), where n counts from 0 upwards. Each
// call to this method raises the exponent by one - in other
// words, it multiplies by 2 within GF(128).
//
// If you understand that, then you're better at this than I am.
__attribute__((noinline)) __attribute__((section(".itcm"))) static void galois_mult(uint8_t *block, size_t block_len) {
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

// Process a BLOCKSIZE byte block of a disk block. This does crypting in-place
__attribute__((noinline)) __attribute__((section(".itcm"))) void process_xex_block(uint8_t *data) {
	// XOR the data with the tweak
	for(int i = 0; i < BLOCKSIZE; i++)
		data[i] ^= tweak[i];
	// then encrypt or decrypt as appropriate
	aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, mode, data, data); // do the encrypt or decrypt
	// XOR it again on the way out. XEX... get it?
	for(int i = 0; i < BLOCKSIZE; i++)
		data[i] ^= tweak[i];
	// now make the next tweak block.
	galois_mult(tweak, sizeof(tweak));
}

// perform an AES CMAC signature on the given buffer.
void CMAC(uint8_t *buf, size_t buf_length, uint8_t *sigbuf) {
	uint8_t kn[BLOCKSIZE];
	memset(kn, 0, BLOCKSIZE); // start with 0.
	aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, kn, kn);
	
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
		aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, sigbuf, sigbuf); // Now roll the intermediate value through ECB.
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

	aes_sync_ecb_crypt(&CRYPTOGRAPHY_0, AES_ENCRYPT, sigbuf, sigbuf); // Now roll THAT and the result is the answer.
}
