/*
**************************************************************************************************
*文件：iBox_wifi.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-21
*描述：iBox iot wifi驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_WIFI_H
#define __IBOX_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stm32f10x.h"
typedef enum {
    ESP8266_STATUS_CHECK = 0,        //检测模块
    ESP8266_STATUS_CHECK_MODE,
    ESP8266_STATUS_WAIT_CHECK_MODE,
    ESP8266_STATUS_SET_MODE,
    ESP8266_STATUS_WAIT_SET_MODE,
    ESP8266_STATUS_RESET,
    ESP8266_STATUS_WAIT_RESET,
    ESP8266_STATUS_WAIT_CLOSE_ECHO,
    ESP8266_STATUS_WIFI_CONNECT,
    ESP8266_STATUS_WAIT_CONNECTED,
    ESP8266_STATUS_WAIT_GOT_IP,
    ESP8266_STATUS_LINK_OK,
    ESP8266_STATUS_GET_IP,
    ESP8266_STATUS_GET_MAC,
    ESP8266_STATUS_RESTART_CONNECT_SEVER,
    ESP8266_STATUS_CONNECT_SERVER,
    ESP8266_STATUS_WAIT_LINK_SERVER,
    ESP8266_STATUS_COMMUNICATE, //正常的通信状态communication
    /*如果采用透传模式，不好判断是否断网，所有先不采用透传模式*/
//    ESP8266_STATUS_SET_CIPMODE,  //设置透传模式
//    ESP8266_STATUS_SETART_CIP,   //开始透传
//    ESP8266_STATUS_END_CIP,
} ESP8266_STATUS_ENUM;

/*wifi reset控制*/
#define WIFI_RESET_PIN GPIO_Pin_0
#define WIFI_RESET_PORT GPIOG
#define WIFI_RESET_RCC_CLOCK RCC_APB2Periph_GPIOG

/*wifi CH_PD引脚控制*/
#define WIFI_CH_PD_PIN GPIO_Pin_1
#define WIFI_CH_PD_PORT GPIOG
#define WIFI_CH_PD_RCC_CLOCK RCC_APB2Periph_GPIOG

/*低电平复位*/
#define WIFI_RESET_L GPIO_ResetBits(WIFI_RESET_PORT, WIFI_RESET_PIN)
#define WIFI_RESET_H GPIO_SetBits(WIFI_RESET_PORT, WIFI_RESET_PIN)

/*高电平选通*/
#define WIFI_CH_PD_L GPIO_ResetBits(WIFI_CH_PD_PORT, WIFI_CH_PD_PIN)
#define WIFI_CH_PD_H GPIO_SetBits(WIFI_CH_PD_PORT, WIFI_CH_PD_PIN)

void wifi_init(void);
void esp8266_at_fsm(void);

void esp8266_send_data(void);
ESP8266_STATUS_ENUM get_esp8266_status(void);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_WIFI_H*/
