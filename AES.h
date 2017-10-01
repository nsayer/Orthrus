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

#define BLOCKSIZE (16)

/*
 * Store the key. The API we use stores separate encrypt and
 * decrypt keys, but we will set them both to the same value.
 */
void setKey(uint8_t *k);

// Wipe out the keys. This protects the user if they pull
// the cards out and leave Orthrus plugged in and unattended.
void clearKeys(void);

// Call this at the start of each block I/O with the nonce value
// and 0 for encrypt, 1 for decrypt.
void init_xex(uint8_t *nonce, size_t nonce_len, uint8_t mode_in);

// Call this with BLOCKSIZE bytes at a time.
void process_xex_block(uint8_t *data);

// Perform an AES CMAC on the given buffer (call setKey() first).
void CMAC(uint8_t *buf, size_t buf_length, uint8_t *sigbuf);
