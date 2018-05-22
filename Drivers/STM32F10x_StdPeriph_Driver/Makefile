CC=arm-none-eabi-gcc
AR=arm-none-eabi-ar

###########################################
vpath %.c src 

CFLAGS += -mthumb -mcpu=cortex-m3

CFLAGS  = -g -O0 -Wall
CFLAGS += -mthumb -mcpu=cortex-m3
CFLAGS += -mlittle-endian  -mthumb-interwork

CFLAGS += -ffreestanding -nostdlib

CFLAGS += -Iinc -I../CMSIS/CM3/CoreSupport -I../CMSIS/CM3/DeviceSupport/ST/STM32F10x 
CFLAGS += -IInc/Legacy -I../../Application/inc
CFLAGS += -DSTM32F10X_HD -DUSE_STDPERIPH_DRIVER

#CFLAGS += --specs=nosys.specs
#LFLAGS += --specs=nosys.specs

SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

.PHONY: libstm32f1.a
all: libstm32f1.a
%.o : %.c
	$(CC) $^ $(CFLAGS) -c -o $@ 
libstm32f1.a: $(OBJS)
	$(AR) -r $@ $(OBJS)
clean:
	rm -f $(OBJS) libstm32f1.a