/*
**************************************************************************************************
*文件：ibox_485.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-26
*描述：iBox 485接口头文件。
* ************************************************************************************************
*/
#include "ibox_485.h"
#include "ibox_uart.h"
#include <stdio.h>
#include <string.h>


/*485接口底层接收缓存*/
static uint8_t uart4_rx_buf[UART4_RX_SIZE];
static uint8_t uart4_tx_buf[UART4_TX_SIZE];

uint16_t uart4_rx_wr_index = 0; // uart4接收buf位置标记
uint16_t uart4_rx_lines    = 0; // uart4接收到的行数计数器
uint16_t uart4_rx_count    = 0; // uart4接收到的字符串计数器
uint16_t uart4_rx_re_index = 0; // uart4读取位置标记

static uint8_t get_char_form_uart4(void);
static void uart4_rx_buf_clear(void);

static void max485_dir_pin_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    RCC_APB2PeriphClockCmd(MAX485_DIR_PORT_RCC, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin   = MAX485_DIR_PIN;

    GPIO_Init(MAX485_DIR_PORT, &GPIO_InitStruct);

    /*默认设置方向为接收*/
    GPIO_ResetBits(MAX485_DIR_PORT, MAX485_DIR_PIN);
}

void max485_init(void)
{
    max485_dir_pin_config();
    uart_init(UART4_485, 115200);
}

void max485_send_data(uint8_t *data)
{
    uint32_t timeout = 0;

    MAX485_DIR_OUTPUT;
    while (*data) {
        timeout = get_sys_time_s();
         USART_SendData(UART_485, (uint8_t) *data++);
        while (USART_GetFlagStatus(UART_485, USART_FLAG_TXE) == RESET) {
            if (get_sys_time_s() - timeout > 1)
                break; //需要break还是return
        }
    }
    /*发送完成之后设置为接收模式*/
    MAX485_DIR_INPUT;
}

void UART4_IRQHandler(void)
{
    uint8_t data;
    if (USART_GetITStatus(UART_485, USART_IT_RXNE) != RESET) {
        /* Read one byte from the receive data register */
        data = USART_ReceiveData(UART_485);
        if (data == 0x0d) //回车符
            uart4_rx_lines++;
        uart4_rx_buf[uart4_rx_wr_index] = data;
        if (++uart4_rx_wr_index == UART4_RX_SIZE)
            uart4_rx_wr_index = 0;
        /*buf[]满了，溢出了。*/
        if (++uart4_rx_count == UART4_RX_SIZE) {
            uart4_rx_wr_index = 0;
            uart4_rx_lines    = 0;
            uart4_rx_count    = 0;
            uart4_rx_re_index = 0;
            memset(uart4_rx_buf, 0, UART4_RX_SIZE);
            ibox_printf(ibox_wifi_debug, ("uart4 USART_FLAG_ORE is set\r\n"));
        }
    }
    if (USART_GetFlagStatus(UART_485, USART_FLAG_ORE) == SET) {
        USART_ClearFlag(UART_485, USART_FLAG_ORE);
        USART_ReceiveData(UART_485);
        ibox_printf(ibox_wifi_debug, ("uart4 USART_FLAG_ORE is set\r\n"));
    }
}
/**
 * 从uart4中获取一行字符串(485通信接口)
 * *buf需要存放数据的缓存
 */
uint16_t get_line_from_uart4(uint8_t *buf)
{
    uint8_t data      = 0;
    uint16_t data_len = 0;

    if (!uart4_rx_lines) //无数据退出
    {
        buf = NULL;
        return 0;
    }
    while (1) {
        if (uart4_rx_count) {
            data = get_char_form_uart4();
        } else {
            uart4_rx_lines = 0;
            buf            = NULL;
            return 0;
        }
        if (data == 0x0a) //换行直接丢掉
            continue;
        if (data == 0x0d) { //回车，收到一帧
            uart4_rx_lines--;
            *buf = '\0';
            return data_len;
        }
        *buf = data;
        buf++;
        data_len++;
    }
}
/**
 * uart4中读取一个字符
 * 返回读取到的字符的值
 */
static uint8_t get_char_form_uart4(void)
{
    uint8_t data;
    while (uart4_rx_count == 0) {
        ibox_printf(ibox_wifi_debug, ("uart4 rx count is 0\r\n"));
    }
    data = uart4_rx_buf[uart4_rx_re_index];
    if (++uart4_rx_re_index == UART4_RX_SIZE)
        uart4_rx_re_index = 0;
    USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
    --uart4_rx_count;
    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    return data;
}
/**
 * 清除uart4的buf的缓存
 */
static void uart4_rx_buf_clear(void)
{
    uart4_rx_wr_index = 0;
    uart4_rx_lines    = 0;
    uart4_rx_count    = 0;
    uart4_rx_re_index = 0;
    memset(uart4_rx_buf, 0, UART4_RX_SIZE);
}