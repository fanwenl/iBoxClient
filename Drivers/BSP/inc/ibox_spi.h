/*
**************************************************************************************************
*文件：iBox_spi.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-05-04
*描述：iBox iot spi的驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_SPI_H
#define __IBOX_SPI_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"

typedef enum {
    W5500 = 0,
    LOAR,
}SPI_typedef;


#define LOAR_CS_H GPIO_SetBits(GPIOA, GPIO_Pin_8)
#define LORA_CS_L GPIO_ResetBits(GPIOA, GPIO_Pin_8)

void spi_init(SPI_typedef module);

#ifdef __cplusplus
    }
#endif /*__cplusplus*/

#endif /*__IBOX_LED_H*/
