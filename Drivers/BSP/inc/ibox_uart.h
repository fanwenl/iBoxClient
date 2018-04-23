/*
**************************************************************************************************
*文件：ibox_uart.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18
*描述：ibox uart驱动程序。
* ************************************************************************************************
*/
#ifndef __IBOX_UART_H
#define __IBOX_UART_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"

typedef enum {
    UART1_DEBUG      = 0,         /*uart1*/
    UART4_485        = 1,           /*uart4*/
    UART3_GPRS_WIFI  = 2,     /*uart3*/
}uart_enum;

#define UART_485                  UART4
#define UART_DEBUG                USART1
#define UART_GPRS_WIFI            USART3

#define UART_485_PIN1             GPIO_Pin_11
#define UART_485_PIN2             GPIO_Pin_10
#define UART_485_PORT             GPIOC

#define UART_DEBUG_PIN1           GPIO_Pin_10        /*Rx*/
#define UART_DEBUG_PIN2           GPIO_Pin_9
#define UART_DEBUG_PORT           GPIOA

#define UART_GPRS_WIFI_PIN1       GPIO_Pin_11
#define UART_GPRS_WIFI_PIN2       GPIO_Pin_10
#define UART_GPRS_WIFI_PORT       GPIOB

#define UART_485_RCC_CLK          RCC_APB1Periph_UART4
#define UART_DEBUG_RCC_CLK        RCC_APB2Periph_USART1
#define UART_GPRS_WIFI_RCC_CLK    RCC_APB1Periph_USART3


void uart_init(uart_enum uart_num, uint32_t baud_rate);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_UART_H*/


