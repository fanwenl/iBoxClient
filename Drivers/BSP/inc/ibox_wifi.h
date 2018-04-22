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

#include "stm32f10x.h"
#include "stdint.h"
#include "ibox_sys.h"

/*wifi reset控制*/
#define WIFI_RESET_PIN          GPIO_Pin_0
#define WIFI_RESET_PORT         GPIOG
#define WIFI_RESET_RCC_CLOCK    RCC_APB2Periph_GPIOG

/*wifi CH_PD引脚控制*/
#define WIFI_CH_PD_PIN          GPIO_Pin_1
#define WIFI_CH_PD_PORT         GPIOG
#define WIFI_CH_PD_RCC_CLOCK    RCC_APB2Periph_GPIOG

/*低电平复位*/
#define WIFI_RESET_L       GPIO_ResetBits(WIFI_RESET_PORT, WIFI_RESET_PIN)   
#define WIFI_RESET_H       GPIO_SetBits(WIFI_RESET_PORT, WIFI_RESET_PIN) 

/*高电平选通*/
#define WIFI_CH_PD_L       GPIO_ResetBits(WIFI_CH_PD_PORT, WIFI_CH_PD_PIN)   
#define WIFI_CH_PD_H      GPIO_SetBits(WIFI_CH_PD_PORT, WIFI_CH_PD_PIN) 

void wifi_init(void);
void uart3_send_str(uint8_t *data);

#ifdef __cplusplus
    }
#endif /*__cplusplus*/

#endif /*__IBOX_WIFI_H*/
