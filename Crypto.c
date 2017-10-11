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
#include <MCI.h>

static const char *MAGIC = "OrthrusVolumeV01";

static uint8_t nonceA[BLOCKSIZE], nonceB[BLOCKSIZE];

static uint8_t cardswap;

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
bool prepVolume(void) {
	uint8_t volid[32], keyblock[2][32];
	uint8_t blockbuf[SECTOR_SIZE];
	if (!readPhysicalBlock(0, 0, blockbuf)) return false; // card A
	if (memcmp(blockbuf, MAGIC, strlen(MAGIC))) return false; // Wrong magic
	cardswap = blockbuf[96] != 0; // we're swapping if A isn't A
	memcpy(volid, blockbuf + 16, sizeof(volid));
	memcpy(cardswap?keyblock[1]:keyblock[0], blockbuf + 48, sizeof(keyblock[0]));
	memcpy(cardswap?nonceB:nonceA, blockbuf + 80, sizeof(nonceA));
	
	if (!readPhysicalBlock(1, 0, blockbuf)) return false; // card B
	if (memcmp(blockbuf, MAGIC, strlen(MAGIC))) return false; // Wrong magic
	if (memcmp(blockbuf + 16, volid, sizeof(keyblock[0]))) return false; // Wrong vol ID
	if (!((blockbuf[96] != 0) ^ cardswap)) return false; // Must be one A, one B.

	memcpy(cardswap?keyblock[0]:keyblock[1], blockbuf + 48, sizeof(volid));
	memcpy(cardswap?nonceA:nonceB, blockbuf + 80, sizeof(nonceA));
	memset(blockbuf, 0, 16); // save RAM - use the block buf as temp
	setKey(blockbuf); // all zero key.
	CMAC((uint8_t*)keyblock, 64, blockbuf); // send in both key blocks, A first
	setKey(blockbuf);
	CMAC(volid, 32, blockbuf); // with the intermediate key, CMAC the volume ID
	setKey(blockbuf); // And that's our volume key
	return true; // all set!
}

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
bool initVolume(void) {
	uint8_t blockbuf[SECTOR_SIZE];
	
	// Just to make sure we don't leak any sensitive memory content.
	memset(blockbuf, 0, sizeof(blockbuf));

	// Magic
	memcpy(blockbuf, MAGIC, strlen(MAGIC));

	// Fill up the volume ID, the key block and the nonce with random
	rand_sync_read_buf8(&RAND_0, blockbuf + 16, 32 + 32 + 16);
	
	blockbuf[96] = 0; // card A
    if (!writePhysicalBlock(false, 0, blockbuf)) return false;
	
	// Leave the volume ID alone, fill only the key and nonce with new random
	rand_sync_read_buf8(&RAND_0, blockbuf + 48, 32 + 16);
	
	blockbuf[96] = 1; // card B
    if (!writePhysicalBlock(true, 0, blockbuf)) return false;

	// as a side effect, perform a volume prep, which will set the key.
	return prepVolume();
}

// return false for *PHYSICAL* card A or true for B
// During the data transfer, we call process_xex_block() on each BLOCKSIZE bytes as we go.
static bool setupBlockCrypto(uint32_t blocknum, enum aes_action mode) {
	uint8_t cardA = (blocknum & 0x1) == 0;

	uint8_t *nonce = cardA?nonceB:nonceA;
	nonce[12] = (uint8_t)(blocknum >> 24);
	nonce[13] = (uint8_t)(blocknum >> 16);
	nonce[14] = (uint8_t)(blocknum >> 8);
	nonce[15] = (uint8_t)(blocknum >> 0);
	init_xex(nonce, sizeof(nonceA), mode);

	return !(cardA ^ cardswap);
}

bool readVolumeBlock(uint32_t blocknum, uint8_t *buf) {
	gpio_set_pin_level(LED_ACT, true);
	bool card = setupBlockCrypto(blocknum, AES_DECRYPT);
	uint32_t phys_blocknum = (blocknum >> 1) + 1;
	if (!readPhysicalBlock(card, phys_blocknum, (uint8_t*)buf)) {
		gpio_set_pin_level(LED_ACT, false);
		gpio_set_pin_level(LED_ERR, true);
		return false; // ERROR
	}
	for(int i = 0; i < SECTOR_SIZE; i += BLOCKSIZE)
		process_xex_block((uint8_t*)buf + i);
	gpio_set_pin_level(LED_ACT, false);
	gpio_set_pin_level(LED_ERR, false);
	return true;
}
	
bool writeVolumeBlock(uint32_t blocknum, uint8_t *buf) {
	gpio_set_pin_level(LED_ACT, true);
	bool card = setupBlockCrypto(blocknum, AES_ENCRYPT);
	uint32_t phys_blocknum = (blocknum >> 1) + 1;
	for(int i = 0; i < SECTOR_SIZE; i += BLOCKSIZE)
		process_xex_block(buf + i);
	bool out = writePhysicalBlock(card, phys_blocknum, buf);
	gpio_set_pin_level(LED_ACT, false);
	gpio_set_pin_level(LED_ERR, !out);
	return out;
}
