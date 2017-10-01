

LUFA_PATH=../lufa-LUFA-151115/LUFA

MCU          = atxmega32a4u
ARCH         = XMEGA
F_CPU        = 32000000
F_USB        = 48000000
OPTIMIZATION = s
TARGET       = Orthrus
SRC          = $(TARGET).c AES.c sd.c Descriptors.c SCSI.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
CC_FLAGS     = -I. -DUSE_LUFA_CONFIG_HEADER -Wno-main
LD_FLAGS     =
AVRDUDE_PROGRAMMER = atmelice_pdi

include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_cppcheck.mk
include $(LUFA_PATH)/Build/lufa_doxygen.mk
include $(LUFA_PATH)/Build/lufa_dfu.mk
include $(LUFA_PATH)/Build/lufa_hid.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk
include $(LUFA_PATH)/Build/lufa_atprogram.mk
