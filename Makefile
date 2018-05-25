#/*
#**************************************************************************************************
#*文件：Makefile
#*作者：fanwenl_
#*版本：V0.0.1
#*日期：2018-05-23
#*描述：Makefile for iBox,add auto build with gcc.
#*	    参考LiteOS编写.	
#**************************************************************************************************
#*/

PROJECT           := iBoxClient

TARGET            := $(PROJECT)
TARGET_ELF        := $(TARGET).elf
TARGET_BIN        := $(TARGET).bin
TARGET_HEX        := $(TARGET).hex

OBJCPFLAGS_ELF_TO_BIN   = -Obinary
OBJCPFLAGS_ELF_TO_HEX   = -Oihex
OBJCPFLAGS_BIN_TO_HEX   = -Ibinary -Oihex
OBJCPFLAGS_ELF_TO_SREC  = -Osrec
OBJCPFLAGS_ELF_TO_LIST  = -S

#ifeq (${ARCH}, arm)
CROSS_COMPILE := arm-none-eabi-
#endif

ifeq ($(USE_CCACHE),1)
CCACHE := ccache
endif
 
# Make variables (CC, etc...)
AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
ifeq ($(USE_CCACHE),1)
CC		= $(CCACHE) $(CROSS_COMPILE)gcc
CXX		= $(CCACHE) $(CROSS_COMPILE)g++
else
CC		:= $(CROSS_COMPILE)gcc
CXX		:= $(CROSS_COMPILE)g++
endif
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
GDB		= $(CROSS_COMPILE)gdb
READELF		= $(CROSS_COMPILE)readelf

ifndef SOURCE_ROOT
    # Assume we're in the source directory if not specified.
    SOURCE_ROOT=.
    export SOURCE_ROOT
endif

APPDIR ?= ${SOURCE_ROOT}/Application/src

CFLAGS  = -g -O0 -Wall
CFLAGS += -mlittle-endian  -mcpu=cortex-m3  -march=armv7-m -ffreestanding -mthumb -mthumb-interwork -std=gnu99 --specs=nosys.specs
# --specs=rdimon.specs https://stackoverflow.com/questions/19419782/exit-c-text0x18-undefined-reference-to-exit-when-using-arm-none-eabi-gcc
# 增加定义
DEFINES += HSE_VALUE=12000000
DEFINES += STM32F10X_HD
DEFINES += USE_STDPERIPH_DRIVER 
# If a parent Makefile has passed us DEFINES, assume they will be missing -D.
DEFINES := ${DEFINES:%=-D%}

ifeq (${USE_RTOS}, 1)
DEFINES += -DUSE_RTOS
endif
ifeq (${USE_WIFI}, 1)
DEFINES += -DUSE_WIFI
endif

# 头文件
vpath %.c    ${SOURCE_ROOT}
vpath %.h    ${SOURCE_ROOT}
vpath %.S    ${SOURCE_ROOT}
vpath %.s    ${SOURCE_ROOT}


INCLUDE_DIRS = ${SOURCE_ROOT}/Application/inc                 \
	       	   ${SOURCE_ROOT}/Drivers/BSP/inc                 \
               ${SOURCE_ROOT}/Drivers/CMSIS/CM3/CoreSupport		\
			   ${SOURCE_ROOT}/Drivers/CMSIS/CM3/DeviceSupport/ST/STM32F10x  \
			   ${SOURCE_ROOT}/Drivers/STM32F10x_StdPeriph_Driver/inc  \
			   ${SOURCE_ROOT}/Middlewares/Eth


INCLUDES += ${INCLUDE_DIRS:%=-I%}

CFLAGS += ${DEFINES}
CFLAGS += ${INCLUDES}

LINKER_SCRIPT = ${SOURCE_ROOT}/Drivers/LinkerScript_gcc/STM32F103ZE_FLASH.ld
LDFLAGS    +=  -T $(LINKER_SCRIPT) 

############################################################
### Sub-makefiles
############################################################

include ${SOURCE_ROOT}/Application/Makefile
include ${SOURCE_ROOT}/Drivers/BSP/Makefile
include ${SOURCE_ROOT}/Drivers/CMSIS/Makefile
include ${SOURCE_ROOT}/Drivers/STM32F10x_StdPeriph_Driver/Makefile
include ${SOURCE_ROOT}/Middlewares/Eth/Makefile

OBJECTS += ${patsubst %.c,build/%.o,$(C_SOURCES)}
OBJECTS += ${patsubst %.s,build/%.o,$(ASM_SOURCES)}


.PHONY: all default clean 

default: all

all:build build/lib build/$(TARGET_BIN)

build:
	@echo -----------------------------------------------------------------------------------
	@echo Building target: $(PROJECT)
	@echo Invoking: $(CC) Linker
	mkdir -p $@

build/lib:
#	$(MAKE) -C Drivers/STM32F10x_StdPeriph_Driver
	
build/$(TARGET_BIN):build/$(TARGET_ELF)
	$(OBJCOPY) $(OBJCPFLAGS_ELF_TO_BIN) -S $^ $@
	$(OBJCOPY) $(OBJCPFLAGS_ELF_TO_HEX) -S $^ $@
	@#$(OBJDUMP) -sStD $(TARGET_ELF) > map
	@echo 'Finished building target: $@'
	@echo -----------------------------------------------------------------------------------

# link 的时候需要加参数--specs=rdimon.specs
build/$(TARGET_ELF):$(OBJECTS) 
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

build/%.o:%.c 
	#$(CC) $(CFLAGS) -c  -o  $(addprefix $(dir $^), $(notdir $@))    $^
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $^

build/%.o:%.s
	mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c -o $@ $^

clean:
	@echo -----------------------------------------------------------------------------------
	@echo "Removing linked and compiled files......"
#	$(MAKE) -C Drivers/STM32F10x_StdPeriph_Driver clean
	-rm -rf build
	find $(SOURCE_ROOT) -iname '*.o' -delete
	find $(SOURCE_ROOT) -iname '*.bin' -delete
	find $(SOURCE_ROOT) -iname '*.elf' -delete
	find $(SOURCE_ROOT) -iname '*.map' -delete
	@echo -----------------------------------------------------------------------------------




# ROOT=$(shell pwd)

# OBJS = $(SRCS:.c=.o)

# .PHONY: lib proj

# all: lib proj

# lib:
# 	$(MAKE) -C HAL
# proj: 	$(PROJ_NAME).elf
# $(PROJ_NAME).elf: $(SRCS)
# 	$(CC) $(CFLAGS)  $^ -o $@ -LHAL -lstm32f7
# 	$(OBJCOPY) -O ihex -S $(PROJ_NAME).elf $(PROJ_NAME).hex
# 	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

# clean:
# 	$(MAKE) -C HAL clean
# 	rm -f $(PROJ_NAME).elf
# 	rm -f $(PROJ_NAME).hex
# 	rm -f $(PROJ_NAME).bin 


