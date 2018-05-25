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
#include "stdio.h"

/*485接口底层接收缓存*/
static uint8_t uart4_rx_buf[MAX485_UART4_RX_SIZE];
uint16_t uart4_rx_index = 0;
uint16_t uart4_rx_lines = 0;

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
        timeout = get_sys_time();
         USART_SendData(UART4_485, (uint8_t) *Data++);
        while (USART_GetFlagStatus(UART4_485, USART_FLAG_TXE) == RESET) {
            if (get_sys_time() - timeout > 1)
                break; //需要break还是return
        }
    }
    /*发送完成之后设置为接收模式*/
    MAX485_DIR_INPUT;
}

void UART4_IRQHandler(void)
{
    uint8_t data;
    if (USART_GetITStatus(UART4, USART_IT_RXNE) != RESET) {
        /* Read one byte from the receive data register */
        data = USART_ReceiveData(UART4);
        if (data == 0x0d) //回车符
            uart4_rx_lines++;
        uart4_rx_buf[uart4_rx_index] = data;
        if (++uart4_rx_index == UART4_RX_SIZE) {
            uart4_rx_index = 0;
            uart4_rx_lines = 0;
            memset(uart4_rx_buf, 0, UART4_RX_SIZE);
            ibox_printf(ibox_wifi_debug, ("uart4 USART_FLAG_ORE is set\r\n"));
        }
    }
    if (USART_GetFlagStatus(UART4, USART_FLAG_ORE) == SET) {
        USART_ClearFlag(UART4, USART_FLAG_ORE);
        USART_ReceiveData(UART4);
        ibox_printf(ibox_wifi_debug, ("uart4 USART_FLAG_ORE is set\r\n"));
    }
}
