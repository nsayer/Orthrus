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
#include <stddef.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <LUFA/Drivers/USB/USB.h>
#include <LUFA/Platform/Platform.h>

#include "Orthrus.h"
#include "SCSI.h"
#include "AES.h"
#include "sd.h"
#include "Descriptors.h"

// 10 microseconds => 100 kHz sampling for the entropy source
#define DELAY_US_TIME (10)

// To turn 32 MHz prescaled by 1024 into a 1 ms timer,
// we count 31 1/4 counts per interrupt. We do this
// by counting to 31 3 times, and then 32 once.
#define TC_BASE (31)
#define TC_LONG_CYCLES (1)
#define TC_TOTAL_CYCLES (4)

// debounce the switch for 50 ms - that means ignoring any state changes
// within that long of the first state change.
#define DEBOUNCE_TIME (50)

static const char PROGMEM MAGIC[] = "OrthrusVolumeV01";

static uint8_t nonceA[BLOCKSIZE], nonceB[BLOCKSIZE];

static uint8_t cardswap;

// Only a minutes worth, but we have a very short attention span
volatile uint16_t milli_timer;

uint8_t force_attention, unit_active;

static int32_t debounce_start; // signed and large so we can use -1 to disable
static uint8_t button_state;

/*
 * Read 8 bits from the RNG at a sample rate of 100 kHz.
 */
static uint8_t randomByte(void) {
	uint8_t out = 0;
	for(int i = 0; i < 8; i++) {
		_delay_us(DELAY_US_TIME);
		out <<= 1;
		out |= RNG_STATE?0:1; // why the hell not?
	}
	return out;
}

/*
 * This method generates 16 bytes at a time of pseudo-random data.
 * We use AES CMAC to whiten the raw data from the entropy source.
 * We gather ENTROPY_EXPANSION + 1 16 byte blocks for this purpose.
 * One is used as the AES CMAC key and the rest are used as the data.
 * The result is a block of 16 PRNG bytes for the key generator.
 */
void fillRandomBuffer(uint8_t *buf) {
	uint8_t keyblock[KEYSIZE];
	for (int i = 0; i < sizeof(keyblock); i++) {
		keyblock[i] = randomByte();
	}
	setKey(keyblock);

	uint8_t ent_block[BLOCKSIZE * ENTROPY_EXPANSION];
	for (int i = 0; i < sizeof(ent_block); i++) {
		ent_block[i] = randomByte();
	}
	CMAC(ent_block, sizeof(ent_block), buf);
}

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
uint8_t prepVolume(void) {
	uint8_t volid[32], keyblock[2][32];
	uint8_t blockbuf[128];
	if (readKeyBlock(0, blockbuf, sizeof(blockbuf))) return 1; // card A
	if (memcmp_P(blockbuf, MAGIC, strlen_P(MAGIC))) return 1; // FAIL!
	cardswap = blockbuf[96] != 0; // we're swapping if A isn't A
	memcpy(volid, blockbuf + 16, sizeof(volid));
	memcpy(cardswap?keyblock[1]:keyblock[0], blockbuf + 48, sizeof(volid));
	memcpy(cardswap?nonceB:nonceA, blockbuf + 80, sizeof(nonceA));
	if (readKeyBlock(1, blockbuf, sizeof(blockbuf))) return 1; // card B
	if (!((blockbuf[96] != 0) ^ cardswap)) return 1; // One A, one B.
	if (memcmp_P(blockbuf, MAGIC, strlen_P(MAGIC))) return 1; // FAIL!
	if (memcmp(blockbuf + 16, volid, sizeof(volid))) return 1; // FAIL!
	memcpy(cardswap?keyblock[0]:keyblock[1], blockbuf + 48, sizeof(volid));
	memcpy(cardswap?nonceA:nonceB, blockbuf + 80, sizeof(nonceA));
	memset(blockbuf, 0, 16); // save RAM - use the block buf as temp
	setKey(blockbuf); // all zero key.
	CMAC((uint8_t*)keyblock, 64, blockbuf);
	setKey(blockbuf);
	CMAC(volid, 32, blockbuf);
	setKey(blockbuf);
	generateDecryptKey();
	return 0; // all set!
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
uint8_t initVolume(void) {
	uint8_t blockbuf[128];

	// Magic
	memcpy_P(blockbuf, MAGIC, strlen_P(MAGIC));

	// volume ID
	fillRandomBuffer(blockbuf + 16);
	fillRandomBuffer(blockbuf + 32);

	// key block A
	fillRandomBuffer(blockbuf + 48);
	fillRandomBuffer(blockbuf + 64);

	// nonce block A
	fillRandomBuffer(blockbuf + 80);

	blockbuf[96] = 0; // card A
	if (writeKeyBlock(0, blockbuf, sizeof(blockbuf))) return 1;

	// key block B
	fillRandomBuffer(blockbuf + 48);
	fillRandomBuffer(blockbuf + 64);

	// nonce block B
	fillRandomBuffer(blockbuf + 80);

	blockbuf[96] = 1; // card B
	if (writeKeyBlock(1, blockbuf, sizeof(blockbuf))) return 1;

	// As a side effect, prep the volume, which essentially
	// means setting the AES key to the volume key and setting
	// cardswap appropriately (to zero, since we did the order right).
	return prepVolume();
}

// return 0 for *PHYSICAL* card A or 1 for B
// During the data transfer, we can call encrypt_CTR_byte() on each byte as we go.
uint8_t setupBlockCrypto(uint32_t blocknum, uint8_t mode) {
	uint8_t cardA = (blocknum & 0x1) == 0;

	uint8_t *nonce = cardA?nonceB:nonceA;
	nonce[12] = (uint8_t)(blocknum >> 24);
	nonce[13] = (uint8_t)(blocknum >> 16);
	nonce[14] = (uint8_t)(blocknum >> 8);
	nonce[15] = (uint8_t)(blocknum >> 0);
	init_xex(nonce, sizeof(nonceA), mode);

	return !(cardA ^ cardswap);
}

ISR(TCC0_OVF_vect) {
	milli_timer++; // the whole point of doing this.
	// Subtract 1 from whatever's here because the counter is
	// 0 based *and* inclusive
	TCC0.PER = TC_BASE + (((milli_timer % TC_TOTAL_CYCLES) < TC_LONG_CYCLES)?1:0) - 1;
}

uint8_t check_button(void) {
	uint16_t now;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		now = milli_timer;
	}
	if (debounce_start != -1 && now - ((uint16_t)debounce_start) < DEBOUNCE_TIME) return button_state;
	debounce_start = -1; // force it off.
	uint8_t now_button = !SWITCH_STATE; // button is active-low
	if (now_button == button_state) return button_state; // no change
	button_state = now_button;
	debounce_start = now; // start a new debounce time.
	return button_state;
}

void init_timer(void) {
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
	TCC0.PER = TC_BASE - 1;
}

#ifdef DEBUG
// For 9600 baud at 32 MHz, BSEL = 12 BSCALE = 4 CLK2X = 0
#define BSEL (12)
#define BSCALE (4)

#define SER_BUFSIZE (128)

volatile static uint8_t diag_tx_buf[SER_BUFSIZE];
volatile static uint8_t diag_tx_head, diag_tx_tail;

ISR(USARTE0_DRE_vect) {
	// If the buffer is empty...
	if (diag_tx_tail == diag_tx_head) {
		// turn off the DRE interrupt.
		USARTE0.CTRLA = USART_DREINTLVL_OFF_gc;
		return;
	}
	USARTE0.DATA = diag_tx_buf[diag_tx_tail];
	if (++diag_tx_tail >= SER_BUFSIZE) diag_tx_tail = 0;
}

void diag_tx(uint8_t data) {
	uint8_t buf_in_use;
	do {
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			buf_in_use = diag_tx_head - diag_tx_tail;
		}
		if (buf_in_use < 0) buf_in_use += SER_BUFSIZE;
	} while (buf_in_use >= SER_BUFSIZE - 2) ; // wait for room in the transmit buffer

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		diag_tx_buf[diag_tx_head] = data;
		if (++diag_tx_head >= SER_BUFSIZE) diag_tx_head = 0;
	}
	USARTE0.CTRLA = USART_DREINTLVL_LO_gc; // force on the DRE interrupt.
}
#endif

void init_ports(void) {
	// start with LEDs and card power off - do this before changing direction.
	CARD_POWER_OFF;
	LED_OFF(LED_RDY_bm | LED_ACT_bm | LED_ERR_bm);
	// LED and cardpower pins are output, switch is input.
	PORTA.DIRSET = (1<<0) | (1<<1) | (1<<2) | (1<<4);
	// pull-up on switch pin
	PORTA.PIN3CTRL = PORT_OPC_PULLUP_gc;

#ifdef PINSWAP
#ifdef USART_SPI
	// shift USART0 into place over the SPI pins
	PORTC.REMAP = PORT_USART0_bm;
#else
	// swap MOSI and SCK to match the traditional USART wiring scheme,
	// which is what the hardware uses.
	PORTC.REMAP = PORT_SPI_bm;
#endif
#endif

	// Port C direction is largely set by CARD_POWER_OFF.
	// The only thing of note we'd do is set the card detect
	// pins to inputs, and they already are.
	// pull-up on MISO and the two card detect pins
	PORTCFG.MPCMASK = (1<<1) | (1<<2) | (1<<6);
	PORTC.PIN1CTRL = PORT_OPC_PULLUP_gc;

	//PORTD.DIR = 0; // all input. Redundant.

#ifdef DEBUG
	diag_tx_head = diag_tx_tail = 0;
	// PE3 is TX_DIAG - output
	PORTE.OUTSET = (1<<3);
	PORTE.DIRSET = (1<<3);
	USARTE0.CTRLA = USART_DREINTLVL_OFF_gc; // for now - when we fill the buf, we turn it on.
	USARTE0.CTRLB = USART_TXEN_bm;
	USARTE0.CTRLC = USART_CHSIZE_8BIT_gc;
	USARTE0.BAUDCTRLA = (uint8_t)BSEL;
	USARTE0.BAUDCTRLB = (BSCALE << USART_BSCALE_gp) | ((uint8_t)(BSEL >> 8));
#endif

}



/** LUFA Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Device_t Disk_MS_Interface =
        {
                .Config =
                        {
                                .InterfaceNumber           = INTERFACE_ID_MassStorage,
                                .DataINEndpoint            =
                                        {
                                                .Address           = MASS_STORAGE_IN_EPADDR,
                                                .Size              = MASS_STORAGE_IO_EPSIZE,
                                                .Banks             = 1,
                                        },
                                .DataOUTEndpoint           =
                                        {
                                                .Address           = MASS_STORAGE_OUT_EPADDR,
                                                .Size              = MASS_STORAGE_IO_EPSIZE,
                                                .Banks             = 1,
                                        },
                                .TotalLUNs                 = 1,
                        },
        };

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
        MS_Device_ConfigureEndpoints(&Disk_MS_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
        MS_Device_ProcessControlRequest(&Disk_MS_Interface);
}

/** Mass Storage class driver callback function the reception of SCSI commands from the host, which must be processed.
 *
 *  \param[in] MSInterfaceInfo  Pointer to the Mass Storage class interface configuration structure being referenced
 */
bool CALLBACK_MS_Device_SCSICommandReceived(USB_ClassInfo_MS_Device_t* const MSInterfaceInfo)
{
        bool CommandSuccess;

        CommandSuccess = SCSI_DecodeSCSICommand(MSInterfaceInfo);

        return CommandSuccess;
}

void __ATTR_NORETURN__ main(void) {

	// Run the CPU at 32 MHz.
	OSC.CTRL = OSC_RC32MEN_bm;
	while(!(OSC.STATUS & OSC_RC32MRDY_bm)) ; // wait for it.
	_PROTECTED_WRITE(CLK.CTRL, CLK_SCLKSEL_RC32M_gc);
	OSC.CTRL &= ~(OSC_RC2MEN_bm); // we're done with the 2 MHz osc.

	// Set up the DFLL to correct to the USB SOF.
	OSC.DFLLCTRL = OSC_RC32MCREF_USBSOF_gc; // correct the 32 MHz oscillator from USB SOF.
	DFLLRC32M.COMP1 = (F_CPU / 1024) & 0xff;
	DFLLRC32M.COMP2 = (F_CPU / 1024) >> 8;
	NVM.CMD = NVM_CMD_READ_CALIB_ROW_gc;
	DFLLRC32M.CALA = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, RCOSC32MA));
	DFLLRC32M.CALB = pgm_read_byte(offsetof(NVM_PROD_SIGNATURES_t, RCOSC32M));
	NVM.CMD = 0;
	DFLLRC32M.CTRL |= DFLL_ENABLE_bm;

	// Now set up the PLL for 48 MHz for USB.
	OSC.PLLCTRL = OSC_PLLSRC_RC32M_gc | (F_USB / (F_CPU / 4)); // The PLL input is the 32 MHz osc divided by 4.
	OSC.CTRL |= OSC_PLLEN_bm;
	while(!(OSC.STATUS & OSC_PLLRDY_bm)) ; // wait for it.
	CLK.USBCTRL = CLK_USBSRC_PLL_gc | CLK_USBSEN_bm; // USB is clocked from the PLL.

	// turn off the bits of the chip we don't need.
	PR.PRGEN = PR_RTC_bm | PR_EBI_bm | PR_EVSYS_bm | PR_DMA_bm; // EBI is probably moot for this chip variant.
	PR.PRPA = PR_DAC_bm | PR_ADC_bm | PR_AC_bm; // all analog stuff off.
	PR.PRPB = PR_DAC_bm | PR_ADC_bm | PR_AC_bm; // all analog stuff off.

	PR.PRPC = PR_TWI_bm | PR_USART1_bm | PR_HIRES_bm | PR_TC1_bm
#ifdef USART_SPI
								     | PR_SPI_bm
#else
								     | PR_USART0_bm
#endif
		;


	PR.PRPE = PR_TWI_bm | PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm
#ifndef DEBUG
											     | PR_USART0_bm
#endif
		;

	PR.PRPD = PR_TWI_bm | PR_USART0_bm | PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;
	PR.PRPF = PR_TWI_bm | PR_USART0_bm | PR_USART1_bm | PR_SPI_bm | PR_HIRES_bm | PR_TC1_bm | PR_TC0_bm;

	init_ports();
	init_spi();
	init_timer();
	init_aes();

	// Enable all levels of the interrupt controller
	PMIC.CTRL = PMIC_HILVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_LOLVLEN_bm;

	LED_OFF(LED_ACT_bm | LED_RDY_bm | LED_ERR_bm);

	unit_active = 0;
	force_attention = 0;

	GlobalInterruptEnable();

	USB_Init();

	uint8_t cards_present = 0;
	uint8_t button_state = 0, ignoring_button = 0;
	uint16_t button_started = 0;

	while(1) {

		uint8_t cards_now = CD_STATE;

		if (cards_present ^ cards_now) {
			// there has been a change. Note it for the future.
			cards_present = cards_now;
			if (cards_present) {
				// cards have now shown up. We want to take note
				// of the button from scratch, so clear all our markers.
				ignoring_button = 0;
				button_state = 0;
				if (init()) {
					LED_ON(LED_ERR_bm);
					CARD_POWER_OFF;
					clearKeys();
					unit_active = 0;
					force_attention = 1;
				} else {
					LED_ON(LED_RDY_bm);
					unit_active = 1;
					force_attention = 1;
				}
			} else {
				CARD_POWER_OFF;
				clearKeys();
				unit_active = 0;
				force_attention = 1;
				// lights out!
				LED_OFF(LED_ACT_bm | LED_RDY_bm | LED_ERR_bm);
			}
		}

		MS_Device_USBTask(&Disk_MS_Interface);
                USB_USBTask();

		// At this point, we're done if there are no cards. The button
		// does nothing.
		if (!cards_present) continue;

		uint8_t new_button = check_button();
		if (new_button != button_state) {
			// The button has changed! Action!
			button_state = new_button;
			if (button_state) {
				// The button is now down. Save the state of the LEDs
				// and start the countdown.
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
					button_started = milli_timer;
				}
				// forceably unmount.
				LED_OFF(LED_RDY_bm);
				unit_active = 0;
				force_attention = 1;
			} else {
				// They let the button up. There's two possibilities:
				// Either they let it up before we blew it up, or they
				// chickened out. If they chickened out, then put the
				// LEDs back the way they were. If it went off, then
				// when we formatted the disk we marked the button as
				// being in an ignored state. We can now undo that.
				if (!ignoring_button) {
					unit_active = 1;
					force_attention = 1;
					LED_OFF(LED_ERR_bm);
					// go back to showing ready.
					LED_ON(LED_RDY_bm);
				}
				ignoring_button = 0;
			}
		}
		if (button_state && !ignoring_button) {
			// The button is down and we're not ignoring it.
			// If the timer is done, then blow up the world.
			uint16_t now;
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				now = milli_timer;
			}
			if (now - button_started >= 5000) {
				if (PORTA.OUT & CRDPWR_bm) {
					// The cards are off. Initialize them first.
					init();
				}
				if (!initVolume()) {
					// success!
					LED_OFF(LED_ACT_bm | LED_RDY_bm | LED_ERR_bm);
					LED_ON(LED_RDY_bm);
					unit_active = 1;
					force_attention = 1;
					ignoring_button = 1;
				} else {
					// power the card off so that if they hit the button
					// again maybe it'll work.
					CARD_POWER_OFF;
					clearKeys();
					LED_OFF(LED_ACT_bm | LED_RDY_bm | LED_ERR_bm);
					LED_ON(LED_ERR_bm);
					unit_active = 0;
					// no, nothing changed. It was offline and still is.
					//force_attention = 1;
					ignoring_button = 1;
				}
			} else {
				// blink the error light to warn them they're about
				// to kill the world.
				if (((now - button_started) / 250) % 2)
					LED_ON(LED_ERR_bm);
				else
					LED_OFF(LED_ERR_bm);
			}
		}
	}
}
