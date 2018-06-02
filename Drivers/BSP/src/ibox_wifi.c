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
#include "string.h"

static uint8_t uart3_rx_buf[UART3_RX_SIZE];
uint16_t uart3_rx_wr_index = 0; // uart3接收buf位置标记
uint16_t uart3_rx_lines    = 0; // uart3接收到的行数计数器
uint16_t uart3_rx_count    = 0; // uart3接收到的字符串计数器
uint16_t uart3_rx_re_index = 0; // uart3读取位置标记

uint8_t wifi_status_error_count = 0; // wifi状态机错误计数器
uint8_t wifi_link_error_count   = 0; // wifi连接错误计数器
uint8_t wifi_error_count        = 0; // wifi错误计数器
uint32_t wifi_timeout           = 0; // wifi连接超时

ESP8266_STATUS_ENUM esp8266_status = ESP8266_STATUS_CHECK;

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
/*不采用透传方式，断网之后好不判断*/
static void esp8266_at_fsm(void)
{
    static uint8_t wifi_rx_temp[UART3_RX_SIZE];
    uint16_t wifi_rx_len = 0;

    memset(wifi_rx_temp, 0, UART3_RX_SIZE);
    /*获取一帧数据*/
    wifi_rx_len = get_line_from_uart3(wifi_rx_temp);
    if (wifi_rx_len) {
        ibox_printf(ibox_wifi_debug, ("wifi rec:%s\r\n", wifi_rx_temp));
    } else {
        return;
    }

    switch (esp8266_status) {
    case ESP8266_STATUS_CHECK: //检测模块
        wifi_status_error_count = 0;
        uart3_send_data("AT\r\n");
        esp8266_status = ESP8266_STATUS_WAIT_CHECK;
        break;
    case ESP8266_STATUS_WAIT_CHECK:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            uart3_send_data("AT+CWMODE=3\r\n"); /*设置模式*/
            esp8266_status = ESP8266_STATUS_WAIT_SET_MODE;
        } else {
            if (wifi_timeout - get_sys_time_s() > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5) {
                    wifi_link_error_count++;
                } else {
                    uart3_send_data("AT\r\n");
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_CHECK;
        }
        break;
    case ESP8266_STATUS_WAIT_SET_MODE:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            uart3_send_data("AT+RST\r\n"); /*重启模块*/
            esp8266_status = ESP8266_STATUS_WAIT_RESET;
        } else {
            if (wifi_timeout - get_sys_time_s() > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_data("AT+CWMODE=3\r\n");
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_SET_MODE;
        }
        break;
    case ESP8266_STATUS_WAIT_RESET:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            uart3_send_data("AT+CWJAP=\"WIFI_SSID\",\"WIFI_Password\"\r\n"); /*连接*/
            esp8266_status = ESP8266_STATUS_WAIT_LINK;
        } else {
            if (wifi_timeout - get_sys_time_s() > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_data("AT+RST\r\n"); /*重启模块*/
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_RESET;
        }
        break;
    case ESP8266_STATUS_WAIT_LINK:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            uart3_send_data("AT+CIFSR\r\n"); /*查询IP*/
            esp8266_status = ESP8266_STATUS_GET_MAC;
        } else {
            if (wifi_timeout - get_sys_time_s() > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5){
                    wifi_link_error_count++;
                } else {
                    uart3_send_data("AT+CWJAP=\"WIFI_SSID\",\"WIFI_Password\"\r\n"); /*连接*/
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_LINK;
        }
        break;
    case ESP8266_STATUS_GET_MAC:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            /*将MAC写入一个变脸,认证的时候需要*/
            /*两种连接方式,域名连接方式命令验证*/
            uart3_send_data("AT+CDNSGIP=lot.zxbike.cc\r\n");
            uart3_send_data("AT+CIPSTART=\"TCP\",\"SERVER_IP\",\"SERVER_PORT\"\r\n"); /*连接服务器*/
            esp8266_status = ESP8266_STATUS_WAIT_LINK_SERVER;
        } else {
            if (wifi_timeout - get_sys_time_s() > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_data("AT+CIFSR\r\n"); /*查询IP*/
                }
            }
            esp8266_status = ESP8266_STATUS_GET_MAC;
        }
        break;
    case ESP8266_STATUS_WAIT_LINK_SERVER:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            /*发送认证信息MAC*/
            uart3_send_data("AT+CIPSEND=6\r\n");
            uart3_send_data("AT+CIPSEND=4\r\n"); //发送MAC地址
            esp8266_status = ESP8266_STATUS_WAIT_SEND_AUTHEN;
        } else {
            if (wifi_timeout - get_sys_time_s() > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_data("AT+CIPSTART=\"TCP\",\"SERVER_IP\",\"SERVER_PORT\"\r\n"); /*连接服务器*/
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_LINK_SERVER;
        }
        break;
    case ESP8266_STATUS_WAIT_SEND_AUTHEN:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            /*发送获取RTC时间*/
            uart3_send_data("AT+CIPSEND=6\r\n");
            uart3_send_data("AT+CIPSEND=4\r\n"); //发送获取RTC，其他认证信息
            esp8266_status = ESP8266_STATUS_CHECK_AUTHEN;
        } else {
            if (wifi_timeout - get_sys_time_s() > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5) //超过错误计数值
                {
                    wifi_link_error_count++;
                } else {
                   uart3_send_data("AT+CIPSEND=4\r\n"); //发送MAC地址
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_SEND_AUTHEN;
        }
        break;
    case ESP8266_STATUS_CHECK_AUTHEN: //收到认证信息(RTC时间等其他的信息)
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            /*发送获取RTC时间*/
            uart3_send_data("AT+CIPSEND=6\r\n");
            uart3_send_data("AT+CIPSEND=4\r\n"); //发送MAC地址
            // 标记已经连接的状态
            //esp8266_status = ESP8266_STATUS_WAIT_SEND_AUTHEN;
        } else {
            if (wifi_timeout - get_sys_time_s() > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_data("AT+CIPSEND=4\r\n"); /*发送MAC地址*/
                }
            }
            esp8266_status = ESP8266_STATUS_CHECK_AUTHEN;
        }
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
}
void uart3_send_data(const char *data)
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
        if (data == 0x0d) //回车符
            uart3_rx_lines++;
        uart3_rx_buf[uart3_rx_wr_index] = data;
        if (++uart3_rx_wr_index == UART3_RX_SIZE)
            uart3_rx_wr_index = 0;
        if (++uart3_rx_count == UART3_RX_SIZE) {
            uart3_rx_wr_index = 0;
            uart3_rx_lines    = 0;
            uart3_rx_count    = 0;
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
    data_len = 0;
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
    USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);
    --uart3_rx_count;
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    return data;
}
