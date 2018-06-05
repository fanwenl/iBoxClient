include ./Drivers/BSP/SX1276/Makefile

DIRECTORIES += Drivers/BSP/src

C_SOURCES += Drivers/BSP/src/ibox_adc.c  \
			 Drivers/BSP/src/ibox_dac.c   \
			 Drivers/BSP/src/ibox_Ethernet.c \
			 Drivers/BSP/src/ibox_gprs.c  \
			 Drivers/BSP/src/ibox_io.c   \
			 Drivers/BSP/src/ibox_key.c  \
			 Drivers/BSP/src/ibox_led.c  \
			 Drivers/BSP/src/ibox_lora.c  \
			 Drivers/BSP/src/ibox_rtc.c   \
			 Drivers/BSP/src/ibox_spi.c   \
			 Drivers/BSP/src/ibox_sys.c   \
			 Drivers/BSP/src/ibox_uart.c  \
			 Drivers/BSP/src/ibox_wdog.c  \
			 Drivers/BSP/src/ibox_config.c   \
			 Drivers/BSP/src/w5500.c

ifdef USE_WIFI
C_SOURCES += Drivers/BSP/src/ibox_wifi.c
endif