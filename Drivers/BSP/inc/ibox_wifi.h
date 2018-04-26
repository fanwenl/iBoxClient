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
    ESP8266_STATUS_INIT = 0,     //状态机及esp8266初始化
    ESP8266_STATUS_CHECK,        //检测模块
    ESP8266_STATUS_SET_MODE,     //设置模式
    ESP8266_STATUS_RESET,        //重启模块
    ESP8266_STATUS_LINK,         //连接wifi
    ESP8266_STATUS_GET_IP,       //查询IP
    ESP8266_STATUS_LINK_SERVER,  //连接服务器
    ESP8266_STATUS_SEND_AUTHEN,  //发送认证信息
    ESP8266_STATUS_CHECK_AUTHEN, //收到认证信息
    ESP8266_STATUS_SET_CIPMODE,  //设置透传模式
    ESP8266_STATUS_SETART_CIP,   //开始透传
    ESP8266_STATUS_END_CIP,
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

/*485 UART4串口接收变量定义*/
#define UART3_RX_SIZE 1024

void wifi_init(void);
void uart3_send_str(uint8_t *data);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_WIFI_H*/
