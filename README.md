Orthrus
=======

This is the source code repository for https://hackaday.io/project/20772-orthrus

This code is for the V3 variant. It is built in Atmel Studio 7 and is based on
Atmel Start. You use Atmel Start with the included .atstart file, and then overlay
the files contained here on top to make a complete, buildable firmware image.

You can load that image using SAM-BA by shorting the ERASE jumper, then using
SAM-BA over USB to load the image into place and set the GPNVM bits to 0x103 (boot
from flash, security bit on, and TCM config 2). That will lock the firmware in and
prevent debug access until and unless an ERASE is performed. You can also load the
code in via SWD over the JTAG connector. Again, setting the security bit after flashing
is recommended for production use.

There is also a single .java file. This is a standalone tool that will decrypt an Orthrus
volume given two card image files. It's both a correctness proof for the firmware and
a hardware failure recovery tool.

A code signing certificate has been checked in here as well. Released firmware won't
be checked into GitHub, but will be available for download on the Hackaday project page.
Released firmware will have a signature file alongside both the certificate and the
binary image. Those who are cautious (or paranoid) will want to validate the signature
and insure that the certificate matches what's checked in here (if you're extra cautious,
you can contact the author for an offline verification).

V2
--

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

