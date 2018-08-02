# Makefile for ATmegaBOOT
# E.Lins, 18.7.2005
# $Id$
#
# Instructions
#
# To make bootloader .hex file:
# make diecimila
# make lilypad
# make ng
# etc...
#
# To burn bootloader .hex file:
# make diecimila_isp
# make lilypad_isp
# make ng_isp
# etc...
#
# Edit History
# 201303xx: WestfW: Major Makefile restructuring.
#                   Allows options on Make command line "make xx LED=B3"
#                   (see also pin_defs.h)
#                   Divide into "chip" targets and "board" targets.
#                   Most boards are (recursive) board targets with options.
#                   Move isp target to separate makefile (fixes m8 EFUSE)
#                   Some (many) targets will now be rebuilt when not
#                     strictly necessary, so that options will be included.
#                     (any "make" with options will always compile.)
#                   Set many variables with ?= so they can be overridden
#                   Use arduinoISP settings as default for ISP targets
#

#----------------------------------------------------------------------
#
# program name should not be changed...
PROGRAM    = optiboot

# The default behavior is to build using tools that are in the users
# current path variables, but we can also build using an installed
# Arduino user IDE setup, or the Arduino source tree.
# Uncomment this next lines to build within the arduino environment,
# using the arduino-included avrgcc toolset (mac and pc)


# export symbols to recursive makes (for ISP)
export

all: atmega328_8Mhz

# defaults
MCU_TARGET = atmega328p
LDSECTIONS  = -Wl,--section-start=.text=0x3e00 -Wl,--section-start=.version=0x3ffe

# Build environments
# Start of some ugly makefile-isms to allow optiboot to be built
# in several different environments.  See the README.TXT file for
# details.

# default
fixpath = $(1)

# GCCROOT = /home/jeffrey/AVR/avr8-gnu-toolchain-linux_x86_64/bin/
GCCROOT = ~/.platformio/packages/toolchain-atmelavr/bin/
AVRDUDE_CONF =

STK500 = "C:\Program Files\Atmel\AVR Tools\STK500\Stk500.exe"
STK500-1 = $(STK500) -e -d$(MCU_TARGET) -pf -vf -if$(PROGRAM)_$(TARGET).hex \
           -lFF -LFF -f$(HFUSE)$(LFUSE) -EF8 -ms -q -cUSB -I200kHz -s -wt
STK500-2 = $(STK500) -d$(MCU_TARGET) -ms -q -lCF -LCF -cUSB -I200kHz -s -wt
#
# End of build environment code.


OBJ        = $(PROGRAM).o
OPTIMIZE = -Os -fno-inline-small-functions -fno-split-wide-types

DEFS       =
LIBS       =

CC         = $(GCCROOT)avr-gcc

# Override is only needed by avr-lib build system.  -ffunction-sections -fdata-sections

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) -DBIGBOOT -DF_CPU=$(AVR_FREQ) $(DEFS)
override LDFLAGS       = $(LDSECTIONS) -Wl,--relax -nostartfiles -nostdlib
#-Wl,--gc-sections

OBJCOPY        = $(GCCROOT)avr-objcopy
OBJDUMP        = $(call fixpath,$(GCCROOT)avr-objdump)

SIZE           = $(GCCROOT)avr-size

#
# Make command-line Options.
# Permit commands like "make atmega328 LED_START_FLASHES=10" to pass the
# appropriate parameters ("-DLED_START_FLASHES=10") to gcc
#

ifdef BAUD_RATE
BAUD_RATE_CMD = -DBAUD_RATE=$(BAUD_RATE)
dummy = FORCE
else
BAUD_RATE_CMD = -DBAUD_RATE=57600
endif

ifdef LED_START_FLASHES
LED_START_FLASHES_CMD = -DLED_START_FLASHES=$(LED_START_FLASHES)
dummy = FORCE
else
LED_START_FLASHES_CMD = -DLED_START_FLASHES=3
endif

# BIG_BOOT: Include extra features, up to 1K.
ifdef BIGBOOT
BIGBOOT_CMD = -DBIGBOOT=1
dummy = FORCE
endif

ifdef SOFT_UART
SOFT_UART_CMD = -DSOFT_UART=1
dummy = FORCE
endif

ifdef LED_DATA_FLASH
LED_DATA_FLASH_CMD = -DLED_DATA_FLASH=1
dummy = FORCE
endif

ifdef LED
LED_CMD = -DLED=$(LED)
dummy = FORCE
endif

ifdef SINGLESPEED
SSCMD = -DSINGLESPEED=1
endif

COMMON_OPTIONS = $(BAUD_RATE_CMD) $(LED_START_FLASHES_CMD) $(BIGBOOT_CMD)
COMMON_OPTIONS += $(SOFT_UART_CMD) $(LED_DATA_FLASH_CMD) $(LED_CMD) $(SSCMD)

#UART is handled separately and only passed for devices with more than one.
ifdef UART
UARTCMD = -DUART=$(UART)
endif

# Not supported yet
# ifdef SUPPORT_EEPROM
# SUPPORT_EEPROM_CMD = -DSUPPORT_EEPROM
# dummy = FORCE
# endif

# Not supported yet
# ifdef TIMEOUT_MS
# TIMEOUT_MS_CMD = -DTIMEOUT_MS=$(TIMEOUT_MS)
# dummy = FORCE
# endif
#

#---------------------------------------------------------------------------
# "Chip-level Platform" targets.
# A "Chip-level Platform" compiles for a particular chip, but probably does
# not have "standard" values for things like clock speed, LED pin, etc.
# Makes for chip-level platforms should usually explicitly define their
# options like: "make atmega1285 AVR_FREQ=16000000L LED=D0"
#---------------------------------------------------------------------------
#
# Note about fuses:
# the efuse should really be 0xf8; since, however, only the lower
# three bits of that byte are used on the atmega168, avrdude gets
# confused if you specify 1's for the higher bits, see:
# http://tinker.it/now/2007/02/24/the-tale-of-avrdude-atmega168-and-extended-bits-fuses/
#
# similarly, the lock bits should be 0xff instead of 0x3f (to
# unlock the bootloader section) and 0xcf instead of 0x2f (to
# lock it), but since the high two bits of the lock byte are
# unused, avrdude would get confused.
#---------------------------------------------------------------------------
# Test platforms
# Virtual boot block test
virboot328: TARGET = atmega328p
virboot328: MCU_TARGET = atmega328p
virboot328: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT'
virboot328: AVR_FREQ ?= 8000000
virboot328: LDSECTIONS  = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
virboot328: $(PROGRAM)_atmega328.hex
virboot328: $(PROGRAM)_atmega328.lst

# Virtual boot block test (8 MHz Internal Clock)
virboot328_8Mhz: TARGET = atmega328_8Mhz
virboot328_8Mhz: MCU_TARGET = atmega328p_8Mhz
virboot328_8Mhz: CFLAGS += $(COMMON_OPTIONS) '-DVIRTUAL_BOOT'
virboot328_8Mhz: AVR_FREQ ?= 8000000L
virboot328_8Mhz: LDSECTIONS  = -Wl,--section-start=.text=0x7e00 -Wl,--section-start=.version=0x7ffe
virboot328_8Mhz: $(PROGRAM)_atmega328_8Mhz.hex
virboot328_8Mhz: $(PROGRAM)_atmega328_8Mhz.lst

######## Make directives (16 MHz) ########
# atmega328: TARGET = moteino_16mhz
# atmega328: MCU_TARGET = atmega328p
# atmega328: CFLAGS += $(COMMON_OPTIONS) -DMOTEINO -DBAUD_RATE=500000L
# atmega328: AVR_FREQ ?= 16000000L
# atmega328: LDSECTIONS  = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
# atmega328: $(PROGRAM)_mot_16mhz.hex
# atmega328: $(PROGRAM)_mot_16mhz.lst

# atmega328_isp: moteino_16mhz
# atmega328_isp: TARGET = atmega328
# atmega328_isp: MCU_TARGET = atmega328p
# # 512 byte boot, SPIEN
# atmega328_isp: HFUSE ?= DC
# ###
# # Low power xtal (16MHz) 16KCK/14CK+65ms
# atmega328_isp: LFUSE ?= DE
# ###
# # 2.7V brownout
# atmega328_isp: EFUSE ?= FD
# ###
# atmega328_isp: isp
##########################################

######## Make directives (8 MHz) #########
atmega328_8Mhz: TARGET = atmega328_8Mhz
atmega328_8Mhz: MCU_TARGET = atmega328p

# 57600 / 500000
atmega328_8Mhz: CFLAGS += $(COMMON_OPTIONS) -DBAUD_RATE=57600
atmega328_8Mhz: AVR_FREQ ?= 8000000L
atmega328_8Mhz: LDSECTIONS  = -Wl,--section-start=.text=0x7c00 -Wl,--section-start=.version=0x7ffe
atmega328_8Mhz: $(PROGRAM)_atmega328_8Mhz.hex
atmega328_8Mhz: $(PROGRAM)_atmega328_8Mhz.lst

atmega328_isp_8Mhz: atmega328_8Mhz
atmega328_isp_8Mhz: TARGET = atmega328_8Mhz
atmega328_isp_8Mhz: MCU_TARGET = atmega328p

# Internal RC OScillator (8 MHz) 16KCK/14CK+0ms
atmega328_isp_8Mhz: LFUSE ?= C2

# 512 word (1024 byte) boot, SPIEN
# atmega328_isp_8Mhz: HFUSE ?= D3
atmega328_isp_8Mhz: HFUSE ?= DC

# brownout disabled
# atmega328_isp_8Mhz: EFUSE ?= FF
atmega328_isp_8Mhz: EFUSE ?= 7F
# atmega328_isp_8Mhz: EFUSE ?= FE

# Internal RC OScillator (8 MHz) 16KCK/14CK+65ms (Factory Default)
# atmega328_isp_8Mhz: LFUSE ?= E2

# 1.8V brownout
# atmega328_isp_8Mhz: EFUSE ?= FE

atmega328_isp_8Mhz: isp
##########################################



#---------------------------------------------------------------------------
#
# Generic build instructions
#

FORCE:

isp: $(TARGET)
	$(MAKE) -f Makefile.isp isp TARGET=$(TARGET)

isp-stk500: $(PROGRAM)_$(TARGET).hex
	$(STK500-1)
	$(STK500-2)

%.elf: $(OBJ) $(dummy)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LIBS)
	$(SIZE) $@

clean:
	rm -rf *.o *.elf *.lst *.map *.sym *.lss *.eep *.srec *.bin *.hex *.tmp.sh
	rm -rf ../optiboot_atmega328.hex

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex $< $@
	echo "\n\n>>>DONE<<<<\nCopying to ../$@ \n\n"
	cp -f $@ ../$@
	rm $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O binary $< $@
