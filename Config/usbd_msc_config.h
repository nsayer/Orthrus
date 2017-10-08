/* Auto-generated config file usbd_msc_config.h */
#ifndef USBD_MSC_CONFIG_H
#define USBD_MSC_CONFIG_H

// <<< Use Configuration Wizard in Context Menu >>>

// <h> MSC Device Descriptor

// <o> bcdUSB
// <0x0200=> USB 2.0 version
// <0x0210=> USB 2.1 version
// <id> usb_msc_bcdusb
#ifndef CONF_USB_MSC_BCDUSB
#define CONF_USB_MSC_BCDUSB 0x200
#endif

// <o> bMaxPackeSize0
// <0x0008=> 8 bytes
// <0x0010=> 16 bytes
// <0x0020=> 32 bytes
// <0x0040=> 64 bytes
// <id> usb_msc_bmaxpksz0
#ifndef CONF_USB_MSC_BMAXPKSZ0
#define CONF_USB_MSC_BMAXPKSZ0 0x40
#endif

// <o> idVender <0x0000-0xFFFF>
// <id> usb_msc_idvender
#ifndef CONF_USB_MSC_IDVENDER
#define CONF_USB_MSC_IDVENDER 0x1209
#endif

// <o> idProduct <0x0000-0xFFFF>
// <id> usb_msc_idproduct
#ifndef CONF_USB_MSC_IDPRODUCT
#define CONF_USB_MSC_IDPRODUCT 0xc0c0
#endif

// <o> bcdDevice <0x0000-0xFFFF>
// <id> usb_msc_bcddevice
#ifndef CONF_USB_MSC_BCDDEVICE
#define CONF_USB_MSC_BCDDEVICE 0x300
#endif

#ifndef CONF_USB_MSC_IMANUFACT
#define CONF_USB_MSC_IMANUFACT 0x01
#endif

#ifndef CONF_MSC_LANGUAGE_ID_STR_DESC
#define CONF_MSC_LANGUAGE_ID_STR_DESC 0x04, 0x03, 0x09, 0x04
#endif

#ifndef CONF_MSC_MANUFACTOR_STR_DESC
#define CONF_MSC_MANUFACTOR_STR_DESC                                                                                   \
0x2a, 0x03, 0x47, 0x00, 0x65, 0x00, 0x70, 0x00, 0x70, 0x00, 0x65, 0x00, 0x74, 0x00, 0x74, 0x00, \
0x6f, 0x00, 0x20, 0x00, 0x45, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x72, 0x00, \
0x6f, 0x00, 0x6e, 0x00, 0x69, 0x00, 0x63, 0x00, 0x73, 0x00
#endif

#ifndef CONF_USB_MSC_IPRODUCT
#define CONF_USB_MSC_IPRODUCT 0x02
#endif

#ifndef CONF_MSC_PRODUCT_STR_DESC
#define CONF_MSC_PRODUCT_STR_DESC                                                                                      \
0x10, 0x03, 0x4f, 0x00, 0x72, 0x00, 0x74, 0x00, 0x68, 0x00, 0x72, 0x00, 0x75, 0x00, 0x73, 0x00
#endif

#ifndef CONF_USB_MSC_ISERIALNUM
#define CONF_USB_MSC_ISERIALNUM 0x00
#endif

// </h>

// <h> MSC Configuration Descriptor

#ifndef CONF_USB_MSC_ICONFIG
#define CONF_USB_MSC_ICONFIG 0x00
#endif

// <o> bmAttributes
// <0x80=> Bus power supply, not support for remote wakeup
// <0xA0=> Bus power supply, support for remote wakeup
// <0xC0=> Self powered, not support for remote wakeup
// <0xE0=> Self powered, support for remote wakeup
// <id> usb_msc_bmattri
#ifndef CONF_USB_MSC_BMATTRI
#define CONF_USB_MSC_BMATTRI 0x80
#endif

// <o> bMaxPower <0x00-0xFF>
// <id> usb_msc_bmaxpower
#ifndef CONF_USB_MSC_BMAXPOWER
#define CONF_USB_MSC_BMAXPOWER 0x4b
#endif
// </h>

// <h> MSC Interface Descriptor

#ifndef CONF_USB_MSC_BIFCNUM
#define CONF_USB_MSC_BIFCNUM 0x00
#endif

#ifndef CONF_USB_MSC_IIFC
#define CONF_USB_MSC_IIFC 0x00
#endif

// <o> BULK IN Endpoint Address
// <0x81=> EndpointAddress = 0x81
// <0x82=> EndpointAddress = 0x82
// <0x83=> EndpointAddress = 0x83
// <0x84=> EndpointAddress = 0x84
// <0x85=> EndpointAddress = 0x85
// <0x86=> EndpointAddress = 0x86
// <0x87=> EndpointAddress = 0x87
// <id> usb_msc_bulkin_epaddr
// <i> Please make sure that the setting here is coincide with the endpoint setting in USB device driver.
#ifndef CONF_USB_MSC_BULKIN_EPADDR
#define CONF_USB_MSC_BULKIN_EPADDR 0x81
#endif

// <o> BULK IN Endpoint wMaxPacketSize
// <0x0008=> 8 bytes
// <0x0010=> 16 bytes
// <0x0020=> 32 bytes
// <0x0040=> 64 bytes
// <id> usb_msc_bulkin_maxpksz
// <i> Please make sure that the setting here is coincide with the endpoint setting in USB device driver.
#ifndef CONF_USB_MSC_BULKIN_MAXPKSZ
#define CONF_USB_MSC_BULKIN_MAXPKSZ 0x200
#endif

// <o> BULK OUT Endpoint Address
// <0x01=> EndpointAddress = 0x01
// <0x02=> EndpointAddress = 0x02
// <0x03=> EndpointAddress = 0x03
// <0x04=> EndpointAddress = 0x04
// <0x05=> EndpointAddress = 0x05
// <0x06=> EndpointAddress = 0x06
// <0x07=> EndpointAddress = 0x07
// <id> usb_msc_bulkout_epaddr
// <i> Please make sure that the setting here is coincide with the endpoint setting in USB device driver.
#ifndef CONF_USB_MSC_BULKOUT_EPADDR
#define CONF_USB_MSC_BULKOUT_EPADDR 0x2
#endif

// <o> BULK OUT Endpoint wMaxPacketSize
// <0x0008=> 8 bytes
// <0x0010=> 16 bytes
// <0x0020=> 32 bytes
// <0x0040=> 64 bytes
// <id> usb_msc_bulkout_maxpksz
// <i> Please make sure that the setting here is coincide with the endpoint setting in USB device driver.
#ifndef CONF_USB_MSC_BULKOUT_MAXPKSZ
#define CONF_USB_MSC_BULKOUT_MAXPKSZ 0x200
#endif
// </h>

#ifndef CONF_USB_MSC_MAX_LUN
#define CONF_USB_MSC_MAX_LUN 0x00
#endif

#ifndef CONF_USB_MSC_LUN0_ENABLE

#define CONF_USB_MSC_LUN0_ENABLE 1

#endif

#ifndef CONF_USB_MSC_LUN0_TYPE
#define CONF_USB_MSC_LUN0_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN0_RMB
#define CONF_USB_MSC_LUN0_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN0_ISO
#define CONF_USB_MSC_LUN0_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN0_ECMA
#define CONF_USB_MSC_LUN0_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN0_ANSI
#define CONF_USB_MSC_LUN0_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN0_REPO
#define CONF_USB_MSC_LUN0_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN0_FACTORY
#define CONF_USB_MSC_LUN0_FACTORY 0x47, 0x65, 0x70, 0x70, 0x65, 0x74, 0x74, 0x6f
#endif

#ifndef CONF_USB_MSC_LUN0_PRODUCT
#define CONF_USB_MSC_LUN0_PRODUCT 0x4f, 0x72, 0x74, 0x68, 0x72, 0x73
#endif

#ifndef CONF_USB_MSC_LUN0_PRODUCT_VERSION
#define CONF_USB_MSC_LUN0_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN0_CAPACITY
#define CONF_USB_MSC_LUN0_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN0_BLOCK_SIZE
#define CONF_USB_MSC_LUN0_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN0_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN0_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN0_CAPACITY * 1024 / CONF_USB_MSC_LUN0_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN1_ENABLE

#define CONF_USB_MSC_LUN1_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN1_TYPE
#define CONF_USB_MSC_LUN1_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN1_RMB
#define CONF_USB_MSC_LUN1_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN1_ISO
#define CONF_USB_MSC_LUN1_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN1_ECMA
#define CONF_USB_MSC_LUN1_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN1_ANSI
#define CONF_USB_MSC_LUN1_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN1_REPO
#define CONF_USB_MSC_LUN1_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN1_FACTORY
#define CONF_USB_MSC_LUN1_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN1_PRODUCT
#define CONF_USB_MSC_LUN1_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN1_PRODUCT_VERSION
#define CONF_USB_MSC_LUN1_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN1_CAPACITY
#define CONF_USB_MSC_LUN1_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN1_BLOCK_SIZE
#define CONF_USB_MSC_LUN1_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN1_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN1_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN1_CAPACITY * 1024 / CONF_USB_MSC_LUN1_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN2_ENABLE

#define CONF_USB_MSC_LUN2_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN2_TYPE
#define CONF_USB_MSC_LUN2_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN2_RMB
#define CONF_USB_MSC_LUN2_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN2_ISO
#define CONF_USB_MSC_LUN2_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN2_ECMA
#define CONF_USB_MSC_LUN2_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN2_ANSI
#define CONF_USB_MSC_LUN2_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN2_REPO
#define CONF_USB_MSC_LUN2_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN2_FACTORY
#define CONF_USB_MSC_LUN2_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN2_PRODUCT
#define CONF_USB_MSC_LUN2_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN2_PRODUCT_VERSION
#define CONF_USB_MSC_LUN2_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN2_CAPACITY
#define CONF_USB_MSC_LUN2_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN2_BLOCK_SIZE
#define CONF_USB_MSC_LUN2_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN2_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN2_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN2_CAPACITY * 1024 / CONF_USB_MSC_LUN2_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN3_ENABLE

#define CONF_USB_MSC_LUN3_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN3_TYPE
#define CONF_USB_MSC_LUN3_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN3_RMB
#define CONF_USB_MSC_LUN3_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN3_ISO
#define CONF_USB_MSC_LUN3_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN3_ECMA
#define CONF_USB_MSC_LUN3_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN3_ANSI
#define CONF_USB_MSC_LUN3_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN3_REPO
#define CONF_USB_MSC_LUN3_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN3_FACTORY
#define CONF_USB_MSC_LUN3_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN3_PRODUCT
#define CONF_USB_MSC_LUN3_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN3_PRODUCT_VERSION
#define CONF_USB_MSC_LUN3_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN3_CAPACITY
#define CONF_USB_MSC_LUN3_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN3_BLOCK_SIZE
#define CONF_USB_MSC_LUN3_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN3_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN3_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN3_CAPACITY * 1024 / CONF_USB_MSC_LUN3_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN4_ENABLE

#define CONF_USB_MSC_LUN4_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN4_TYPE
#define CONF_USB_MSC_LUN4_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN4_RMB
#define CONF_USB_MSC_LUN4_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN4_ISO
#define CONF_USB_MSC_LUN4_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN4_ECMA
#define CONF_USB_MSC_LUN4_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN4_ANSI
#define CONF_USB_MSC_LUN4_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN4_REPO
#define CONF_USB_MSC_LUN4_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN4_FACTORY
#define CONF_USB_MSC_LUN4_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN4_PRODUCT
#define CONF_USB_MSC_LUN4_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN4_PRODUCT_VERSION
#define CONF_USB_MSC_LUN4_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN4_CAPACITY
#define CONF_USB_MSC_LUN4_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN4_BLOCK_SIZE
#define CONF_USB_MSC_LUN4_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN4_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN4_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN4_CAPACITY * 1024 / CONF_USB_MSC_LUN4_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN5_ENABLE

#define CONF_USB_MSC_LUN5_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN5_TYPE
#define CONF_USB_MSC_LUN5_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN5_RMB
#define CONF_USB_MSC_LUN5_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN5_ISO
#define CONF_USB_MSC_LUN5_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN5_ECMA
#define CONF_USB_MSC_LUN5_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN5_ANSI
#define CONF_USB_MSC_LUN5_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN5_REPO
#define CONF_USB_MSC_LUN5_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN5_FACTORY
#define CONF_USB_MSC_LUN5_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN5_PRODUCT
#define CONF_USB_MSC_LUN5_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN5_PRODUCT_VERSION
#define CONF_USB_MSC_LUN5_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN5_CAPACITY
#define CONF_USB_MSC_LUN5_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN5_BLOCK_SIZE
#define CONF_USB_MSC_LUN5_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN5_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN5_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN5_CAPACITY * 1024 / CONF_USB_MSC_LUN5_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN6_ENABLE

#define CONF_USB_MSC_LUN6_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN6_TYPE
#define CONF_USB_MSC_LUN6_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN6_RMB
#define CONF_USB_MSC_LUN6_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN6_ISO
#define CONF_USB_MSC_LUN6_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN6_ECMA
#define CONF_USB_MSC_LUN6_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN6_ANSI
#define CONF_USB_MSC_LUN6_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN6_REPO
#define CONF_USB_MSC_LUN6_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN6_FACTORY
#define CONF_USB_MSC_LUN6_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN6_PRODUCT
#define CONF_USB_MSC_LUN6_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN6_PRODUCT_VERSION
#define CONF_USB_MSC_LUN6_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN6_CAPACITY
#define CONF_USB_MSC_LUN6_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN6_BLOCK_SIZE
#define CONF_USB_MSC_LUN6_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN6_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN6_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN6_CAPACITY * 1024 / CONF_USB_MSC_LUN6_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN7_ENABLE

#define CONF_USB_MSC_LUN7_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN7_TYPE
#define CONF_USB_MSC_LUN7_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN7_RMB
#define CONF_USB_MSC_LUN7_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN7_ISO
#define CONF_USB_MSC_LUN7_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN7_ECMA
#define CONF_USB_MSC_LUN7_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN7_ANSI
#define CONF_USB_MSC_LUN7_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN7_REPO
#define CONF_USB_MSC_LUN7_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN7_FACTORY
#define CONF_USB_MSC_LUN7_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN7_PRODUCT
#define CONF_USB_MSC_LUN7_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN7_PRODUCT_VERSION
#define CONF_USB_MSC_LUN7_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN7_CAPACITY
#define CONF_USB_MSC_LUN7_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN7_BLOCK_SIZE
#define CONF_USB_MSC_LUN7_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN7_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN7_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN7_CAPACITY * 1024 / CONF_USB_MSC_LUN7_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN8_ENABLE

#define CONF_USB_MSC_LUN8_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN8_TYPE
#define CONF_USB_MSC_LUN8_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN8_RMB
#define CONF_USB_MSC_LUN8_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN8_ISO
#define CONF_USB_MSC_LUN8_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN8_ECMA
#define CONF_USB_MSC_LUN8_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN8_ANSI
#define CONF_USB_MSC_LUN8_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN8_REPO
#define CONF_USB_MSC_LUN8_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN8_FACTORY
#define CONF_USB_MSC_LUN8_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN8_PRODUCT
#define CONF_USB_MSC_LUN8_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN8_PRODUCT_VERSION
#define CONF_USB_MSC_LUN8_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN8_CAPACITY
#define CONF_USB_MSC_LUN8_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN8_BLOCK_SIZE
#define CONF_USB_MSC_LUN8_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN8_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN8_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN8_CAPACITY * 1024 / CONF_USB_MSC_LUN8_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN9_ENABLE

#define CONF_USB_MSC_LUN9_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN9_TYPE
#define CONF_USB_MSC_LUN9_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN9_RMB
#define CONF_USB_MSC_LUN9_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN9_ISO
#define CONF_USB_MSC_LUN9_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN9_ECMA
#define CONF_USB_MSC_LUN9_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN9_ANSI
#define CONF_USB_MSC_LUN9_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN9_REPO
#define CONF_USB_MSC_LUN9_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN9_FACTORY
#define CONF_USB_MSC_LUN9_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN9_PRODUCT
#define CONF_USB_MSC_LUN9_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN9_PRODUCT_VERSION
#define CONF_USB_MSC_LUN9_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN9_CAPACITY
#define CONF_USB_MSC_LUN9_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN9_BLOCK_SIZE
#define CONF_USB_MSC_LUN9_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN9_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN9_LAST_BLOCK_ADDR                                                                              \
	((uint32_t)CONF_USB_MSC_LUN9_CAPACITY * 1024 / CONF_USB_MSC_LUN9_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN10_ENABLE

#define CONF_USB_MSC_LUN10_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN10_TYPE
#define CONF_USB_MSC_LUN10_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN10_RMB
#define CONF_USB_MSC_LUN10_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN10_ISO
#define CONF_USB_MSC_LUN10_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN10_ECMA
#define CONF_USB_MSC_LUN10_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN10_ANSI
#define CONF_USB_MSC_LUN10_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN10_REPO
#define CONF_USB_MSC_LUN10_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN10_FACTORY
#define CONF_USB_MSC_LUN10_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN10_PRODUCT
#define CONF_USB_MSC_LUN10_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN10_PRODUCT_VERSION
#define CONF_USB_MSC_LUN10_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN10_CAPACITY
#define CONF_USB_MSC_LUN10_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN10_BLOCK_SIZE
#define CONF_USB_MSC_LUN10_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN10_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN10_LAST_BLOCK_ADDR                                                                             \
	((uint32_t)CONF_USB_MSC_LUN10_CAPACITY * 1024 / CONF_USB_MSC_LUN10_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN11_ENABLE

#define CONF_USB_MSC_LUN11_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN11_TYPE
#define CONF_USB_MSC_LUN11_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN11_RMB
#define CONF_USB_MSC_LUN11_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN11_ISO
#define CONF_USB_MSC_LUN11_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN11_ECMA
#define CONF_USB_MSC_LUN11_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN11_ANSI
#define CONF_USB_MSC_LUN11_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN11_REPO
#define CONF_USB_MSC_LUN11_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN11_FACTORY
#define CONF_USB_MSC_LUN11_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN11_PRODUCT
#define CONF_USB_MSC_LUN11_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN11_PRODUCT_VERSION
#define CONF_USB_MSC_LUN11_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN11_CAPACITY
#define CONF_USB_MSC_LUN11_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN11_BLOCK_SIZE
#define CONF_USB_MSC_LUN11_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN11_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN11_LAST_BLOCK_ADDR                                                                             \
	((uint32_t)CONF_USB_MSC_LUN11_CAPACITY * 1024 / CONF_USB_MSC_LUN11_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN12_ENABLE

#define CONF_USB_MSC_LUN12_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN12_TYPE
#define CONF_USB_MSC_LUN12_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN12_RMB
#define CONF_USB_MSC_LUN12_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN12_ISO
#define CONF_USB_MSC_LUN12_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN12_ECMA
#define CONF_USB_MSC_LUN12_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN12_ANSI
#define CONF_USB_MSC_LUN12_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN12_REPO
#define CONF_USB_MSC_LUN12_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN12_FACTORY
#define CONF_USB_MSC_LUN12_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN12_PRODUCT
#define CONF_USB_MSC_LUN12_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN12_PRODUCT_VERSION
#define CONF_USB_MSC_LUN12_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN12_CAPACITY
#define CONF_USB_MSC_LUN12_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN12_BLOCK_SIZE
#define CONF_USB_MSC_LUN12_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN12_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN12_LAST_BLOCK_ADDR                                                                             \
	((uint32_t)CONF_USB_MSC_LUN12_CAPACITY * 1024 / CONF_USB_MSC_LUN12_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN13_ENABLE

#define CONF_USB_MSC_LUN13_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN13_TYPE
#define CONF_USB_MSC_LUN13_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN13_RMB
#define CONF_USB_MSC_LUN13_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN13_ISO
#define CONF_USB_MSC_LUN13_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN13_ECMA
#define CONF_USB_MSC_LUN13_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN13_ANSI
#define CONF_USB_MSC_LUN13_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN13_REPO
#define CONF_USB_MSC_LUN13_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN13_FACTORY
#define CONF_USB_MSC_LUN13_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN13_PRODUCT
#define CONF_USB_MSC_LUN13_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN13_PRODUCT_VERSION
#define CONF_USB_MSC_LUN13_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN13_CAPACITY
#define CONF_USB_MSC_LUN13_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN13_BLOCK_SIZE
#define CONF_USB_MSC_LUN13_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN13_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN13_LAST_BLOCK_ADDR                                                                             \
	((uint32_t)CONF_USB_MSC_LUN13_CAPACITY * 1024 / CONF_USB_MSC_LUN13_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN14_ENABLE

#define CONF_USB_MSC_LUN14_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN14_TYPE
#define CONF_USB_MSC_LUN14_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN14_RMB
#define CONF_USB_MSC_LUN14_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN14_ISO
#define CONF_USB_MSC_LUN14_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN14_ECMA
#define CONF_USB_MSC_LUN14_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN14_ANSI
#define CONF_USB_MSC_LUN14_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN14_REPO
#define CONF_USB_MSC_LUN14_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN14_FACTORY
#define CONF_USB_MSC_LUN14_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN14_PRODUCT
#define CONF_USB_MSC_LUN14_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN14_PRODUCT_VERSION
#define CONF_USB_MSC_LUN14_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN14_CAPACITY
#define CONF_USB_MSC_LUN14_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN14_BLOCK_SIZE
#define CONF_USB_MSC_LUN14_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN14_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN14_LAST_BLOCK_ADDR                                                                             \
	((uint32_t)CONF_USB_MSC_LUN14_CAPACITY * 1024 / CONF_USB_MSC_LUN14_BLOCK_SIZE - 1)
#endif

#ifndef CONF_USB_MSC_LUN15_ENABLE

#define CONF_USB_MSC_LUN15_ENABLE 0

#endif

#ifndef CONF_USB_MSC_LUN15_TYPE
#define CONF_USB_MSC_LUN15_TYPE 0x00
#endif

#ifndef CONF_USB_MSC_LUN15_RMB
#define CONF_USB_MSC_LUN15_RMB 0x01
#endif

#ifndef CONF_USB_MSC_LUN15_ISO
#define CONF_USB_MSC_LUN15_ISO 0x00
#endif

#ifndef CONF_USB_MSC_LUN15_ECMA
#define CONF_USB_MSC_LUN15_ECMA 0x00
#endif

#ifndef CONF_USB_MSC_LUN15_ANSI
#define CONF_USB_MSC_LUN15_ANSI 0x00
#endif

#ifndef CONF_USB_MSC_LUN15_REPO
#define CONF_USB_MSC_LUN15_REPO 0x01
#endif

#ifndef CONF_USB_MSC_LUN15_FACTORY
#define CONF_USB_MSC_LUN15_FACTORY 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN15_PRODUCT
#define CONF_USB_MSC_LUN15_PRODUCT 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN15_PRODUCT_VERSION
#define CONF_USB_MSC_LUN15_PRODUCT_VERSION 0x00, 0x00, 0x00, 0x00
#endif

#ifndef CONF_USB_MSC_LUN15_CAPACITY
#define CONF_USB_MSC_LUN15_CAPACITY 0x16
#endif

#ifndef CONF_USB_MSC_LUN15_BLOCK_SIZE
#define CONF_USB_MSC_LUN15_BLOCK_SIZE 512
#endif

#ifndef CONF_USB_MSC_LUN15_LAST_BLOCK_ADDR
#define CONF_USB_MSC_LUN15_LAST_BLOCK_ADDR                                                                             \
	((uint32_t)CONF_USB_MSC_LUN15_CAPACITY * 1024 / CONF_USB_MSC_LUN15_BLOCK_SIZE - 1)
#endif

// <<< end of configuration section >>>

#endif // USBD_MSC_CONFIG_H
