/*
**************************************************************************************************
*文件：iBox_gprs.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-21
*描述：iBox iot gprs驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_GPRS_H
#define __IBOX_GPRS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stm32f10x.h"

/*gprs reset控制*/
#define GPRS_RESET_PIN          GPIO_Pin_0
#define GPRS_RESET_PORT         GPIOG
#define GPRS_RESET_PIN_CLK      RCC_APB2Periph_GPIOG

/*低电平复位*/
#define GPRS_RESET_L        GPIO_ResetBits(GPRS_RESET_PORT, GPRS_RESET_PIN)
#define GPRS_RESET_H        GPIO_SetBits(GPRS_RESET_PORT, GPRS_RESET_PIN)

typedef enum{
    GPRS_STATUS_INIT = 0,
    
    
}GPRS_STATUS_ENUM;


void gprs_init(void);
void gprs_at_fsm(void);


#ifdef __cplusplus
}
#endif

#endif /*__IBOX_GPRS_H*/
