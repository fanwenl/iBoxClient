/*
**************************************************************************************************
*文件：iBox_wifi.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-21
*描述：iBox iot wifi驱动。
* ************************************************************************************************
*/

#include "ibox_wifi.h"

static void wifi_ctrl_pin_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(WIFI_RESET_RCC_CLOCK | WIFI_CH_PD_RCC_CLOCK, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin = WIFI_RESET_PIN;
    
    GPIO_Init(WIFI_RESET_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = WIFI_CH_PD_PIN;
    GPIO_Init(WIFI_CH_PD_PORT, &GPIO_InitStruct);
}

void wifi_init(void)
{
    wifi_ctrl_pin_config();

    /*复位wifi模块*/
    WIFI_RESET_L;
    WIFI_CH_PD_L;
    sys_delay_ms(1000);
    WIFI_RESET_H;
    WIFI_CH_PD_H;

    uart3_send_str("AT\r\n");
}
void uart3_send_str(uint8_t *data)
{
    while(*data)
    {
        USART_SendData(USART3, (uint8_t)*data++);
        while( USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET );

    }
}

void USART3_IRQHandler(void)
{
    uint8_t data;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    { 
        /* Read one byte from the receive data register */
        data = USART_ReceiveData(USART3);
        printf("%c",data);
//        if (data == 0x0d) recvlines++;  
//        rx_buffer2[rx_wr_index2]=data;
//        if (++rx_wr_index2 == RX_BUFFER_SIZE2) rx_wr_index2=0;
//        if (++rx_counter2 == RX_BUFFER_SIZE2)
//        {
//	     printf("usart2 overflow\r\n");
//            rx_counter2=0;
//            rx_buffer_overflow2=1;
//		clearevbuf2();
//        }
    }
    if(USART_GetFlagStatus(USART3,USART_FLAG_ORE) == SET)
    {
        USART_ClearFlag(USART3,USART_FLAG_ORE);
        USART_ReceiveData(USART3);
        ibox_printf(ibox_wifi_debug, ("usart3 USART_FLAG_ORE is set\r\n"));
    }
}
