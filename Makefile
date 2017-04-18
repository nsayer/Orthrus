

LUFA_PATH=/Users/nsayer/Downloads/lufa-LUFA-151115/LUFA

MCU          = atmega32u2
ARCH         = AVR8
#BOARD        = USBKEY
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = Orthrus
SRC          = $(TARGET).c AES.c sd.c Descriptors.c SCSI.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
CC_FLAGS     = -I. -DUSE_LUFA_CONFIG_HEADER -Wno-main
LD_FLAGS     =
AVRDUDE_PROGRAMMER = usbtiny
AVRDUDE_FLAGS = -B 0.1

include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
