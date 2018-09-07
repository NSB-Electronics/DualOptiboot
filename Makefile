################################################################################
# Automatically-generated file. Do not edit!
################################################################################
include .local/config.mk

USER_OBJS :=

LIBS := 
PROJ := 

CMSIS_DIR := $(SDK_PATH)/devicePack/arm/cmsis/5.0.1/CMSIS/Include
ATMEL_DFP := $(SDK_PATH)/devicePack/atmel/SAMD20_DFP/1.2.91/samd20/include
TOOLCHAIN_PATH := $(SDK_PATH)/toolchain/bin

PROJ_DIR = $(PWD)
BUILD_DIR := build

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
QUOTE := 
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS :=  \
$(PROJ_DIR)/

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
$(PROJ_DIR)/extFlash.c \
$(PROJ_DIR)/hardware.c \
$(PROJ_DIR)/NVM.c \
$(PROJ_DIR)/optiboot.c \
$(PROJ_DIR)/serial.c \
#../Device_Startup/startup_samd20.c \
#../Device_Startup/system_samd20.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
$(BUILD_DIR)/extFlash.o \
$(BUILD_DIR)/hardware.o \
$(BUILD_DIR)/NVM.o \
$(BUILD_DIR)/optiboot.o \
$(BUILD_DIR)/serial.o \
#Device_Startup/startup_samd20.o \
#Device_Startup/system_samd20.o

OBJS_AS_ARGS +=  \
$(BUILD_DIR)/extFlash.o \
$(BUILD_DIR)/hardware.o \
$(BUILD_DIR)/NVM.o \
$(BUILD_DIR)/optiboot.o \
$(BUILD_DIR)/serial.o \
#Device_Startup/startup_samd20.o \
#Device_Startup/system_samd20.o

C_DEPS +=  \
$(BUILD_DIR)/extFlash.d \
$(BUILD_DIR)/hardware.d \
$(BUILD_DIR)/NVM.d \
$(BUILD_DIR)/optiboot.d \
$(BUILD_DIR)/serial.d \
#Device_Startup/startup_samd20.d \
#Device_Startup/system_samd20.d

C_DEPS_AS_ARGS +=  \
$(BUILD_DIR)/extFlash.d \
$(BUILD_DIR)/hardware.d \
$(BUILD_DIR)/NVM.d \
$(BUILD_DIR)/optiboot.d \
$(BUILD_DIR)/serial.d \
#Device_Startup/startup_samd20.d \
#Device_Startup/system_samd20.d

OUTPUT_FILE_PATH +=$(BUILD_DIR)/DualOptiboot.elf

OUTPUT_FILE_PATH_AS_ARGS +=$(BUILD_DIR)/DualOptiboot.elf

ADDITIONAL_DEPENDENCIES:=

LIB_DEP+= 

LINKER_SCRIPT_DEP+=

# AVR32/GNU C Compiler
$(BUILD_DIR)/%.o: $(PROJ_DIR)/%.c
	@echo Building file: $<
	@echo Invoking: ARM/GNU C Compiler : 6.3.1
	$(TOOLCHAIN_PATH)/arm-none-eabi-gcc -x c -mthumb -D__SAMD20E18__ -DDEBUG  -I$(CMSIS_DIR) -I$(ATMEL_DFP) -I$(PROJ_DIR)  -Og -ffunction-sections -mlong-calls -g3 -Wall -mcpu=cortex-m0plus -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<


# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 
directories: 
	mkdir build

# All Target
all: directories $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP)
	@echo Building target: $@
	@echo Invoking: ARM/GNU Archiver : 6.3.1
	$(TOOLCHAIN_PATH)/arm-none-eabi-ar -r -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) 
	@echo Finished building target: $@
	
	





# Other Targets
clean:
	rm -rf build
	