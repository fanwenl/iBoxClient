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
      
#include "stm32f10x_conf.h"
#include "main.h"
#include "stdio.h"
#include "stm32f10x_it.h"
#include "ibox_led.h"
#include "ibox_sys.h"
#include "ibox_uart.h"
#include "ibox_adc.h"
#include "ibox_dac.h"
#include "ibox_rtc.h"
#include "ibox_wifi.h"
#include "ibox_wdog.h"
#include "ibox_485.h"

/*服务器信息配置*/
#define SERVER_IP   "192.168.0.0"
#define SERVER_PORT "10000"
#define SERVER_DNS  "fanwenl.top"

#ifdef USE_WIFI
#define WIFI_SSID   "ziroom2" 
#define WIFI_Password "4001001111"
#endif



#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_CONFIG_H*/
