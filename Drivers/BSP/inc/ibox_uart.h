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
#include "ibox_sys.h"

typedef enum {
    UART1_DEBUG      = 0,         /*uart1*/
    UART3_GPRS_WIFI  = 1,         /*uart3*/
    UART4_485        = 2,         /*uart4*/
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

#define UART_GPRS_WIFI_PIN1       GPIO_Pin_11       /*RX*/
#define UART_GPRS_WIFI_PIN2       GPIO_Pin_10       /*Tx*/
#define UART_GPRS_WIFI_PORT       GPIOB

#define UART_485_UART_RCC_CLK          RCC_APB1Periph_UART4
#define UART_DEBUG_UART_RCC_CLK        RCC_APB2Periph_USART1
#define UART_GPRS_WIFI_UART_RCC_CLK    RCC_APB1Periph_USART3

#define UART_485_GPIO_RCC_CLK          RCC_APB2Periph_GPIOC
#define UART_DEBUG_GPIO_RCC_CLK        RCC_APB2Periph_GPIOA
#define UART_GPRS_WIFI_GPIO_RCC_CLK    RCC_APB2Periph_GPIOB

#define UART1_RX_SIZE               256

/*GPRS/WIFI UART3串口接收变量定义,1460 GPRS介绍发送最大长度ASCII*/
#define UART3_RX_SIZE 1460
#define UART3_TX_SIZE 1460

extern uint8_t uart3_rx_buf[];
extern uint8_t uart3_tx_buf[];

void uart_init(uart_enum uart_num, uint32_t baud_rate);
uint8_t get_char_form_uart1(char *ch);

void uart3_send_str(const char *data);
void uart3_send_data(uint8_t*data, uint16_t len);
uint8_t get_char_form_uart3(void);
uint16_t get_line_from_uart3(uint8_t *buf);
void uart3_rx_buf_clear(void);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_UART_H*/


