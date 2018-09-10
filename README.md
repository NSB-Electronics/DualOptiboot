# Configuring

_BAUD_: 115200

# Hardware Dependencies

_Processor_: Atmel SAMD20E18 is the only supported processor at this time

_External Flash Memory_: AT25XE011, AT25DF041B


# Compiling

You will need to make a directory in your project called `.local`, in that directory create a file named `config.mk`.
In `config.mk` you will need to define the following symbols:
```shell
SDK_PATH := /path/to/SAM-SDK
```

Then you will just have to call make to compile!
```shell
cd path/to/DualOptiboot
make all
```

# Usage

Booting an image into flash memory requires the Flume AMR tool [AMRConsole](https://github.com/FlumeTech/Flume_AMR/tree/GA/tools/AMRConsole). Ensure your device is connected to a serial port
1. Launch the `AMRConsole` with the command:
```shell
python ./amrConsole.py -p PORTNAME -b 115200
```
2. Power on your device
3. Type in the boot command
```python
-b path/to/someProgram.hex
```
4. Wait for the boot sequence to finish

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
