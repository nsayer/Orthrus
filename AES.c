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

volatile static uint8_t nonce[BLOCKSIZE], pre_ct[VIRTUAL_MEMORY_BLOCK_SIZE];
static uint16_t pre_ct_tail;
volatile static uint16_t pre_ct_head;

void init_aes(void) {
	AES.CTRL |= AES_RESET_bm;
	// reset and enable the whole DMA system
	DMA.CTRL |= DMA_RESET_bm;
	DMA.CTRL |= DMA_ENABLE_bm;
}

// DMA channel 0 copies the data out to the buffer
ISR(DMA_CH0_vect) {
	// move the head pointer forward another block.
	pre_ct_head += BLOCKSIZE;
	// if that means we're done, then stop.
	if (pre_ct_head >= sizeof(pre_ct)) {
		return;
	}
	// increment the counter
	if (++nonce[sizeof(nonce) - 1] == 0) ++nonce[sizeof(nonce) - 2];
	// Start the nonce bufer copy process, which is an
	// enable and a transfer request (done separately just to be sure).
	DMA.CH1.CTRLA |= DMA_CH_ENABLE_bm;
	DMA.CH1.CTRLA |= DMA_CH_TRFREQ_bm;
}

ISR(DMA_CH1_vect) {
	// enable channel 0
	DMA.CH0.CTRLA |= DMA_CH_ENABLE_bm;
	// and kick off AES
	AES.CTRL |= AES_START_bm;
}

// Why is this not defined?
#ifndef DMA_CH_TRIGSRC_AES_gc
#define DMA_CH_TRIGSRC_AES_gc (0x04<<0)
#endif

/*
 * Set up a, AES+DMA mechanism to generate a (disk) block's worth
 * of AES counter mode pre-ciphertext (the stuff you XOR with the
 * data during encryption or decryption). This will go along with
 * our ISRs to facilitate building the buffer in the background.
 */
void init_CTR(uint8_t* nonce_in, size_t nonce_length) {
	pre_ct_head = pre_ct_tail = 0;
	// Copy in the nonce, zero-filling first.
        memset((void*)nonce, 0, sizeof(nonce));
	// The last 2 bytes are the counter - force it to remain zero.
        memcpy((void*)nonce, nonce_in, MIN(nonce_length, sizeof(nonce) - 2));

	// We assume the key is already loaded. Make sure
	// the rest of the control register is set right
	// NO - redundant.
	//AES.CTRL &= ~(AES_DECRYPT_bm | AES_AUTO_bm | AES_XOR_bm);

	// Set up DMA channel 0 to do 16 byte blocks from AES
	// to our pre_ct, advancing forward every time.
	DMA.CH0.CTRLB |= DMA_CH_TRNINTLVL_HI_gc;
	DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_AES_gc;
	DMA.CH0.TRFCNT = BLOCKSIZE;
	// Going from a single address to a constantly moving forward buffer every time
	DMA.CH1.ADDRCTRL = DMA_CH_SRCRELOAD_NONE_gc | DMA_CH_SRCDIR_FIXED_gc | DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_INC_gc;
	DMA.CH0.REPCNT = 0;
	// Going from the AES state register to our pre_ct buffer
	DMA.CH0.SRCADDR0 = (uint8_t)(AES.STATE >> 0);
	DMA.CH0.SRCADDR1 = 0; //(uint8_t)(AES.STATE >> 8);
	DMA.CH0.SRCADDR2 = 0; //(uint8_t)(AES.STATE >> 16);
	DMA.CH0.DESTADDR0 = (uint8_t)(((uint16_t)pre_ct) >> 0);
	DMA.CH0.DESTADDR1 = (uint8_t)(((uint16_t)pre_ct) >> 8);
	DMA.CH0.DESTADDR2 = 0; //(uint8_t)(((uint16_t)pre_ct) >> 16);

	// Set up DMA channel 1 to do 16 bytes from the nonce buffer to AES,
	// repeating the same thing every time.
	DMA.CH1.CTRLB |= DMA_CH_TRNINTLVL_HI_gc;
	DMA.CH1.TRIGSRC = DMA_CH_TRIGSRC_OFF_gc;
	DMA.CH1.TRFCNT = BLOCKSIZE;
	// Going from the same buffer every time to just a single address
	DMA.CH1.ADDRCTRL = DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTRELOAD_NONE_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH1.REPCNT = 0;
	// Going from our nonce buffer to the AES state register
	DMA.CH1.SRCADDR0 = (uint8_t)(((uint16_t)nonce) >> 0);
	DMA.CH1.SRCADDR1 = (uint8_t)(((uint16_t)nonce) >> 8);
	DMA.CH1.SRCADDR2 = 0; //(uint8_t)(((uint16_t)nonce) >> 16);
	DMA.CH1.DESTADDR0 = (uint8_t)(AES.STATE >> 0);
	DMA.CH1.DESTADDR1 = 0; //(uint8_t)(AES.STATE >> 8);
	DMA.CH1.DESTADDR2 = 0; //(uint8_t)(AES.STATE >> 16);

	// And go!
	DMA.CH1.CTRLA |= DMA_CH_ENABLE_bm;
	DMA.CH1.CTRLA |= DMA_CH_TRFREQ_bm;
}

/*
 * Process a streaming block. This assumes you called init_CTR already.
 * Hopefully you did it "a while ago" and the background process has
 * gotten far enough ahead that the wait never happens.
 */
uint8_t encrypt_CTR_byte(uint8_t data) {
	uint16_t local_head;
	do {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			local_head = pre_ct_head;
		}
	} while (pre_ct_tail == local_head) ; // wait for the data to fill in
	return pre_ct[pre_ct_tail++] ^ data;
}

/*
 * Copy in the key
 */
void setKey(uint8_t* key) {
	for(int i = 0; i < KEYSIZE; i++) {
		AES.KEY = key[i];
	}
}

/*
 * For CMAC, we don't use DMA - we just do a block interactively
 */
static void encrypt_ECB(uint8_t *block) {
	// Assume the key is loaded already.
	for(int i = 0; i < BLOCKSIZE; i++) {
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

