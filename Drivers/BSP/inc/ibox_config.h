/*
**************************************************************************************************
*文件：iBox_config.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18
*描述：iBox iot开发套件配置文件。
* ************************************************************************************************
*/
#ifndef __IBOX_CONFIG_H
#define __IBOX_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ibox_485.h"
#include "ibox_adc.h"
#include "ibox_dac.h"
#include "ibox_led.h"
#include "ibox_rtc.h"
#include "ibox_sys.h"
#include "ibox_uart.h"
#include "ibox_wdog.h"
#include "ibox_wifi.h"
#include "main.h"
#include "stdio.h"
#include "stm32f10x_conf.h"
#include "stm32f10x_it.h"

#define MAX_SERVER_IP_LEN       16
#define MAX_SERVER_PORT_LEN     5
#define MAX_SERVER_DSN_LEN      12
#define MAX_WIFI_SSID_LEN       10
#define MAX_WIFI_PASS_LEN       10
#define MAX_WIFI_MAC_LEN        17

#pragma pack(1)
typedef struct __IBOX_CONFIG {
    uint8_t server_ip[MAX_SERVER_IP_LEN];
    uint8_t server_port[MAX_SERVER_PORT_LEN];
    uint8_t server_dsn[MAX_SERVER_DSN_LEN];
#ifdef USE_WIFI
    uint8_t wifi_ssid[MAX_WIFI_SSID_LEN];
    uint8_t wifi_password[MAX_WIFI_PASS_LEN];
    uint8_t wifi_mac[MAX_WIFI_MAC_LEN];
#else
    uint64_t gprs_imei;
#endif

} IBOX_CONFIG;
#pragma pack()

extern IBOX_CONFIG ibox_config;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_CONFIG_H*/
