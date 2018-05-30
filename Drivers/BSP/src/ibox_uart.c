#include "ibox_uart.h"
#include "stdio.h"

uint32_t uart_pin[6]       = {UART_DEBUG_PIN1, UART_DEBUG_PIN2, UART_GPRS_WIFI_PIN1,
                        UART_GPRS_WIFI_PIN2, UART_485_PIN1,   UART_485_PIN2};
GPIO_TypeDef *uart_port[3] = {UART_DEBUG_PORT, UART_GPRS_WIFI_PORT, UART_485_PORT};
uint32_t uart_rcc[3]       = {UART_DEBUG_UART_RCC_CLK, UART_GPRS_WIFI_UART_RCC_CLK, UART_485_UART_RCC_CLK};
uint32_t uart_gpio_rcc[3]  = {UART_DEBUG_GPIO_RCC_CLK,UART_GPRS_WIFI_GPIO_RCC_CLK,UART_485_GPIO_RCC_CLK};
USART_TypeDef *uart[3]     = {USART1, USART3, UART4};
uint8_t uart_irqn[3] = {USART1_IRQn,USART3_IRQn, UART4_IRQn};


void uart_init(uart_enum uart_num, uint32_t baud_rate)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    if(uart_num == UART1_DEBUG)
        RCC_APB2PeriphClockCmd(uart_rcc[uart_num], ENABLE);
    else
        RCC_APB1PeriphClockCmd(uart_rcc[uart_num], ENABLE);

    RCC_APB2PeriphClockCmd(uart_gpio_rcc[uart_num], ENABLE);

    /* Configure USART Rx as input floating */
    GPIO_InitStructure.GPIO_Pin  = uart_pin[uart_num * 2];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(uart_port[uart_num], &GPIO_InitStructure);

    /* Configure USART Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   = uart_pin[uart_num * 2 + 1];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(uart_port[uart_num], &GPIO_InitStructure);

    /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = uart_irqn[uart_num];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Disable the USART */
    USART_Cmd(uart[uart_num], DISABLE);
    /* Configure USART */
    USART_InitStructure.USART_BaudRate            = baud_rate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart[uart_num], &USART_InitStructure);

    /* Enable USART Receive interrupts */
    USART_ITConfig(uart[uart_num], USART_IT_RXNE, ENABLE);
    /* Enable the USART */
    USART_Cmd(uart[uart_num], ENABLE);
}

void USART1_IRQHandler(void)
{
    uint8_t data;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    { 
        /* Read one byte from the receive data register */
        data = USART_ReceiveData(USART1);
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
    if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)
    {
        USART_ClearFlag(USART1,USART_FLAG_ORE);
        USART_ReceiveData(USART1);
        ibox_printf(1, ("usart1 USART_FLAG_ORE is set\r\n"));
    }
}
