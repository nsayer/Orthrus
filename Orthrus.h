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

PC1 - !CDA - card detect A - requires pull-up
PC2 - !CDB - card detect B - requires pull-up
PC3 - !CSA - card select A
PC4 - !CSB - card select B
PC5 - MOSI
PC6 - MISO
PC7 - SCK

PA0 - LED_RDY - the ready LED
PA1 - LED_ACT - the activity LED
PA2 - LED_ERR - the error LED
PA3 - !SW0 - the history eraser button - requires pull-up
PA4 - !CARDEN - power on/off for the SD cards

PD0 - RNG - The output from the entropy generator

In addition, USARTE0 is the diag port.

*/

#include <avr/io.h>

#define CS_A_bm (1<<3)
#define CS_B_bm (1<<4)
#define CD_A_bm (1<<1)
#define CD_B_bm (1<<2)
#define CD_MASK (CD_A_bm | CD_B_bm)
#define CD_STATE (!(PORTC.IN & CD_MASK))

// Assert the card CS line, 0 for A, 1 for B
#define ASSERT_CARD(x) (PORTC.OUTCLR = ~((x)?CS_B_bm:CS_A_bm))
// Deassert both cards (only one will be asserted at a time, but this is ok)
#define DEASSERT_CARDS (PORTC.OUTSET = (CS_A_bm | CS_B_bm))

#define LED_RDY_bm (1<<0)
#define LED_ACT_bm (1<<1)
#define LED_ERR_bm (1<<2)
#define LED_ON(x) (PORTA.OUTSET = (x))
#define LED_OFF(x) (PORTA.OUTCLR = (x))

#define SW_bm (1<<3)
#define SWITCH_STATE (PORTA.IN & SW_bm)

#define CRDPWR_bm (1<<4)
#define CARD_POWER_OFF (PORTA.OUTSET = CRDPWR_bm)
#define CARD_POWER_ON (PORTA.OUTCLR = CRDPWR_bm)

#define RNG_bm (1<<0)
#define RNG_STATE (PORTD.IN & RNG_bm)

// These are callbacks from the sd code.
uint8_t setupBlockCrypto(uint32_t blocknum);
uint8_t initVolume(void);
uint8_t prepVolume(void);

void init_timer(void);
void init_ports(void);
uint8_t check_button(void);

// This is the millisecond time counter.
extern volatile uint16_t milli_timer;
extern uint8_t unit_active;
extern uint8_t force_attention;

