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

Pins:

PC0 - RNGPWR - power on/off for the RNG
PC1 - !CDA - card detect A - requires pull-up
PC2 - !CDB - card detect B - requires pull-up
PC3 - !CS - chip select
PC4 - A/!B - Pick between card A and B
PC5 - SCK
PC6 - MISO
PC7 - MOSI

Note that MOSI and SCK are swapped from normal SPI wiring. For normal
SPI, remap them with REMAP.PORT_SPI_bm. For USART0 in SPI master mode,
remap the USART with REMAP.PORT_USART0_bm. Older hardware does not have
this pinswap in effect. See the PINSWAP macro below.

PA0 - LED_RDY - the ready LED
PA1 - LED_ACT - the activity LED
PA2 - LED_ERR - the error LED
PA3 - !SW0 - the history eraser button - requires pull-up
PA4 - !CRDPWR - power on/off for the SD cards

PD0 - RNG - The output from the entropy generator

In addition, USARTE0 is the diag port (currently unused).

*/

// Hardware starting with board version v2.0.2 swap MOSI and SCK. For those
// versions, define PINSWAP, leaving it undefined for previous versions.
#define PINSWAP

// To use USART0 in SPI Master mode instead of traditional SPI, turn this on.
// (Since it's supposed to be faster, you probably want it on...
// unless you don't). This is only supported on PINSWAP hardware.
//#define USART_SPI

// Turn on the diagnostic output port. Currently, it doesn't do anything anyway, though.
//#define DEBUG

// How much source entropy per output RNG do we require? Note that
// this value is actually less because we also pull a random key
// block for the CMAC key. Since the key and block are the same size,
// we can just subtract 1 from the value we want. More bits ostensibly
// dilute whatever biases there are in the RNG, but take more RAM and
// time. 8 way expansions means the whole key generation process
// takes at least 80 ms (probably more because of AES etc).
#define ENTROPY_EXPANSION (8 - 1)

#if !defined(PINSWAP) && defined(USART_SPI)
#error USART in SPI master mode requires PINSWAP hardware.
#endif

#include <avr/io.h>

#define RNGPWR_bm (1<<0)
#define CS_bm (1<<3)
#define A_NOT_B_bm (1<<4)
#define CD_A_bm (1<<1)
#define CD_B_bm (1<<2)
#define CD_MASK (CD_A_bm | CD_B_bm)
#define CD_STATE (!(PORTC.IN & CD_MASK))

// Pick a card.
#define ASSERT_CARD(x) do { if (x) PORTC.OUTCLR = A_NOT_B_bm; else PORTC.OUTSET = A_NOT_B_bm; PORTC.OUTCLR = CS_bm; } while(0)
// Deassert both cards (only one will be asserted at a time, but this is ok)
#define DEASSERT_CARDS (PORTC.OUTSET = CS_bm)

#define LED_RDY_bm (1<<0)
#define LED_ACT_bm (1<<1)
#define LED_ERR_bm (1<<2)
#define LED_ON(x) (PORTA.OUTSET = (x))
#define LED_OFF(x) (PORTA.OUTCLR = (x))

#define SW_bm (1<<3)
#define SWITCH_STATE (PORTA.IN & SW_bm)

#define CRDPWR_bm (1<<4)
// Turn the card power off and make all card logic lines inputs (hi impedance)
#define CARD_POWER_OFF do { PORTC.DIRCLR = (1<<3) | (1<<5) | (1<<7); PORTA.OUTSET = CRDPWR_bm; } while(0)
// Turn the card power on and make the correct card logic lines outputs and force both !CS lines high
#define CARD_POWER_ON do { DEASSERT_CARDS; PORTC.DIRSET = (1<<3) | (1<<5) | (1<<7); PORTA.OUTCLR = CRDPWR_bm; } while(0)

#define RNG_POWER_ON (PORTC.OUTSET = RNGPWR_bm)
#define RNG_POWER_OFF (PORTC.OUTCLR = RNGPWR_bm)

#define RNG_bm (1<<0)
#define RNG_STATE (PORTD.IN & RNG_bm)

// These are callbacks from the sd code.
uint8_t setupBlockCrypto(uint32_t blocknum, uint8_t mode);
uint8_t initVolume(void);
uint8_t prepVolume(void);

void init_timer(void);
void init_ports(void);
uint8_t check_button(void);

// This is the millisecond time counter.
extern volatile uint16_t milli_timer;
extern uint8_t unit_active;
extern uint8_t force_attention;

