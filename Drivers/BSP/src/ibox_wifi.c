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
#include "ibox_board.h"
#include "ibox_sys.h"
#include "ibox_uart.h"
#include "network.h"

#include <stdio.h>
#include <string.h>

uint8_t wifi_status_error_count = 0; // wifi状态机错误计数器
uint8_t wifi_link_error_count   = 0; // wifi连接错误计数器
uint8_t wifi_error_count        = 0; // wifi错误计数器
uint8_t wifi_tcp_error_count    = 0; // wifi TCP连接错误计数器
uint32_t wifi_timeout           = 0; // wifi连接超时

uint8_t wifi_send_flag = 1; // wifi数据发送标志位

uint16_t wifi_tx_len               = 0; // wifi需要发送的数据长度
ESP8266_STATUS_ENUM esp8266_status = ESP8266_STATUS_CHECK;

#ifdef USE_WIFI
static void wifi_server_connect(void);
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
void esp8266_at_fsm(void)
{
    static uint8_t wifi_rx_temp[UART3_RX_SIZE];

    char *p_strstr = NULL;

    uint16_t wifi_rx_len      = 0;
    uint16_t wifi_rx_temp_len = 0;
    // uint16_t  count = 0;
    // char atoi_buf[3];
    uint8_t wifi_mode = 0;

    memset(wifi_rx_temp, 0, UART3_RX_SIZE);

    /*获取一帧数据*/
    wifi_rx_len = get_line_from_uart3(wifi_rx_temp);
    if (wifi_rx_len)
    {
        ibox_printf(ibox_wifi_debug, ("[WIFI_REC]->:%s\r\n", wifi_rx_temp));
    }

    switch (esp8266_status)
    {
    case ESP8266_STATUS_CHECK: //检测模块；ATE0关闭回显
        uart3_rx_buf_clear();
        uart3_send_str("AT\r\n");
        wifi_status_error_count = 0;
        wifi_timeout            = get_sys_time_s();
        esp8266_status          = ESP8266_STATUS_CHECK_MODE;
        break;
    case ESP8266_STATUS_CHECK_MODE: //检测模块模式
        if (strstr((char *) wifi_rx_temp, "OK") != NULL)
        {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            ibox_printf(ibox_wifi_debug, ("AT+CWMODE?\r\n"));
            uart3_send_str("AT+CWMODE?\r\n");
            esp8266_status = ESP8266_STATUS_WAIT_CHECK_MODE;
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 2)
            {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_error_count++;
                    wifi_status_error_count = 0;
                }
                else
                {
                    uart3_send_str("AT\r\n");
                }
            }
        }
        break;
    case ESP8266_STATUS_WAIT_CHECK_MODE: //等待检测模式
        p_strstr = NULL;
        p_strstr = strstr((char *) wifi_rx_temp, "+CWMODE:");
        if (p_strstr != NULL)
        {
            wifi_mode = p_strstr[8] - '0';
            ibox_printf(ibox_wifi_debug, ("AT+CWMODE=%d\r\n", wifi_mode));
            if (wifi_mode == 1)
            {
                esp8266_status = ESP8266_STATUS_LINK_OK; //模式正确尝试获取本机IP
            }
            else
            {
                esp8266_status = ESP8266_STATUS_SET_MODE;
            }
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 2)
            {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_error_count++;
                    wifi_status_error_count = 0;
                }
                else
                {
                    ibox_printf(ibox_wifi_debug, ("AT+CWMODE?\r\n"));
                    uart3_send_str("AT+CWMODE?\r\n");
                }
            }
        }
        break;
    case ESP8266_STATUS_SET_MODE:
        wifi_status_error_count = 0;
        wifi_timeout            = get_sys_time_s();
        ibox_printf(ibox_wifi_debug, ("set mode <AT+CWMODE=1>\r\n"));
        uart3_send_str("AT+CWMODE=1\r\n"); /*设置模式为Station*/
        esp8266_status = ESP8266_STATUS_WAIT_SET_MODE;
        uart3_rx_buf_clear();
        break;
    case ESP8266_STATUS_WAIT_SET_MODE:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL)
        {
//            esp8266_status = ESP8266_STATUS_RESET;
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 3)
            {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_error_count++;
                    wifi_status_error_count = 0;
                }
                else
                {
                    ibox_printf(ibox_wifi_debug, ("set mode <AT+CWMODE=1>\r\n"));
                    uart3_send_str("AT+CWMODE=1\r\n");
                }
            }
        }
        break;
    case ESP8266_STATUS_RESET: //增加reset状态，wifi有问题可以调到该状态
        wifi_status_error_count = 0;
        wifi_timeout            = get_sys_time_s();
        uart3_send_str("AT+RST\r\n"); /*重启模块*/
        ibox_printf(ibox_wifi_debug, ("reset wifi mode！\r\n"));
        esp8266_status = ESP8266_STATUS_WAIT_RESET;
        break;
    case ESP8266_STATUS_WAIT_RESET: //重启完成后关闭回显
        if (strstr((char *) wifi_rx_temp, "OK") != NULL)
        {
            sys_delay_ms(2000);
            uart3_rx_buf_clear();
            uart3_send_str("ATE0\r\n");
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status          = ESP8266_STATUS_WAIT_CLOSE_ECHO;
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 4)
            {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_error_count++;
                    wifi_status_error_count = 0;
                }
                else
                {
                    esp8266_status = ESP8266_STATUS_RESET;
                }
            }
        }
        break;
    case ESP8266_STATUS_WAIT_CLOSE_ECHO:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL)
        {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status          = ESP8266_STATUS_WIFI_CONNECT;
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 3)
            {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_error_count++;
                    wifi_status_error_count = 0;
                }
                else
                {
                    uart3_send_str("AT\r\n");
                }
            }
        }
        break;
    case ESP8266_STATUS_WIFI_CONNECT:
        uart3_rx_buf_clear();
        wifi_status_error_count = 0;
        wifi_timeout            = get_sys_time_s();
        uart3_send_str("AT+CWJAP=\"");
        uart3_send_str((const char *)ibox_config.wifi_ssid);
        uart3_send_str("\",\"");
        uart3_send_str((const char *)ibox_config.wifi_password);
        uart3_send_str("\"\r\n");
        ibox_printf(ibox_wifi_debug, ("wifi connect....\r\n"));
        esp8266_status = ESP8266_STATUS_WAIT_CONNECTED;
        break;
    case ESP8266_STATUS_WAIT_CONNECTED:
        if (strstr((char *) wifi_rx_temp, "WIFI CONNECTED") != NULL)
        {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status          = ESP8266_STATUS_WAIT_GOT_IP;
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 3)
            {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_status_error_count = 0;
                    wifi_error_count++;
                }
                else
                {
                    esp8266_status = ESP8266_STATUS_WIFI_CONNECT;
                }
            }
        }
        break;
    case ESP8266_STATUS_WAIT_GOT_IP:
        if (strstr((char *) wifi_rx_temp, "WIFI GOT IP") != NULL)
        {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status          = ESP8266_STATUS_LINK_OK;
            ibox_printf(ibox_wifi_debug, ("wifi connect ok!\r\n"));
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 4)
            {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_status_error_count = 0;
                    wifi_error_count++;
                }
                else
                {
                    esp8266_status = ESP8266_STATUS_WIFI_CONNECT;
                }
            }
        }
        break;
    case ESP8266_STATUS_LINK_OK:
        wifi_status_error_count = 0;
        wifi_timeout            = get_sys_time_s();
        uart3_rx_buf_clear();
        uart3_send_str("AT+CIFSR\r\n"); /*查询IP*/
        ibox_printf(ibox_wifi_debug, ("wifi send <AT+CIFSR>\r\n"));
        esp8266_status = ESP8266_STATUS_GET_IP;
        break;
    /*获得IP才是正真的连接*/
    case ESP8266_STATUS_GET_IP:
        if (strstr((char *) wifi_rx_temp, "+CIFSR:STAIP,") != NULL)
        {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            sscanf((char *) wifi_rx_temp, "%*[^\"]\"%[^\"]", ibox_config.wifi_ip);
            if (strcmp((char *) ibox_config.wifi_ip, "0.0.0.0") == 0)
            {
                esp8266_status = ESP8266_STATUS_WIFI_CONNECT;
                ibox_printf(ibox_wifi_debug, ("wifi get ip error reconnect..\r\n"));
            }
            else
            {
                esp8266_status = ESP8266_STATUS_GET_MAC;
                ibox_printf(ibox_wifi_debug, ("wifi get ip ok..\r\n"));
            }
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 3)
            {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_status_error_count = 0;
                    wifi_error_count++;
                }
                else
                {
                    esp8266_status = ESP8266_STATUS_LINK_OK;
                }
            }
        }
        break;
    case ESP8266_STATUS_GET_MAC:
        if (strstr((char *) wifi_rx_temp, "+CIFSR:STAMAC,") != NULL)
        {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            /*将MAC写入一个变量,认证的时候需要*/
            sscanf((char *) wifi_rx_temp, "%*[^\"]\"%[^\"]", ibox_config.wifi_mac);
            uart3_rx_buf_clear(); //要等待上面的状态完成,这个地方需要需要等待OK
            ibox_printf(ibox_wifi_debug, ("wifi get MAC ok..\r\n"));
            esp8266_status = ESP8266_STATUS_CONNECT_SERVER;
        }
        else
        {
            if (get_sys_time_s() - wifi_timeout > 5)
            {
                ibox_printf(ibox_wifi_debug, ("wifi get MAC error restart..\r\n"));
                wifi_error_count++;
                esp8266_status          = ESP8266_STATUS_RESET;
                wifi_status_error_count = 0;
            }
        }
        break;
    case ESP8266_STATUS_RESTART_CONNECT_SEVER:
        uart3_rx_buf_clear();
        uart3_send_str("AT+CIPCLOSE\r\n"); /*关闭TCP*/
        sys_delay_ms(1000);        
        esp8266_status = ESP8266_STATUS_CONNECT_SERVER;
        break;
    case ESP8266_STATUS_CONNECT_SERVER:
        uart3_rx_buf_clear();
        wifi_status_error_count = 0;
        wifi_timeout            = get_sys_time_s();
        wifi_server_connect();
        esp8266_status = ESP8266_STATUS_WAIT_LINK_SERVER;
        break;
    case ESP8266_STATUS_WAIT_LINK_SERVER:
        if (strstr((char *) wifi_rx_temp, "ERROR") != NULL)
        {
            wifi_tcp_error_count++;
            esp8266_status = ESP8266_STATUS_CONNECT_SERVER;
            break;
        }
        if (strstr((char *) wifi_rx_temp, "CONNECT") != NULL)
        {
            wifi_tcp_error_count    = 0;
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status          = ESP8266_STATUS_COMMUNICATE;
            ibox_printf(ibox_wifi_debug, ("wifi connect sever ok..\r\n"));
            uart3_rx_buf_clear();
            break;
        }
        if (get_sys_time_s() - wifi_timeout > 3)
        {
            wifi_timeout = get_sys_time_s();
            wifi_status_error_count++;
            if (wifi_status_error_count > 5)
            {
                wifi_error_count++;
            }
            else
            {
                esp8266_status = ESP8266_STATUS_CONNECT_SERVER;
            }
        }
        break;
    case ESP8266_STATUS_COMMUNICATE:
        // 判断是否有数据接收
        if (wifi_rx_len)
        {
            /*重新连接WIFI*/
            if (strstr((char *) wifi_rx_temp, "ERROR") != NULL)
            {
                esp8266_status = ESP8266_STATUS_WIFI_CONNECT;
                break;
            }
            /*TCP断开了,重新连接*/
            if (strstr((char *) wifi_rx_temp, "CLOSED") != NULL)
            {
                esp8266_status = ESP8266_STATUS_RESTART_CONNECT_SEVER;
                break;
            }
            if (strstr((char *) wifi_rx_temp, "SEND OK") != NULL)
            {
                wifi_tx_len    = 0;
                wifi_send_flag = 1;
                break;
            }
            p_strstr = NULL;
            p_strstr = strstr((char *) wifi_rx_temp, "+IPD,");
            if (p_strstr != NULL)
            {
                wifi_rx_temp_len = 0;
                p_strstr += 5;
                while (*p_strstr != ':')
                {
                    wifi_rx_temp_len = wifi_rx_temp_len * 10 + *p_strstr - '0';
                    p_strstr++;
                }
                p_strstr++;
                /*
                //使用cJSON直接处理字符串，不用下面的转换
                for (count = 0; count < wifi_rx_temp_len; count++) {
                    atoi_buf[0] = *(p_strstr + count * 2);
                    atoi_buf[1] = *(p_strstr + (count * 2 + 1));
                    net_rx_bottom_buf[count] = strtol((const char *) atoi_buf, NULL, 16);
                }
                */
                net_rx_write(p_strstr, wifi_rx_temp_len);
            }
            break;
        }
                // 判断是否有数据要发送
        if (wifi_tx_len && wifi_send_flag)
        {
            wifi_timeout   = get_sys_time_s();
            wifi_send_flag = 0;
            esp8266_send_data();
        }
        if ((get_sys_time_s() - wifi_timeout > 5) && wifi_send_flag)
        {
            /*有可能是wifi掉了,或者是TCP断了，重新来一遍*/
            ibox_printf(ibox_wifi_debug, ("wifi data send timeout...\r\n"));
            uart3_send_str("AT+CIPCLOSE\r\n"); /*关闭TCP*/
            sys_delay_ms(1000);
            esp8266_status = ESP8266_STATUS_CHECK;
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
    sys_delay_ms(2000);
    uart3_rx_buf_clear();
}

/**
 * ESP8266发送数据底层接口函数
 * 要发送的数据在uart3_tx_buf[]中，数据的长度写在全局变量wifi_tx_len中.
 */
void esp8266_send_data(void)
{
    uint16_t i;
    uint8_t temp[20];

    memset(temp, 0, 20);
    sprintf((char *) temp, "AT+CIPSEND=%d\r\n", wifi_tx_len);
    uart3_send_str((char *) temp);
    sys_delay_ms(50);
    uart3_send_data(temp, 10);
    wifi_tx_len = 0;
}
ESP8266_STATUS_ENUM get_esp8266_status(void)
{
    return esp8266_status;
}
static void wifi_server_connect(void)
{
    char temp_buf[6];
    uart3_send_str("AT+CIPSTART=\"TCP\",\"");

    /* tcp错误计数超了 选用原有域名重新连接服务器.或者是默认使用IP连接服务器*/
    if ((ibox_config.use_dns == 0) || (wifi_tcp_error_count >= 3))
    {
        ibox_printf(ibox_wifi_debug,
                    ("wifi tcp connect err out range || default use the ip \"%s\" port \"%d\"connect\r\n",
                     ibox_config.server_ip, ibox_config.server_port));
        uart3_send_str(ibox_config.server_ip);
        uart3_send_str("\",");
    }
    else
    {
        ibox_printf(ibox_wifi_debug, ("wifi tcp connect use the DNS \"%s\" port \"%d\"connect\r\n",
                                      ibox_config.server_dsn, ibox_config.server_port));
        uart3_send_str(ibox_config.server_dsn);
        uart3_send_str("\",");
    }
    memset((uint8_t *) temp_buf, 0, 6);
    sprintf(temp_buf, "%d", ibox_config.server_port);
    uart3_send_str(temp_buf);
    uart3_send_str("\r\n");
}
#endif
