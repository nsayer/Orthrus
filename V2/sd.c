
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
#include <stdlib.h>
#include <avr/io.h>
#include <util/atomic.h>

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Drivers/USB/Class/Device/MassStorageClassDevice.h>

#include "AES.h"
#include "Orthrus.h"
#include "SCSI.h"

#define INIT_TIMEOUT 2000
#define CMD_TIMEOUT 300
#define RW_TIMEOUT 500

// ready state is all bits 0
#define R1_READY_STATE 0
#define R1_IDLE_STATE _BV(0)
#define R1_ERASE_RESET _BV(1)
#define R1_ILLEGAL_CMD _BV(2)
#define R1_CMD_CRC_ERR _BV(3)
#define R1_ERASE_SEQ_ERR _BV(4)
#define R1_ADDR_ERR _BV(5)
#define R1_PARAM_ERR _BV(6)
// bit 7 is always 0.

static inline void set_speed_slow(void) ATTR_ALWAYS_INLINE;
static inline void set_speed_slow(void) {
	// 250 kHz
#ifdef USART_SPI
	USARTC0.BAUDCTRLA = 63; // 32 MHz / 2*250 kHz - 1
	USARTC0.BAUDCTRLB = 0;
#else
	SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV128_gc;
#endif
}
static inline void set_speed_fast(void) ATTR_ALWAYS_INLINE;
static inline void set_speed_fast(void) {
	// RAMMING SPEED!
#ifdef USART_SPI
	USARTC0.BAUDCTRLA = 0;
	USARTC0.BAUDCTRLB = 0;
#else
	SPIC.CTRL = SPI_CLK2X_bm | SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc;
#endif
}

void init_spi(void) {
#ifdef USART_SPI
	USARTC0.CTRLC = USART_CMODE_MSPI_gc; // SPI Master mode
	USARTC0.CTRLB = USART_TXEN_bm | USART_RXEN_bm;
	USARTC0.CTRLA = 0; // no interrupts
#endif
	set_speed_slow();
}

static inline uint8_t SPI_byte(uint8_t data) ATTR_ALWAYS_INLINE;
static inline uint8_t SPI_byte(uint8_t data) {
#ifdef USART_SPI
	while (!(USARTC0.STATUS & USART_DREIF_bm)) ; // just an assertion (no-op)
	USARTC0.DATA = data;
	while (!(USARTC0.STATUS & USART_TXCIF_bm)) ; // wait for tx complete
	while (!(USARTC0.STATUS & USART_RXCIF_bm)) ; // Wait for the data
	return USARTC0.DATA;
#else
	SPIC.DATA = data;
	while(!(SPIC.STATUS & SPI_IF_bm));
	return SPIC.DATA;
#endif
}

static uint8_t waitForStart(uint16_t timeout) {
	uint8_t byte;
	// XXX this idiom isn't interrupt-safe.
	for(uint16_t now = milli_timer; milli_timer - now < timeout; ) {
		if ((byte = SPI_byte(0xff)) != 0xff) break;
	}
	return (byte != 0xfe);
}

static uint8_t waitForIdle(uint16_t timeout) {
	// XXX this idiom isn't interrupt-safe.
	for(uint16_t now = milli_timer; milli_timer - now < timeout; ) {
		if (SPI_byte(0xff) == 0xff) return 0; //idle
	}
	return 1; // timeout
}

static uint8_t sendCommand_R1(uint8_t cmd, uint32_t arg) {
	uint8_t response;
	if (waitForIdle(CMD_TIMEOUT)) {
		response = 0xff;
		return response;
	}

	SPI_byte(cmd | 0x40);
	SPI_byte((uint8_t)(arg >> 24));
	SPI_byte((uint8_t)(arg >> 16));
	SPI_byte((uint8_t)(arg >> 8));
	SPI_byte((uint8_t)(arg));
	switch(cmd) {
		case 0: SPI_byte(0x95); // checksum for command 0
			break;
		case 8: SPI_byte(0x87); // checksum for command 8 with 0x1aa payload
			break;
		default: SPI_byte(0xff); // bogus checksum
			break;
	}
	response = 0xff;
	// XXX this idiom isn't interrupt-safe.
	for(uint16_t now = milli_timer; milli_timer - now < CMD_TIMEOUT; ) {
		response = SPI_byte(0xff);
		if (!(response & 0x80)) break;
	}
	return response;
}

static uint8_t init_card(uint8_t card) {
	// Make sure the cards are still in
	if (!CD_STATE) return 1; // fail

	ASSERT_CARD(card);

	// We only support SDHC/SDXC cards.
	uint8_t init_success = 0;
	// XXX this idiom isn't interrupt-safe.
	for(uint16_t now = milli_timer; milli_timer - now < INIT_TIMEOUT; ) {
		if (sendCommand_R1(0, 0) == R1_IDLE_STATE) {
			init_success = 1;
			break;
		}
	}
	if (!init_success) goto fail;

	if (sendCommand_R1(8, 0x1aaUL) & R1_ILLEGAL_CMD) goto fail;
	uint32_t r3resp = 0;
	for(int i = 0; i < 4; i++) {
		r3resp <<= 8;
		r3resp |= SPI_byte(0xff);
	}
	if ((r3resp & 0xff) != 0xaa) goto fail;

	// Make sure the cards are still in
	if (!CD_STATE) goto fail;

	init_success = 0;
	// XXX this idiom isn't interrupt-safe.
	for(uint16_t now = milli_timer; milli_timer - now < INIT_TIMEOUT; ) {
		sendCommand_R1(55, 0);
		if (sendCommand_R1(41, 0x40000000UL) == R1_READY_STATE) {
			init_success = 1;
			break;
		}
	}
	if (!init_success) goto fail;

	// Make sure the cards are still in
	if (!CD_STATE) goto fail;

	// the top 2 bits of the OCR register
	if (sendCommand_R1(58, 0) != R1_READY_STATE) goto fail;
	uint8_t r58_1st = SPI_byte(0xff);
	for(int i = 0; i < 3; i++) SPI_byte(0xff); //ignore the remaining R3 bytes
	if ((r58_1st & 0xc0) != 0xc0) goto fail;

	DEASSERT_CARDS;
	return 0;
fail:
	DEASSERT_CARDS;
	return 1;
	
}

static uint8_t getCardSize(uint8_t card, uint32_t *size) {
	if (!CD_STATE) return 1; // fail
	ASSERT_CARD(card);
	// read the CSD register
	if (sendCommand_R1(9, 0) != R1_READY_STATE) goto fail;

	if (waitForStart(CMD_TIMEOUT)) goto fail;
	uint8_t resp[16];
	for(int i = 0; i < 16; i++)
		resp[i] = SPI_byte(0xff);

	SPI_byte(0xff); // CRC
	SPI_byte(0xff); // CRC
	DEASSERT_CARDS;

	uint32_t out;
	out = ((uint32_t)(resp[7] & 0x3f)) << 16; // lop off the reserved bytes
	out |= ((uint32_t)resp[8]) << 8;
	out |= ((uint32_t)resp[9]) << 0;

	//out++;
	//out <<= 10;

	*size = (out + 1) << 10;

	return 0;

fail:
	DEASSERT_CARDS;
	return 1;
}

uint32_t volume_size;

uint8_t init(void) {
	DEASSERT_CARDS;
	CARD_POWER_ON;

	// For 100 ms, the card detect lines have to be on (low).
	// XXX this idiom isn't interrupt-safe.
	for(uint16_t now = milli_timer; milli_timer - now < 500; ) {
		if (!CD_STATE) return 1; // fail
	}

	set_speed_slow();
	// send 80 slow clocks.
	for(int i = 0; i < 10; i++) {
		SPI_byte(0xff);
	}

#if 0
	// This is tricky. We have to perform at least one CMD0 with CS asserted.... on both cards.
	// This isn't really kosher, since both cards will be on MISO at the same time, but we'll
	// ignore it. Each individual card will get their own CMD0, but at that point both will
	// be in SPI mode, so they'll know to respect !CS.
	ASSERT_CARD(0);
	ASSERT_CARD(1);
	sendCommand_R1(0, 0);
	DEASSERT_CARDS;
#endif

	if (init_card(0)) return 1; // card A

	if (init_card(1)) return 1; // card B

	set_speed_fast();

	uint32_t size_a, size_b;
	if (getCardSize(0, &size_a)) return 1;
	if (getCardSize(1, &size_b)) return 1;
	volume_size = (((size_a > size_b)?size_b:size_a) - 1) * 2 - 1;

	if (!CD_STATE) return 1; // fail
	return prepVolume(); // try to initialize the crypto
}

uint8_t volumeReadBlock(uint32_t blocknum) {
	uint8_t card = setupBlockCrypto(blocknum, 1);
	uint32_t phys_block = (blocknum >> 1) + 1;

	if (!CD_STATE) return 1; // fail
	ASSERT_CARD(card);
	LED_ON(LED_ACT_bm);

	if (waitForIdle(RW_TIMEOUT)) goto fail;

	if (sendCommand_R1(17, phys_block) != R1_READY_STATE) goto fail;

	if (waitForStart(RW_TIMEOUT)) goto fail;

	for(int i = 0; i < VIRTUAL_MEMORY_BLOCK_SIZE / MASS_STORAGE_IO_EPSIZE; i++) {
		if (!(Endpoint_IsReadWriteAllowed())) {
			/* Clear the current endpoint bank */
			Endpoint_ClearIN();

			/* Wait until the host has sent another packet */
			if (Endpoint_WaitUntilReady())
				goto fail;
		}
		for(int j = 0; j < MASS_STORAGE_IO_EPSIZE / BLOCKSIZE; j++) {
			uint8_t data[BLOCKSIZE];
			for(int k = 0; k < BLOCKSIZE; k++) {
#ifdef USART_SPI
				while(!(USARTC0.STATUS & USART_DREIF_bm)); // wait for ready
				USARTC0.DATA = 0xff;
				while(!(USARTC0.STATUS & USART_RXCIF_bm)); // wait to read
				data[k] = USARTC0.DATA;
#else
				SPIC.DATA = 0xff; // run the clock
				while(!(SPIC.STATUS & SPI_IF_bm)) ; // wait for it
				data[k] = SPIC.DATA;
#endif
			}
			process_xex_block(data);
			Endpoint_Write_Stream_LE(data, sizeof(data), NULL);
		}
	}

	SPI_byte(0xff); // CRC
	SPI_byte(0xff); // CRC

	DEASSERT_CARDS;
	LED_OFF(LED_ACT_bm);
	LED_OFF(LED_ERR_bm);
	return 0;

fail:
	DEASSERT_CARDS;
	LED_OFF(LED_ACT_bm);
	LED_ON(LED_ERR_bm);
	return 1;
}

uint8_t volumeWriteBlock(uint32_t blocknum) {
	uint8_t card = setupBlockCrypto(blocknum, 0);
	uint32_t phys_block = (blocknum >> 1) + 1;

	if (!CD_STATE) return 1; // fail
	ASSERT_CARD(card);
	LED_ON(LED_ACT_bm);

	if (waitForIdle(RW_TIMEOUT)) goto fail;

	if (sendCommand_R1(24, phys_block) != R1_READY_STATE) goto fail;

	if (waitForIdle(RW_TIMEOUT)) goto fail;
	SPI_byte(0xfe);
	for(int i = 0; i < VIRTUAL_MEMORY_BLOCK_SIZE / MASS_STORAGE_IO_EPSIZE; i++) {
		if (!(Endpoint_IsReadWriteAllowed())) {
			/* Clear the current endpoint bank */
			Endpoint_ClearOUT();

			/* Wait until the host has sent another packet */
			if (Endpoint_WaitUntilReady())
				goto fail;
		}
		for(int j = 0; j < MASS_STORAGE_IO_EPSIZE / BLOCKSIZE; j++) {
			uint8_t data[BLOCKSIZE];
			Endpoint_Read_Stream_LE(data, sizeof(data), NULL);
			process_xex_block(data);
			for(int k = 0; k < sizeof(data); k++) {
#ifdef USART_SPI
				while(!(USARTC0.STATUS & USART_DREIF_bm)) ; // wait for ready
				USARTC0.DATA = data[k];
#else
				if (i != 0 || j != 0 || k != 0)
					while(!(SPIC.STATUS & SPI_IF_bm)) ; // wait for ready
				SPIC.DATA = data[k];
#endif
			}
		}
	}
	// Wait for the last one
#ifdef USART_SPI
	while(!(USARTC0.STATUS & USART_TXCIF_bm)) ; // wait for transmit complete
#else
	while(!(SPIC.STATUS & SPI_IF_bm)) ; // wait for it to go
#endif

	SPI_byte(0xff); // CRC
	SPI_byte(0xff); // CRC
	uint8_t data_status = SPI_byte(0xff);
	if ((data_status & 0x1f) != 5) goto fail;
	DEASSERT_CARDS;
	LED_OFF(LED_ACT_bm);
	LED_OFF(LED_ERR_bm);

	return 0;

fail:
	DEASSERT_CARDS;
	LED_OFF(LED_ACT_bm);
	LED_ON(LED_ERR_bm);
	return 1;
}

uint8_t readKeyBlock(uint8_t card, uint8_t *block, size_t length) {
	ASSERT_CARD(card);

	if (waitForIdle(RW_TIMEOUT)) goto fail;

	if (sendCommand_R1(17, 0) != R1_READY_STATE) goto fail;

	if (waitForStart(RW_TIMEOUT)) goto fail;

	for(int i = 0; i < 512; i++) {
		uint8_t b = SPI_byte(0xff);
		if (i < length) block[i] = b;
	}

	SPI_byte(0xff); // CRC
	SPI_byte(0xff); // CRC

	DEASSERT_CARDS;
	return 0;

fail:
	DEASSERT_CARDS;
	return 1;
}

uint8_t writeKeyBlock(uint8_t card, uint8_t *block, size_t length) {
	ASSERT_CARD(card);

	if (waitForIdle(RW_TIMEOUT)) goto fail;

	if (sendCommand_R1(24, 0) != R1_READY_STATE) goto fail;

	if (waitForIdle(RW_TIMEOUT)) goto fail;
	SPI_byte(0xfe);
	for (int i = 0; i < 512; i++)
		SPI_byte((i >= length)?0xff:block[i]);

	SPI_byte(0xff); // CRC
	SPI_byte(0xff); // CRC
	uint8_t data_status = SPI_byte(0xff);
	DEASSERT_CARDS;

	return (data_status & 0x1f) != 5;

fail:
	DEASSERT_CARDS;
	return 1;
}

