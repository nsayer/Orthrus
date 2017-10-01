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

enum volume_states { NO_CARDS, ERROR, READY, UNINITIALIZED };
enum button_states { UP, DOWN, IGNORING };
	
enum volume_states state;
enum button_states button_state;

struct timer_task milli_task;

// Millisecond counter - used for dealing with the button
volatile uint32_t millis;
uint32_t button_start;

static void milli_timer_cb(const struct timer_task *const timer_task) {
	millis++;
}

int main(void)
{
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

	while (1) {
		
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
								
				state = READY;
				gpio_set_pin_level(LED_RDY, true);
				gpio_set_pin_level(LED_ERR, false);
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
				if (state == READY) {
					// XXX do something to forcibly unmount the volume
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
				if (state == READY) {
					// XXX Have to do something in here to forcibly unmount.
				}
				if (initVolume()) {
					gpio_set_pin_level(LED_ERR, false);
					gpio_set_pin_level(LED_RDY, true);
					state = READY;
				} else {
					gpio_set_pin_level(LED_ERR, true);
					gpio_set_pin_level(LED_RDY, false);
					state = ERROR;
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
					if (state == UNINITIALIZED || state == READY) {
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
				if (state == READY) {
					gpio_set_pin_level(LED_RDY, true);
					gpio_set_pin_level(LED_ERR, false);
				} else if (state == ERROR || state == UNINITIALIZED) {
					gpio_set_pin_level(LED_RDY, false);
					gpio_set_pin_level(LED_ERR, true);
				}
			}
			button_state = UP;
		}

/*
		usb_task();
		disk_task();
*/
	}
}
