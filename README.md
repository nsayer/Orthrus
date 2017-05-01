Orthrus
=======

This is the source code repository for https://hackaday.io/project/20772-orthrus

To build, fetch LUFA and unzip it alongside the location of this source tree
(or change the LUFA_PATH in the makefile to point to it).

make all to build the code.
make avrdude to use AVRDUDE to write it to the device (you may need to adjust
the AVRDUDE values in the Makefile). For more information about the build
system, look into the LUFA documentation.

For more info on LUFA, see:

Web:    http://lufa-lib.org/

GitHub: http://github.com/abcminiuser/lufa

