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
#include <Crypto.h>
#include <hpl_delay.h>
#include <usb_start.h>
#include <hpl_pmc_config.h>


enum volume_states { NO_CARDS, ERROR, OK, UNINITIALIZED };
enum button_states { UP, DOWN, IGNORING };
	
static enum volume_states state;
static enum button_states button_state;

static struct timer_task milli_task;

static uint32_t button_start;

// 128 bit unique ID
uint8_t unique_id[16];

uint8_t usb_serial_string[32]; // 15 chars from unique ID, plus length & type.

// Millisecond counter - used for dealing with the button
volatile uint32_t millis;

static void milli_timer_cb(const struct timer_task *const timer_task) {
	millis++;
}

__attribute__((noinline)) __attribute__((section(".itcm"))) static void fetch_unique_id() {

	memset(unique_id, 0, sizeof(unique_id));
	EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_STUI);
	while ((EFC->EEFC_FSR & EEFC_FSR_FRDY) == EEFC_FSR_FRDY); // Oddly, you have to wait for FRDY to *clear*
	memcpy(unique_id, (void*)0x00400000, sizeof(unique_id));
	EFC->EEFC_FCR = (EEFC_FCR_FKEY_PASSWD | EEFC_FCR_FCMD_SPUI);
	while ((EFC->EEFC_FSR & EEFC_FSR_FRDY) != EEFC_FSR_FRDY);

}

__attribute__((noinline)) __attribute__((section(".itcm"))) static void do_main_loop(void) {
	while (1) {
		// We're alive as long as we keep coming through the main loop.
		wdt_feed(&WDT_0);
		
		bool cards_in = !gpio_get_pin_level(CARD_DETECT_A) && !gpio_get_pin_level(CARD_DETECT_B);
		if (cards_in) {
			if (state == NO_CARDS) {
				// cards have just been inserted. Get to work!
				if (!init_cards()) {
					state = ERROR;
					goto error;
				}
				if (!prepVolume()) {
					state = UNINITIALIZED;
					goto error;
				}							
				state = OK;
				gpio_set_pin_level(LED_RDY, true);
				gpio_set_pin_level(LED_ERR, false);
				set_state(READY);
				continue;
error:
				gpio_set_pin_level(LED_ERR, true);
				gpio_set_pin_level(LED_RDY, false);
				continue;			
			}
		} else {
			if (state != NO_CARDS) {
				// cards have just been removed. Turn everything off.
				shutdown_cards();
				clearKeys();
				if (state == OK) {
					set_state(NOT_READY);
				}
				state = NO_CARDS;
				gpio_set_pin_level(LED_ERR, false);
				gpio_set_pin_level(LED_RDY, false);
				continue;
			}	
		}
		
		// Handle the button being held down.
		if (button_state == DOWN) {
			if (millis - button_start > 5000) { // 5 seconds
				button_state = IGNORING;
				gpio_set_pin_level(LED_ERR, false);
				gpio_set_pin_level(LED_RDY, false);
				if (initVolume()) {
					gpio_set_pin_level(LED_ERR, false);
					gpio_set_pin_level(LED_RDY, true);
					state = OK;
					set_state(READY);
				} else {
					gpio_set_pin_level(LED_ERR, true);
					gpio_set_pin_level(LED_RDY, false);
					state = ERROR;
					set_state(NOT_READY);
				}
			} else {
				// Make the ERROR LED blink as a warning
				gpio_set_pin_level(LED_ERR, (((millis - button_start) / 250) % 2)?false:true);
			}
		}
		
		bool button = !gpio_get_pin_level(BUTTON) || !gpio_get_pin_level(BUTTON_ALT);
		
		if (button) {
			switch(button_state) {
				case UP:
					if (state == UNINITIALIZED || state == OK) {
						button_state = DOWN;
						button_start = millis;
					} else {
						button_state = IGNORING;
					}
					break;
				case IGNORING:
				case DOWN:
					// nothing has changed
					break;
			}
		} else {
			if (button_state != UP) {
				if (state == OK) {
					gpio_set_pin_level(LED_RDY, true);
					gpio_set_pin_level(LED_ERR, false);
				} else if (state == ERROR || state == UNINITIALIZED) {
					gpio_set_pin_level(LED_RDY, false);
					gpio_set_pin_level(LED_ERR, true);
				}
			}
			button_state = UP;
		}
		disk_task();
	}
}

int main(void)
{

	SCB_EnableICache();
	//SCB_EnableDCache(); //No... we need to sprinkle cache invalidations to do this.

	// Start doesn't do this for you, it seems.
	#if (CONF_XOSC20M_SELECTOR == 16000000)
	UTMI->UTMI_CKTRIM &= ~UTMI_CKTRIM_FREQ_Msk;
	UTMI->UTMI_CKTRIM |= UTMI_CKTRIM_FREQ(UTMI_CKTRIM_FREQ_XTAL16);
	#endif

	fetch_unique_id();
	memset(usb_serial_string, 0, sizeof(usb_serial_string)); // clear it out
	usb_serial_string[0] = sizeof(usb_serial_string); // length
	usb_serial_string[1] = USB_DT_STRING; // descriptor type is string
	for(int i = 0; i < 15; i++)
	usb_serial_string[(i + 1) * 2] = unique_id[i + 1];
	
	
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();

	aes_sync_enable(&CRYPTOGRAPHY_0);

	rand_sync_enable(&RAND_0);
	
	timer_start(&TIMER_0);
	milli_task.cb = milli_timer_cb;
	milli_task.interval = 1;
	milli_task.mode = TIMER_TASK_REPEAT;
	timer_add_task(&TIMER_0, &milli_task);
	
	state = NO_CARDS;
	button_state = UP;
	set_state(NOT_READY);
	
	delay_ms(25); // Can't feed the watchdoog too soon after enabling it.
	do_main_loop();
}
