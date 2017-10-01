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

// This is the size of a disk block in bytes.
#define SECTOR_SIZE (512)

extern uint32_t volume_size;

// Call this when two cards are freshly inserted. It will power up the cards and try
// to prepare each for I/O. The caller needs to initialize the crypto themselves if this
// succeeds.
bool init_cards();

// Call this when a card is detected as removed. It will power down the slots.
bool shutdown_cards();

// These two methods read or write a block from the given physical card slot
// slot "A" is false, slot "B" is true. buf points to a SECTOR_SIZE length buffer.
// blocknum is the block number on that card - not the volume block
bool readPhysicalBlock(bool card, uint32_t blocknum, void* buf);
bool writePhysicalBlock(bool card, uint32_t blocknum, void *buf);
