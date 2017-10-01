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


// Reads or writes part of block 0 (up to length). The remainder of
// block zero is ignored on read and filled with 0xff on write.
uint8_t writeKeyBlock(uint8_t card, uint8_t *block, size_t length);
uint8_t readKeyBlock(uint8_t card, uint8_t *block, size_t length);

// Call this at boot. Assumes the GPIO setup has already happened.
void init_spi(void);

// When both cards are detected, call this. It returns 0 on success, at which point the
// host can be notified. It returns non-zero for failure. As a side effect, it
// sets the AES key and the volume_size value.
uint8_t init(void);

// This is set as a side effect of init() (if successful).
extern uint32_t volume_size;

// Call this with the block number. It will read a block (512 bytes) from
// the USB endpoint, encrypt it and write it out to the correct card and physical block.
extern uint8_t volumeWriteBlock(uint32_t blocknum);

// Call this with the block number. It will read a block (512 bytes) from the
// correct card and physical block, decrypt it and write it to the USB endpoint.
extern uint8_t volumeReadBlock(uint32_t blocknum);
