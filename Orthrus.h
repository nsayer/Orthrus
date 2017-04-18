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

Fuses: low: 0xDE, high: 0xD9, Extended: 0xF8

High speed crystal oscillator, full speed. No HWBE, no debugWire, RESET enabled, no bootvector, 4.3v brownout.

Pins:

PB0 - NC - must be an output
PB1 - SCK
PB2 - MOSI
PB3 - MISO
PB4 - !CSA - card select A
PB5 - !CSB - card select B
PB6 - !CDA - card detect A - requires pull-up
PB7 - !CDB - card detect B - requires pull-up

PC2 - !SW0 - the history eraser button - requires pull-up
PC4 - LED_ERR - the error LED
PC5 - LED_ACT - the activity LED
PC6 - LED_RDY - the ready LED
PC7 - NC

PD0 - RNG - The output from the entropy generator
PD1-7 - NC

Note that the cards run at 3.3v, but the controller runs at 5v. That means
that MOSI, SCK and the two card select lines must be level-shifted. MISO
need not be level shifted, but the ramification of not doing so is that
you must not perform ISP (programming) at 5v with cards inserted. It's
probably a bad idea to reprogram it with USB connected too.

*/

#include <avr/io.h>

// Port B has the two cards. Despite using hardware SPI, we must
// set the directions correctly. MOSI, SCK and the two !CS lines
// are outputs, MISO and the two card detect bits are inputs, and
// all of those requre pull-ups. We include the two !CS lines
// in the pull-ups because we want their initial state to be high,
// not because they're a pull-up.
#define DDRB_BITS _BV(DDB0) | _BV(DDB1) | _BV(DDB2) | _BV(DDB4) | _BV(DDB5)
#define PULLUP_B_BITS _BV(PORTB3) | _BV(PORTB4) | _BV(PORTB5) | _BV(PORTB6) | _BV(PORTB7)

#define CS_A _BV(PORTB4)
#define CS_B _BV(PORTB5)
#define CD_A _BV(PINB6)
#define CD_B _BV(PINB7)
#define CD_MASK (CD_A | CD_B)
#define CD_REG PINB
#define CS_REG PORTB

// Assert the card CS line, 0 for A, 1 for B
#define ASSERT_CARD(x) (CS_REG &= ~((x)?CS_B:CS_A))
// Deassert both cards (only one will be asserted at a time, but this is ok)
#define DEASSERT_CARDS (CS_REG |= (CS_A | CS_B))

// Port C has the LEDs and button. The button must be pulled-up.
#define DDRC_BITS _BV(DDC4) | _BV(DDC5) | _BV(DDC6)
#define PULLUP_C_BITS _BV(PORTC2)

#define BUTTON_PIN _BV(PINC2)
#define BUTTON_REG PINC

#define LED_REG PORTC
#define LED_RDY _BV(PORTC6)
#define LED_ACT _BV(PORTC5)
#define LED_ERR _BV(PORTC4)

// The entropy source. The bit read is random. It can be read at perhaps
// around 10 kHz or so.

// All port D pins are inputs and none require pull-up.
//#define DDRD_BITS 0
//#define PULLUP_D_BITS 0

#define RNG_PIN _BV(PIND0)
#define RNG_REG PIND

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

