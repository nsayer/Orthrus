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
#include <MCI.h>

extern uint32_t millis; // from main.

#define INIT_TIMEOUT (1000UL)

// 50 MHz - It's not strictly speaking kosher to go above 25
// without asking first, but almost no cards nowadays don't
// support it.
#define MCI_CLOCK (50000000UL)
#define INIT_MCI_CLOCK (400000UL)

// 4 bits wide
#define MCI_BUS_WIDTH (4)
#define INIT_MCI_BUS_WIDTH (1)

#define MCI_SLOT (0)

uint32_t volume_size;
uint32_t card_size[2];
uint16_t rca[2];

// This initializes a single card. It'll be called twice, with the AB select line one way
// then the other. This method assumes the cards have JUST been powered up.
static bool do_card_init(bool card) {
	uint32_t resp;
	
	gpio_set_pin_level(AB_SELECT, card); // Select the card

	if (mci_sync_select_device(&MCI_0, MCI_SLOT, INIT_MCI_CLOCK, INIT_MCI_BUS_WIDTH, false) != ERR_NONE) goto error;

	delay_ms(10);
	mci_sync_send_clock(&MCI_0); // send the initialization clock cycles
	delay_ms(10);

	if (!mci_sync_send_cmd(&MCI_0, 0, 0)) // GO_IDLE - reset
		goto error;

	if (!mci_sync_send_cmd(&MCI_0, 8 | MCI_RESP_PRESENT | MCI_RESP_CRC, 0x1aaUL))
		goto error;
	resp = mci_sync_get_response(&MCI_0);
	if ((resp & 0xfff) != 0x1aa)
		goto error;

	uint32_t timeout_start = millis;
	volatile bool ready = false;
	do {
		if (!mci_sync_send_cmd(&MCI_0, 55 | MCI_RESP_PRESENT | MCI_RESP_CRC, 0)) goto error;
		// 0x503... high capacity, max performance and 3.3 volts
		ready = mci_sync_send_cmd(&MCI_0, 41 | MCI_RESP_PRESENT, 0x50300000UL);
		resp = mci_sync_get_response(&MCI_0);
		if (millis - timeout_start > INIT_TIMEOUT) goto error;
	} while(!ready || !(resp & 0x80000000UL));

	uint8_t resp_buf[16];
	if (!mci_sync_send_cmd(&MCI_0, 2 | MCI_RESP_PRESENT | MCI_RESP_136, 0)) goto error;
	mci_sync_get_response_128(&MCI_0, resp_buf); // What do we do with this?
	
	if (!mci_sync_send_cmd(&MCI_0, 3 | MCI_RESP_PRESENT | MCI_RESP_CRC, 0)) goto error;
	resp = mci_sync_get_response(&MCI_0);
	rca[card] = (resp & 0xffff0000UL) >> 16;
	
	if (!mci_sync_send_cmd(&MCI_0, 9 | MCI_RESP_PRESENT | MCI_RESP_136 | MCI_RESP_CRC, rca[card] << 16)) goto error;
	mci_sync_get_response_128(&MCI_0, resp_buf);
	
	card_size[card] = ((uint32_t)(resp_buf[7] & 0x3f)) << 16; // lop off the reserved bytes
	card_size[card] |= ((uint32_t)resp_buf[8]) << 8;
	card_size[card] |= ((uint32_t)resp_buf[9]) << 0;
	card_size[card]++;
	card_size[card] <<= 10;

	// We must perform the switch to 4 bit mode in "selected" state.
	if (!mci_sync_send_cmd(&MCI_0, 7 | MCI_RESP_PRESENT | MCI_RESP_BUSY | MCI_RESP_CRC, rca[card] << 16)) goto error;

	// disconnect the pull-up on DAT3.
	if (!mci_sync_send_cmd(&MCI_0, 55 | MCI_RESP_PRESENT | MCI_RESP_CRC, rca[card] << 16)) goto error;
	if (!mci_sync_send_cmd(&MCI_0, 42 | MCI_RESP_PRESENT | MCI_RESP_CRC, 0)) goto error;

	// Switch to 4 bit mode.
	if (!mci_sync_send_cmd(&MCI_0, 55 | MCI_RESP_PRESENT | MCI_RESP_CRC, rca[card] << 16)) goto error;
	if (!mci_sync_send_cmd(&MCI_0, 6 | MCI_RESP_PRESENT | MCI_RESP_CRC, 2)) goto error;
	if (mci_sync_select_device(&MCI_0, MCI_SLOT, INIT_MCI_CLOCK, MCI_BUS_WIDTH, false) != ERR_NONE) goto error;

	// Switch to high speed (50 MHz)
	if (!mci_sync_send_cmd(&MCI_0, 6 | MCI_RESP_PRESENT | MCI_RESP_CRC, 0x80fffff1)) goto error;
	// Leave us in high speed, 4 bit mode as a side effect.
	if (mci_sync_select_device(&MCI_0, MCI_SLOT, MCI_CLOCK, MCI_BUS_WIDTH, true) != ERR_NONE) goto error;

	mci_sync_send_cmd(&MCI_0, 7, 0);
	
	return true;	
error:
	return false;
	
}

// Call this when two cards are freshly inserted. It will power up the cards and try
// to prepare each for I/O. The caller needs to initialize the crypto themselves if this
// succeeds.
bool init_cards() {
	if (mci_sync_select_device(&MCI_0, MCI_SLOT, INIT_MCI_CLOCK, INIT_MCI_BUS_WIDTH, false) != ERR_NONE) goto error;
	gpio_set_pin_level(CARD_PWR, false); // turn on the power
	delay_ms(10);
	gpio_set_pin_level(CARD_EN, false); // enable the bus
	if (!do_card_init(false)) goto error; // card A
	if (!do_card_init(true)) goto error; // card B
	
	// The last card init step left us in 4 bit mode as a side effect
		
	uint32_t block_count = (card_size[0] > card_size[1])?card_size[1]:card_size[0];
	volume_size = (block_count - 1) << 1;

	return true;
	
error:
	gpio_set_pin_level(CARD_EN, true); // disable the bus
	gpio_set_pin_level(CARD_PWR, true); // turn off the power
	return false;
}

// Call this when a card is detected as removed. It will power down the slots.
bool shutdown_cards() {
	gpio_set_pin_level(CARD_EN, true); // disable the card bus
	gpio_set_pin_level(CARD_PWR, true); // turn off the power
	return true;
}

// These two methods read or write a block from the given physical card slot
// slot A is false, slot "B" is true. buf points to a SECTOR_SIZE length buffer.
// blocknum is the block number on that card - not the volume block
bool readPhysicalBlock(bool card, uint32_t blocknum, uint8_t *buf) {
	gpio_set_pin_level(AB_SELECT, card);

	if (!mci_sync_send_cmd(&MCI_0, 7 | MCI_RESP_PRESENT | MCI_RESP_BUSY | MCI_RESP_CRC, rca[card] << 16)) goto err;

	if (!mci_sync_adtc_start(&MCI_0, 17 | MCI_RESP_PRESENT | MCI_RESP_CRC | MCI_CMD_SINGLE_BLOCK, blocknum, SECTOR_SIZE, 1, true)) goto err;
	if (!mci_sync_start_read_blocks(&MCI_0, buf, 1)) goto err;
	if (!mci_sync_wait_end_of_read_blocks(&MCI_0)) goto err;
	
	mci_sync_send_cmd(&MCI_0, 7, 0); // force de-select
	return true;
err:
	return false;
}

bool writePhysicalBlock(bool card, uint32_t blocknum, volatile uint8_t *buf) {
	gpio_set_pin_level(AB_SELECT, card);

	if (!mci_sync_send_cmd(&MCI_0, 7 | MCI_RESP_PRESENT | MCI_RESP_BUSY | MCI_RESP_CRC, rca[card] << 16)) goto err;
	
	if (!mci_sync_adtc_start(&MCI_0, 24 | MCI_CMD_WRITE | MCI_RESP_PRESENT | MCI_RESP_CRC | MCI_CMD_SINGLE_BLOCK, blocknum, SECTOR_SIZE, 1, true)) goto err;
	if (!mci_sync_start_write_blocks(&MCI_0, buf, 1)) goto err;
	if (!mci_sync_wait_end_of_write_blocks(&MCI_0)) goto err;

	mci_sync_send_cmd(&MCI_0, 7, 0); // force de-select
	return true;
err:
	return false;
}
