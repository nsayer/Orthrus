/**
 * \file
 *
 * \brief USB Device Stack MSC Function Descriptor Setting.
 *
 * Copyright (C) 2016 Atmel Corporation. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 * Atmel AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#ifndef USBDF_MSC_DESC_H_
#define USBDF_MSC_DESC_H_

#include "usb_protocol.h"
#include "usb_protocol_msc.h"
#include "usbd_msc_config.h"

#define MSC_DEV_DESC                                                                                                   \
	USB_DEV_DESC_BYTES(CONF_USB_MSC_BCDUSB,                                                                            \
	                   0x00,                                                                                           \
	                   0x00,                                                                                           \
	                   0x00,                                                                                           \
	                   CONF_USB_MSC_BMAXPKSZ0,                                                                         \
	                   CONF_USB_MSC_IDVENDER,                                                                          \
	                   CONF_USB_MSC_IDPRODUCT,                                                                         \
	                   CONF_USB_MSC_BCDDEVICE,                                                                         \
	                   CONF_USB_MSC_IMANUFACT,                                                                         \
	                   CONF_USB_MSC_IPRODUCT,                                                                          \
	                   CONF_USB_MSC_ISERIALNUM,                                                                        \
	                   0x01)

#define MSC_CFG_DESC                                                                                                   \
	USB_CONFIG_DESC_BYTES(32, 1, 0x01, CONF_USB_MSC_ICONFIG, CONF_USB_MSC_BMATTRI, CONF_USB_MSC_BMAXPOWER)

#define MSC_IFACE_DESCES                                                                                               \
	USB_IFACE_DESC_BYTES(CONF_USB_MSC_BIFCNUM, 0x00, 2, 0x08, 0x06, 0x50, CONF_USB_MSC_IIFC)                           \
	, USB_ENDP_DESC_BYTES(CONF_USB_MSC_BULKOUT_EPADDR, 2, CONF_USB_MSC_BULKOUT_MAXPKSZ, 0),                            \
	    USB_ENDP_DESC_BYTES(CONF_USB_MSC_BULKIN_EPADDR, 2, CONF_USB_MSC_BULKIN_MAXPKSZ, 0)

/** USB Device descriptors and configuration descriptors */
#define MSC_DESCES_LS_FS MSC_DEV_DESC, MSC_CFG_DESC, MSC_IFACE_DESCES, CONF_MSC_LANGUAGE_ID_STR_DESC, CONF_MSC_MANUFACTOR_STR_DESC, CONF_MSC_PRODUCT_STR_DESC

#endif /* USBDF_MSC_DESC_H_ */
