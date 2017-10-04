/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */
#ifndef USB_DEVICE_MAIN_H
#define USB_DEVICE_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "mscdf.h"
#include "mscdf_desc.h"

void usbd_msc_init(void);
void usbd_msc_example(void);

/**
 * \berif Initialize USB
 */
void usb_init(void);

enum usb_volume_state {NOT_READY, READY, BOUNCING};

void set_state(enum usb_volume_state state);

// call this from the main event loop
void disk_task(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // USB_DEVICE_MAIN_H
