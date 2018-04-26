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
#include "ibox_sys.h"
#include "ibox_uart.h"

static uint8_t uart3_rx_buf[MAX485_UART4_RX_SIZE];
uint16_t uart3_rx_wr_index = 0;
uint16_t uart3_rx_lines = 0;

ESP8266_STATUS_ENUM esp8266_status = ESP8266_STATUS_INIT;

static void wifi_ctrl_pin_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(WIFI_RESET_RCC_CLOCK | WIFI_CH_PD_RCC_CLOCK, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin   = WIFI_RESET_PIN;

    GPIO_Init(WIFI_RESET_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.GPIO_Pin = WIFI_CH_PD_PIN;
    GPIO_Init(WIFI_CH_PD_PORT, &GPIO_InitStruct);
}
/*ESP8266联网状态机*/
static void esp8266_at_fsm(void)
{
    switch (esp8266_status) {
    case ESP8266_STATUS_INIT: //状态机及esp8266初始化
        break;
    case ESP8266_STATUS_CHECK: //检测模块
        break;
    case ESP8266_STATUS_SET_MODE: //设置模式
        break;
    case ESP8266_STATUS_RESET: //重启模块
        break;
    case ESP8266_STATUS_LINK: //连接wifi
        break;
    case ESP8266_STATUS_GET_IP: //查询IP
        break;
    case ESP8266_STATUS_LINK_SERVER: //连接服务器
        break;
    case ESP8266_STATUS_SEND_AUTHEN: //发送认证信息
        break;
    case ESP8266_STATUS_CHECK_AUTHEN: //收到认证信息
        break;
    case ESP8266_STATUS_SET_CIPMODE: //设置透传模式
        break;
    case ESP8266_STATUS_SETART_CIP: //开始透传
        break;
    case ESP8266_STATUS_END_CIP: //结束透传
        break;
    default:
        break;
    }
}

void wifi_init(void)
{
    wifi_ctrl_pin_config();

    uart_init(UART3_GPRS_WIFI, 115200);

    /*复位wifi模块*/
    WIFI_RESET_L;
    WIFI_CH_PD_L;
    sys_delay_ms(1000);
    WIFI_RESET_H;
    WIFI_CH_PD_H;

    uart3_send_str("AT\r\n");
}
void uart3_send_data(uint8_t *data)
{
    uint32_t timeout = 0;

    while (*data) {
        tiemout = get_sys_time();
        USART_SendData(USART3, (uint8_t) *data++);
        while (USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET) {
            if (get_sys_time() - timeout > 1)
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
        if (data == 0x0d) //回车符
            uart3_rx_lines++;
        uart3_rx_buf[uart3_rx_wr_index] = data;
        if (++uart3_rx_wr_index == UART3_RX_SIZE) {
            uart3_rx_wr_index = 0;
            uart3_rx_lines = 0;
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
        *buf = NULL;
        return 0;
    }
    data_len = 0;
    while (1) {
        if (rx_counter2) //于4月25日修改
        {
            data = GET_CHAR_FROM_GSM();
        } else {
            recvlines = 0;
            *buf      = NULL;
            return 0;
        }
        if (data == 0x0a)
            continue; //收到0x0A直接丢弃
        if (data == 0x0d) {
            recvlines--;
            *buf = '\0';
            return data_len;
        }
        *buf = data;
        buf++;
        data_len++;
    }
}

uint8_t get_char_form_uart3(void){
    uint8_t data;
    while (rx_counter2==0){
      EBIKE_DEBUG_LOG(ebike_debug_gprs,("getchar2 rx_counter2 is 0\r\n"));
    }
    data = rx_buffer2[rx_rd_index2];
//    if (bike_state.debug_mode) putchar4(data);
    if (++rx_rd_index2 == RX_BUFFER_SIZE2) rx_rd_index2=0;
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    --rx_counter2;
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    return data;
}
