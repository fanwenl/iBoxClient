/*
**************************************************************************************************
*文件：ibox_lora.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-07-10
*描述：ibox lora驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_LORA_H
#define __IBOX_LORA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"
#include <stdint.h>

#define LORA_RST_PIN    GPIO_Pin_13
#define LORA_RST_PORT   GPIOF
#define LORA_RST_CLK    RCC_APB2Periph_GPIOF

#define LORA_DIO0_PIN    GPIO_Pin_7
#define LORA_DIO1_PIN    GPIO_Pin_8
#define LORA_DIO2_PIN    GPIO_Pin_9
#define LORA_DIO3_PIN    GPIO_Pin_10
#define LORA_DIO5_PIN    GPIO_Pin_12
#define LORA_DIO_PORT    GPIOG
#define LORA_DIO_CLK     RCC_APB2Periph_GPIOG

#define LORA_RST_SET_H    GPIO_SetBits(LORA_RST_PORT, GPIO_Pin_13)
#define LORA_RST_SET_L    GPIO_ResetBits(LORA_RST_PORT, GPIO_Pin_13)

void lora_init(void);
uint8_t lora_dio0_read(void);
uint8_t lora_dio1_read(void);
uint8_t lora_dio2_read(void);
uint8_t lora_dio3_read(void);
uint8_t lora_dio5_read(void);

#ifdef __cplusplus
}
#endif

#endif /*__IBOX_LORA_H*/
