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


#include <stdint.h>
#include "SCSI.h"

// The block size for AES - this is how many bytes we copy in to
// AES.STATE
#define BLOCKSIZE (16)
// Our key size (128 bits) - this is how many bytes we copy in to
// AES.KEY
#define KEYSIZE (16)

extern volatile uint8_t key[KEYSIZE], pre_ct[VIRTUAL_MEMORY_BLOCK_SIZE];
extern volatile uint16_t pre_ct_head, pre_ct_tail;

/*
 * Process a streaming block. This assumes you called init_CTR already.
 * Hopefully you did it "a while ago" and the background process has
 * gotten far enough ahead that the wait never happens.
 */
static inline uint8_t encrypt_CTR_byte(uint8_t data) ATTR_ALWAYS_INLINE;
static inline uint8_t encrypt_CTR_byte(uint8_t data) {
        uint16_t local_head;
        do {
                ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                        local_head = pre_ct_head;
                }
        } while (pre_ct_tail == local_head) ; // wait for the data to fill in
        return pre_ct[pre_ct_tail++] ^ data;
}

// Make this "always inline," effectively.
#define setKey(k) do { memcpy((uint8_t*)key, (k), sizeof(key)); } while(0);

// set up the AES+DMA subsystem.
void init_aes(void);

// Call this at the very start of a disk transfer (either direction).
// It will set up things so you can call encrypt_CTR_byte()
//  VIRTUAL_MEMORY_BLOCK_SIZE times.
void init_CTR(uint8_t* nonce, size_t nonce_length);

// Call this with each byte read prior to writing. It's misnamed - it
// actually does either encryption or decryption in counter mode.
uint8_t encrypt_CTR_byte(uint8_t data);

// Perform an AES CMAC on the given buffer.
void CMAC(uint8_t *buf, size_t buf_length, uint8_t *sigbuf);

