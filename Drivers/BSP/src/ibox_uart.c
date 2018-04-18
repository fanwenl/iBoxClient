#include "ibox_uart.h"
#include "stdio.h"

uint32_t uart_pin[6]       = {UART_DEBUG_PIN1,     UART_DEBUG_PIN2, UART_GPRS_WIFI_PIN1,
                        UART_GPRS_WIFI_PIN2, UART_485_PIN1,   UART_485_PIN2};
GPIO_TypeDef *uart_port[3] = {UART_DEBUG_PORT, UART_GPRS_WIFI_PORT, UART_485_PORT};
uint32_t uart_rcc[3]       = {UART_DEBUG_RCC_CLK, UART_GPRS_WIFI_RCC_CLK, UART_485_RCC_CLK};
USART_TypeDef *uart[3]     = {USART1, USART3, UART4};

void uart_init(uart_enum uart_num, uint32_t baud_rate)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(uart_rcc[uart_num], ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* Configure USART1 Rx as input floating */
    GPIO_InitStructure.GPIO_Pin  = uart_pin[uart_num * 2];
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(uart_port[uart_num], &GPIO_InitStructure);

    /* Configure USART1 Tx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin   = uart_pin[uart_num * 2 + 1];
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(uart_port[uart_num], &GPIO_InitStructure);

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /* Disable the USART1 */
    USART_Cmd(uart[uart_num], DISABLE);
    /* Configure USART1 */
    USART_InitStructure.USART_BaudRate            = baud_rate;
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;
    USART_InitStructure.USART_Parity              = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart[uart_num], &USART_InitStructure);

    /* Enable USART1 Receive interrupts */
    USART_ITConfig(uart[uart_num], USART_IT_RXNE, ENABLE);
    /* Enable the USART1 */
    USART_Cmd(uart[uart_num], ENABLE);
}
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (unsigned char) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
    }
    return (ch);
}
