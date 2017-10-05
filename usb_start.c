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

#include "atmel_start.h"
#include "usb_start.h"
#include <Crypto.h>
#include <MCI.h>

/* use a mock buffer for clarify transfer process */
#define MOCK_BUF_AMOUNT 6
#define MOCK_BUF_SIZE 512
#define MOCK_DATA_TYPE uint8_t *

static enum usb_volume_state vol_state;

enum xfer_dirs { READ, WRITE };

static enum xfer_dirs xfer_dir;
static uint32_t xfer_addr;
static int32_t num_blocks;
static bool xfer_busy;

COMPILER_ALIGNED(4) uint8_t blockbuf[SECTOR_SIZE];

static uint8_t single_desc_bytes[] = {
    /* Device descriptors and Configuration descriptors list. */
    MSC_DESCES_LS_FS,
};

static struct usbd_descriptors single_desc = {single_desc_bytes, single_desc_bytes + sizeof(single_desc_bytes)};

/* Ctrl endpoint buffer */
static uint8_t ctrl_buffer[64];

#define DISK_INFORMATION(n)                                                                                            \
	{                                                                                                                  \
		CONF_USB_MSC_LUN##n##_TYPE, (CONF_USB_MSC_LUN##n##_RMB << 7),                                                  \
		    ((CONF_USB_MSC_LUN##n##_ISO << 6) + (CONF_USB_MSC_LUN##n##_ECMA << 3) + CONF_USB_MSC_LUN##n##_ANSI),       \
		    CONF_USB_MSC_LUN##n##_REPO, 31, 0x00, 0x00, 0x00, CONF_USB_MSC_LUN##n##_FACTORY,                           \
		    CONF_USB_MSC_LUN##n##_PRODUCT, CONF_USB_MSC_LUN##n##_PRODUCT_VERSION                                       \
	}

/* Inquiry Information */
static uint8_t inquiry_info[CONF_USB_MSC_MAX_LUN + 1][36] = { DISK_INFORMATION(0) };

static bool in_attention;

void set_state(enum usb_volume_state st_in) {
	vol_state = st_in;
	in_attention = true;
}
	
/**
 * \brief Eject Disk
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static int32_t disk_eject(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return ERR_NOT_FOUND;
	}
	// No, we can't just spit the cards out for you.
	return ERR_UNSUPPORTED_OP;
}

static int32_t check_state() {
	int32_t ret = ERR_NOT_FOUND;
	if (in_attention) {
		ret = ERR_ABORTED;
		} else if (vol_state == NOT_READY) {
		ret = ERR_NOT_READY;
		} else {
		ret = ERR_NONE;
	}
	return ret;
}

/**
 * \brief Inquiry whether Disk is ready
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static int32_t disk_is_ready(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return ERR_NOT_FOUND;
	}
	int32_t ret = check_state();
	// Once we've been checked here, an ATTENTION state is cleared.
	in_attention = false;
	return ret;
}

/**
 * \brief Callback invoked when a new read blocks command received
 * \param[in] lun logic unit number
 * \param[in] addr start address of disk to be read
 * \param[in] nblocks block amount to be read
 * \return Operation status.
 */
static int32_t msc_new_read(uint8_t lun, uint32_t addr, uint32_t nblocks)
{
	int32_t ret = check_state();
	if (ret != ERR_NONE) return ret;
	
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return ERR_NOT_READY;
	}

	xfer_dir  = READ;
	xfer_addr = addr;
	num_blocks = nblocks;
	xfer_busy = false;
	
	return ERR_NONE;
}

/**
 * \brief Callback invoked when a new write blocks command received
 * \param[in] lun logic unit number
 * \param[in] addr start address of disk to be written
 * \param[in] nblocks block amount to be written
 * \return Operation status.
 */
static int32_t msc_new_write(uint8_t lun, uint32_t addr, uint32_t nblocks)
{
	int32_t ret = check_state();
	if (ret != ERR_NONE) return ret;

	if (lun > CONF_USB_MSC_MAX_LUN) {
		return ERR_NOT_READY;
	}
	xfer_dir  = WRITE;
	xfer_addr = addr;
	num_blocks = nblocks;
	mscdf_xfer_blocks(false, blockbuf, 1);
	xfer_busy = true;

	return ERR_NONE;
}

/**
 * \brief Callback invoked when a blocks transfer is done
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static int32_t msc_xfer_done(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN) {
		return ERR_DENIED;
	}

	xfer_busy = false;
	
	return ERR_NONE;
}

/**
 * \brief Disk loop
 */
void disk_task(void)
{
	if (xfer_busy) return; // USB is busy
	if (num_blocks < 0) return; // the whole system is idle
	if (xfer_dir == READ) {
		xfer_busy = true;
		readVolumeBlock(xfer_addr++, blockbuf);
		if (ERR_NONE != mscdf_xfer_blocks(true, blockbuf, 1))
			ASSERT(false);
		if (--num_blocks == 0) {
			// we're done (once we're no longer busy).
			num_blocks = -1;
		}
		return;
	} else {
		// We previously did a transfer into blockbuf
		writeVolumeBlock(xfer_addr++, blockbuf);
		if (--num_blocks > 0) {
			xfer_busy = true;
			if (ERR_NONE != mscdf_xfer_blocks(false, blockbuf, 1))
				ASSERT(false);
		} else {
			// This special call tells the MSC system that the block
			// is committed and the ACK can be sent to the host.
			xfer_busy = true;
			volatile uint8_t result = mscdf_xfer_blocks(false, blockbuf, 0);
			if (ERR_NONE != result)
				ASSERT(false);
			num_blocks = -1;
			xfer_busy = false;
		}
	}
}

/**
 * \brief Callback invoked when inquiry data command received
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static uint8_t *msc_inquiry_info(uint8_t lun)
{
        if (lun > CONF_USB_MSC_MAX_LUN) {
                return NULL;
        } else {
                num_blocks = -1;
                xfer_busy    = false;
                return &inquiry_info[lun][0];
        }
}

static uint8_t cap_buffer[8];

/**
 * \brief Callback invoked when read format capacities command received
 * \param[in] lun logic unit number
 * \return Operation status.
 */
static uint8_t *msc_get_capacity(uint8_t lun)
{
	if (lun > CONF_USB_MSC_MAX_LUN || vol_state != READY) {
		return NULL;
	} else {
		cap_buffer[0] = (uint8_t)((volume_size - 1) >> 24);
		cap_buffer[1] = (uint8_t)((volume_size - 1) >> 16);
		cap_buffer[2] = (uint8_t)((volume_size - 1) >> 8);
		cap_buffer[3] = (uint8_t)((volume_size - 1) >> 0);
		cap_buffer[4] = (uint8_t)(SECTOR_SIZE >> 24);
		cap_buffer[5] = (uint8_t)(SECTOR_SIZE >> 16);
		cap_buffer[6] = (uint8_t)(SECTOR_SIZE >> 8);
		cap_buffer[7] = (uint8_t)(SECTOR_SIZE >> 0);
		return cap_buffer;
	}
}

/**
 * \brief USB MSC Init
 */
void usbd_msc_init(void)
{
	/* usb stack init */
	usbdc_init(ctrl_buffer);

	/* usbdc_register_funcion inside */
	mscdf_init(CONF_USB_MSC_MAX_LUN);
}

void usb_init(void)
{	
	xfer_busy = false;
	num_blocks = -1;
	vol_state = NOT_READY;
	
	usbd_msc_init();
	mscdf_register_callback(MSCDF_CB_INQUIRY_DISK, (FUNC_PTR)msc_inquiry_info);
	mscdf_register_callback(MSCDF_CB_GET_DISK_CAPACITY, (FUNC_PTR)msc_get_capacity);
	mscdf_register_callback(MSCDF_CB_START_READ_DISK, (FUNC_PTR)msc_new_read);
	mscdf_register_callback(MSCDF_CB_START_WRITE_DISK, (FUNC_PTR)msc_new_write);
	mscdf_register_callback(MSCDF_CB_EJECT_DISK, (FUNC_PTR)disk_eject);
	mscdf_register_callback(MSCDF_CB_TEST_DISK_READY, (FUNC_PTR)disk_is_ready);
	mscdf_register_callback(MSCDF_CB_XFER_BLOCKS_DONE, (FUNC_PTR)msc_xfer_done);
	usbdc_start(&single_desc);
	usbdc_attach();
}
