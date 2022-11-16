# Configuring

_BAUD_: 230400

# Hardware Dependencies

_Processor_: Atmel SAMD21G18 is the only supported processor at this time

_External Flash Memory_: AT25XE011, AT25DF041B, MX25R8035F, MX25R6435F, W25X40


# Compiling

You will need to make a directory in your project called `.local`, in that directory create a file named after your local host `.mk` You can always discover your hostname by typing `hostname` into your command line. You will need the following symbols in your local host makefile:
```shell
################################### ATSAMD21 ###################################
SAM_SDK_DIR     := /home/warren/sam-sdk
ARMBIN          := $(SAM_SDK_DIR)/toolchain/bin/
SAM_ARDUINO_DIR := $(SAM_SDK_DIR)/arduino
################################################################################
```

Then you will just have to call make to compile!
```shell
cd path/to/DualOptiboot
make all
```

# Image Check

This Optiboot version is modified to add the capability of reflashing
from an external SPI flash memory chip.
Summary of how this Optiboot version works:
* it looks for an external flash chip
* if one is found (SPI returns valid data) it will further look for a new sketch
flash image signature and size
* starting at address 0:   FLXIMG:NNNN:
* where:
   - 'FLXIMG' is fixed signature indicating FLASH chip contains a valid new
   flash image to be burned
   - 'NNNN' are 4 size bytes (uint_16) indicating how long the new flash image is
   (how many bytes to read), max 4,294,967,295 Bytes
   - ':' colons have fixed positions (delimiters)
* starting at adress 256: XXXX..
* where:
   - 'XXXX...' is the firmware image that is to be burned
- if no valid signature/size are found, it will check for an existing image that
has already been burned, if there is one it will run it. If there is no image it
will listen on a serial port for an incoming image.
