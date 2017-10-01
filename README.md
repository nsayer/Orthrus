Orthrus
=======

This is the source code repository for https://hackaday.io/project/20772-orthrus

This code is for the V3 variant. It is built in Atmel Studio 7 and is based on
Atmel Start. You use Atmel Start with the included .atstart file, and then overlay
the files contained here on top to make a complete, buildable firmware image.

You can load that image using SAM-BA by shorting the ERASE jumper, then using
SAM-BA over USB to load the image into place and set the GPNVM bits to 0x03 (boot
from flash and security bit on). That will lock the firmware in and prevent debug
access until and unless an ERASE is performed. You can also load the code in via
SWD over the JTAG connector. Again, setting the security bit after flashing is
recommended for production use.

The V2 directory contains the old LUFA based code for the ATXMega32u4 hardware. The
old README content for that is here:

To build, fetch LUFA and unzip it alongside the location of this source tree
(or change the LUFA_PATH in the makefile to point to it).

make all to build the code.
make avrdude to use AVRDUDE to write it to the device (you may need to adjust
the AVRDUDE values in the Makefile). For more information about the build
system, look into the LUFA documentation.

For more info on LUFA, see:

Web:    http://lufa-lib.org/

GitHub: http://github.com/abcminiuser/lufa

