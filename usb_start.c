/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
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

static uint8_t blockbuf[SECTOR_SIZE];

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

#define DISK_CAPACITY(n)                                                                                               \
	{                                                                                                                  \
		(uint8_t)(CONF_USB_MSC_LUN##n##_LAST_BLOCK_ADDR >> 24),                                                        \
		    (uint8_t)(CONF_USB_MSC_LUN##n##_LAST_BLOCK_ADDR >> 16),                                                    \
		    (uint8_t)(CONF_USB_MSC_LUN##n##_LAST_BLOCK_ADDR >> 8),                                                     \
		    (uint8_t)(CONF_USB_MSC_LUN##n##_LAST_BLOCK_ADDR >> 0),                                                     \
		    (uint8_t)((uint32_t)(CONF_USB_MSC_LUN##n##_BLOCK_SIZE) >> 24),                                             \
		    (uint8_t)((uint32_t)(CONF_USB_MSC_LUN##n##_BLOCK_SIZE) >> 16),                                             \
		    (uint8_t)((uint32_t)(CONF_USB_MSC_LUN##n##_BLOCK_SIZE) >> 8),                                              \
		    (uint8_t)((uint32_t)(CONF_USB_MSC_LUN##n##_BLOCK_SIZE) >> 0)                                               \
	}

/* Inquiry Information */
static uint8_t inquiry_info[CONF_USB_MSC_MAX_LUN + 1][36] = {DISK_INFORMATION(0),
#if CONF_USB_MSC_LUN1_ENABLE == 1
                                                             DISK_INFORMATION(1),
#endif
#if CONF_USB_MSC_LUN2_ENABLE == 1
                                                             DISK_INFORMATION(2),
#endif
#if CONF_USB_MSC_LUN3_ENABLE == 1
                                                             DISK_INFORMATION(3),
#endif
#if CONF_USB_MSC_LUN4_ENABLE == 1
                                                             DISK_INFORMATION(4),
#endif
#if CONF_USB_MSC_LUN5_ENABLE == 1
                                                             DISK_INFORMATION(5),
#endif
#if CONF_USB_MSC_LUN6_ENABLE == 1
                                                             DISK_INFORMATION(6),
#endif
#if CONF_USB_MSC_LUN7_ENABLE == 1
                                                             DISK_INFORMATION(7),
#endif
#if CONF_USB_MSC_LUN8_ENABLE == 1
                                                             DISK_INFORMATION(8),
#endif
#if CONF_USB_MSC_LUN9_ENABLE == 1
                                                             DISK_INFORMATION(9),
#endif
#if CONF_USB_MSC_LUN10_ENABLE == 1
                                                             DISK_INFORMATION(10),
#endif
#if CONF_USB_MSC_LUN11_ENABLE == 1
                                                             DISK_INFORMATION(11),
#endif
#if CONF_USB_MSC_LUN12_ENABLE == 1
                                                             DISK_INFORMATION(12),
#endif
#if CONF_USB_MSC_LUN13_ENABLE == 1
                                                             DISK_INFORMATION(13),
#endif
#if CONF_USB_MSC_LUN14_ENABLE == 1
                                                             DISK_INFORMATION(14),
#endif
#if CONF_USB_MSC_LUN15_ENABLE == 1
                                                             DISK_INFORMATION(15)
#endif
};

void set_state(enum usb_volume_state st_in) {
	vol_state = st_in;
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
	int32_t ret = (vol_state == READY)?ERR_NONE:ERR_NOT_READY;
	// Once we've returned a false once, we can go back to being READY.
	if (vol_state == BOUNCING) vol_state = READY;
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
	if (lun > CONF_USB_MSC_MAX_LUN || vol_state != READY) {
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
	if (lun > CONF_USB_MSC_MAX_LUN || vol_state != READY) {
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
		readVolumeBlock(xfer_addr++, blockbuf);
		if (ERR_NONE != mscdf_xfer_blocks(true, blockbuf, 1))
			ASSERT(false);
		xfer_busy = true;
		if (--num_blocks == 0) {
			// we're done (once we're no longer busy).
			num_blocks = -1;
		}
		return;
	} else {
		// We previously did a transfer into blockbuf
		writeVolumeBlock(xfer_addr++, blockbuf);
		if (--num_blocks >= 0) {
			if (ERR_NONE != mscdf_xfer_blocks(false, blockbuf, 1))
				ASSERT(false);
			xfer_busy = true;
		} else {
			if (ERR_NONE != mscdf_xfer_blocks(false, blockbuf, 0))
				ASSERT(false);
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
        if (lun > CONF_USB_MSC_MAX_LUN || vol_state != READY) {
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
