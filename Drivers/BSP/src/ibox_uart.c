#include "ibox_uart.h"
#include "stdio.h"
#include "string.h"

uint32_t uart_pin[6]       = {UART_DEBUG_PIN1, UART_DEBUG_PIN2, UART_GPRS_WIFI_PIN1,
                        UART_GPRS_WIFI_PIN2, UART_485_PIN1,   UART_485_PIN2};
GPIO_TypeDef *uart_port[3] = {UART_DEBUG_PORT, UART_GPRS_WIFI_PORT, UART_485_PORT};
uint32_t uart_rcc[3]       = {UART_DEBUG_UART_RCC_CLK, UART_GPRS_WIFI_UART_RCC_CLK, UART_485_UART_RCC_CLK};
uint32_t uart_gpio_rcc[3]  = {UART_DEBUG_GPIO_RCC_CLK,UART_GPRS_WIFI_GPIO_RCC_CLK,UART_485_GPIO_RCC_CLK};
USART_TypeDef *uart[3]     = {USART1, USART3, UART4};
uint8_t uart_irqn[3] = {USART1_IRQn,USART3_IRQn, UART4_IRQn};

static uint8_t uart1_rx_buf[UART1_RX_SIZE];
static uint16_t uart1_rx_wr_index = 0;
static uint16_t uart1_rx_re_index = 0;

uint8_t uart3_rx_buf[UART3_RX_SIZE];
uint8_t uart3_tx_buf[UART3_TX_SIZE];

uint16_t uart3_rx_wr_index = 0; // uart3接收buf位置标记
uint16_t uart3_rx_lines    = 0; // uart3接收到的行数计数器
uint16_t uart3_rx_count    = 0; // uart3接收到的字符串计数器
uint16_t uart3_rx_re_index = 0; // uart3读取位置标记


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
/**
 * UART1中断服务函数
 */
extern void rt_hw_console_getchar_callback(void);
void USART1_IRQHandler(void)
{
    uint8_t data = 0;

    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    { 
        /* Read one byte from the receive data register */
        data = USART_ReceiveData(USART1);
        printf("%c",data);
        uart1_rx_buf[uart1_rx_wr_index] = data;
        uart1_rx_wr_index += 1;
        if (uart1_rx_wr_index >= UART1_RX_SIZE) 
            uart1_rx_wr_index = 0;
        if(uart1_rx_wr_index == uart1_rx_re_index) {
            uart1_rx_re_index += 1;  
            if(uart1_rx_re_index >= UART1_RX_SIZE)
                uart1_rx_re_index = 0;
        }
        rt_hw_console_getchar_callback();
    }
    if(USART_GetFlagStatus(USART1,USART_FLAG_ORE) == SET)
    {
        USART_ClearFlag(USART1,USART_FLAG_ORE);
        USART_ReceiveData(USART1);
        ibox_printf(1, ("usart1 USART_FLAG_ORE is set\r\n"));
    }
}

uint8_t get_char_form_uart1(char *ch)
{
    char data = 0;

    if(uart1_rx_re_index != uart1_rx_wr_index){
        data = uart1_rx_buf[uart1_rx_re_index];
        uart1_rx_re_index += 1;
        if(uart1_rx_re_index >= UART1_RX_SIZE)
            uart1_rx_re_index = 0;

        *ch = data & 0xff;
        return 1;   
    }
    else
    {
        return 0;
    }
}
void uart3_send_data(uint8_t*data, uint16_t len)
{
    uint32_t timeout = 0;
    while (len) {
        timeout = get_sys_time_s();
        USART_SendData(USART3, (uint8_t) *data++);
        len--;
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {
            if (get_sys_time_s() - timeout > 1)
                break;
        }
    }
}
void uart3_send_str(const char *data)
{
    uint32_t timeout = 0;

    while (*data) {
        timeout = get_sys_time_s();
        USART_SendData(USART3, (uint8_t) *data++);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {
            if (get_sys_time_s() - timeout > 1)
                break;
        }
    }
}

void USART3_IRQHandler(void)
{
    uint8_t data;
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        /* Read one byte from the receive data register */
        data = USART_ReceiveData(USART3);
        printf("%c",data);
        if (data == 0x0d) //回车符
            uart3_rx_lines++;
        uart3_rx_buf[uart3_rx_wr_index] = data;
        if (++uart3_rx_wr_index == UART3_RX_SIZE)
            uart3_rx_wr_index = 0;
        /*uart3_rx_buf[]满了，溢出了。*/
        if (++uart3_rx_count == UART3_RX_SIZE) {
            uart3_rx_wr_index = 0;
            uart3_rx_lines    = 0;
            uart3_rx_count    = 0;
            uart3_rx_re_index = 0;
            memset(uart3_rx_buf, 0, UART3_RX_SIZE);
            ibox_printf(ibox_wifi_debug, ("uart3 USART_FLAG_ORE is set\r\n"));
        }
    }
    if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) == SET) {
        USART_ClearFlag(USART3, USART_FLAG_ORE);
        USART_ReceiveData(USART3);
        ibox_printf(ibox_wifi_debug, ("uart3 USART_FLAG_ORE is set\r\n"));
    }
}
//返回的是读取数据的长度
uint16_t get_line_from_uart3(uint8_t *buf)
{
    uint8_t data      = 0;
    uint16_t data_len = 0;

    if (!uart3_rx_lines) //无数据退出
    {
        buf = NULL;
        return 0;
    }
    while (1) {
        if (uart3_rx_count) {
            data = get_char_form_uart3();
        } else {
            uart3_rx_lines = 0;
            buf            = NULL;
            return 0;
        }
        if (data == 0x0a) //换行直接丢掉
            continue;
        if (data == 0x0d) { //回车，收到一帧
            uart3_rx_lines--;
            *buf = '\0';
            return data_len;
        }
        *buf = data;
        buf++;
        data_len++;
    }
}

uint8_t get_char_form_uart3(void)
{
    uint8_t data;
    while (uart3_rx_count == 0) {
        ibox_printf(ibox_wifi_debug, ("uart3 rx count is 0\r\n"));
    }
    data = uart3_rx_buf[uart3_rx_re_index];
    if (++uart3_rx_re_index == UART3_RX_SIZE)
        uart3_rx_re_index = 0;
    USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);
    --uart3_rx_count;
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    return data;
}
//清除uart3的接收buf。
void uart3_rx_buf_clear(void)
{
    uart3_rx_wr_index = 0;
    uart3_rx_lines    = 0;
    uart3_rx_count    = 0;
    uart3_rx_re_index = 0;
    memset(uart3_rx_buf, 0, UART3_RX_SIZE);
}
