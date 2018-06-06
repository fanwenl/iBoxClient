/*
**************************************************************************************************
*文件：ibox_485.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-26
*描述：iBox 485接口头文件。
* ************************************************************************************************
*/
#ifndef __IBOX_485_H
#define __IBOX_485_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"

#define MAX485_DIR_PORT         GPIOD
#define MAX485_DIR_PIN          GPIO_Pin_0
#define MAX485_DIR_PORT_RCC     RCC_APB2Periph_GPIOD

#define MAX485_DIR_OUTPUT       GPIO_SetBits(MAX485_DIR_PORT, MAX485_DIR_PIN);
#define MAX485_DIR_INPUT        GPIO_ResetBits(MAX485_DIR_PORT, MAX485_DIR_PIN);

/*485 UART4串口接收变量定义*/
#define UART4_RX_SIZE    1024 
#define UART4_TX_SIZE    1024 


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_485_H*/
