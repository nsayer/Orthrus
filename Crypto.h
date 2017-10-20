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
 * The keyblock on each card looks like this:
 * 00-0F: magic value
 * 10-2F: volume ID
 * 30-4F: key block
 * 50-5F: nonce for the *other* card (only 50-5b actually used)
 * 60: flag - 0 for "A", 1 for "B"
 * 61-1FF: unused
 *
 * To make the volume key, you concatenate the key blocks
 * from card A and B together (A first) and perform an AES CMAC over
 * that with an all-zero key. You then use that result as the key
 * for an AES CMAC over the volume ID. The result of that is
 * the volume key.
 */
bool prepVolume(void);

// call this to clear the keys and nonce when a volume goes offline
void unmountVolume(void);

/*
 * This method initializes a volume by writing a newly created key block for
 * each card.
 *
 * Each card gets the same volume ID, but unique key blocks and nonces (only the
 * first ten bytes of the 16 byte PRNG block are used for the nonce). Finally,
 * one card is marked as "A" and the other as "B".
 *
 * Note that this operation doesn't prevent you from initializing an already
 * initialized volume. This is by design - it very quickly trashes all of the
 * data on the volume (by changing the key out from under the data).
 */
bool initVolume(void);

// These methods are the volume I/O methods. They are synchronous.
// Returns false on error.
bool readVolumeBlock(uint32_t blocknum, uint8_t *buf);
	
bool writeVolumeBlock(uint32_t blocknum, uint8_t *buf);