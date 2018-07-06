/*
**************************************************************************************************
*文件：iBox_board.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18
*描述：iBox iot开发套件配置文件。
* ************************************************************************************************
*/
#ifndef __IBOX_BOARD_H
#define __IBOX_BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#define FIRMWARE_VER_MAIN       0L                                                                                                  
#define FIRMWARE_VER_SUB        3L
#define FIRMWARE_VER_REV        0L
#define FIRMWARE_VER            (FIRMWARE_VER_MAIN*10000+FIRMWARE_VER_SUB*100+FIRMWARE_VER_REV)

#include "ibox_485.h"
#include "ibox_adc.h"
#include "ibox_dac.h"
#include "ibox_led.h"
#include "ibox_rtc.h"
#include "ibox_sys.h"
#include "ibox_uart.h"
#include "ibox_wdog.h"
#include "ibox_wifi.h"
#include "ibox_key.h"
#include "ibox_spi.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_it.h"
#include "ibox_Ethernet.h"

#include <stdio.h>
#include <stdint.h>
#include "wizchip_conf.h"
#include "dhcp.h"
#include "dns.h"

/*下面的这些长度需要多定义一位存放'/0',否则和后面数据连起来*/
#define MAX_IP_LEN              16
#define MAX_ETH_MAC_LEN         13
#define MAX_DSN_LEN             12
#define MAX_WIFI_SSID_LEN       10
#define MAX_WIFI_PASS_LEN       11
#define MAX_WIFI_MAC_LEN        18

#pragma pack(1)
typedef struct __IBOX_CONFIG {
    uint8_t server_ip[MAX_IP_LEN];
    uint8_t server_dsn[MAX_DSN_LEN];
    uint16_t server_port;
    uint16_t local_port;
    uint8_t eth_mac[MAX_ETH_MAC_LEN];
#ifdef USE_WIFI
    uint8_t wifi_ssid[MAX_WIFI_SSID_LEN];
    uint8_t wifi_password[MAX_WIFI_PASS_LEN];
    uint8_t wifi_mac[MAX_WIFI_MAC_LEN];
    uint8_t wifi_ip[MAX_IP_LEN];
#else
    uint64_t gprs_imei;
#endif

} IBOX_CONFIG;
#pragma pack()

/* board configuration */

/* whether use board external SRAM memory */
// <e>Use external SRAM memory on the board
// 	<i>Enable External SRAM memory
#define STM32_EXT_SRAM          0
//	<o>Begin Address of External SRAM
//		<i>Default: 0x68000000
#define STM32_EXT_SRAM_BEGIN    0x68000000 /* the begining address of external SRAM */
//	<o>End Address of External SRAM
//		<i>Default: 0x68080000
#define STM32_EXT_SRAM_END      0x68080000 /* the end address of external SRAM */
// </e>

// <o> Internal SRAM memory size[Kbytes] <8-64>
//	<i>Default: 64
#define STM32_SRAM_SIZE         64
#define STM32_SRAM_END          (0x20000000 + STM32_SRAM_SIZE * 1024)

// <<< Use Configuration Wizard in Context Menu >>>

/* USART driver select. */
// #define RT_USING_UART1
// #define RT_USING_UART2
// #define RT_USING_UART3
// #define USING_BXCAN1

#define IBOX_ASSERT(expr) ((expr) ? (void)0 : \
ibox_printf(1,("(%s) assertion failed at function:%s, line number:%d \n", expr, __FUNCTION__, __LINE__)))

void rt_hw_board_init(void);

extern IBOX_CONFIG ibox_config;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_BOARD_H*/
