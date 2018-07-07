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
#include "ibox_board.h"
#include "stdio.h"

uint8_t wifi_status_error_count = 0; // wifi状态机错误计数器
uint8_t wifi_link_error_count   = 0; // wifi连接错误计数器
uint8_t wifi_error_count        = 0; // wifi错误计数器
uint32_t wifi_timeout           = 0; // wifi连接超时

uint16_t wifi_tx_len = 0; //wifi需要发送的数据长度
ESP8266_STATUS_ENUM esp8266_status = ESP8266_STATUS_CHECK;


#ifdef USE_WIFI
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

    uint16_t wifi_rx_len = 0;
    uint8_t wifi_mode = 0;

    memset(wifi_rx_temp, 0, UART3_RX_SIZE);
    /*获取一帧数据*/
    wifi_rx_len = get_line_from_uart3(wifi_rx_temp);
    if (wifi_rx_len) {
        ibox_printf(ibox_wifi_debug, ("[WIFI_REC]:%s\r\n", wifi_rx_temp));
    } else {
//        return;
    }

    switch (esp8266_status) {
    case ESP8266_STATUS_CHECK: //检测模块；ATE0关闭回显
        uart3_rx_buf_clear();
        uart3_send_str("ATE0\r\n");
        wifi_status_error_count = 0;
        wifi_timeout            = get_sys_time_s();
        esp8266_status = ESP8266_STATUS_CHECK_MODE;
        break;
    case ESP8266_STATUS_CHECK_MODE: //检测模块模式
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            //ibox_printf(ibox_wifi_debug, ("AT+CWMODE?\r\n"));
            uart3_send_str("AT+CWMODE?\r\n"); 
            esp8266_status = ESP8266_STATUS_WAIT_CHECK_MODE;
        } else {
            if (get_sys_time_s() - wifi_timeout > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5) {
                    wifi_link_error_count++;
                } else {
                    uart3_send_str("AT\r\n");
                }
            }
//            esp8266_status = ESP8266_STATUS_CHECK_MODE;
        }
        break;
    case ESP8266_STATUS_WAIT_CHECK_MODE: //等待检测模式
        if (strstr((char *) wifi_rx_temp, "+CWMODE:") != NULL) {
            sscanf((char *) wifi_rx_temp, "+CWMODE:%d", (int *)&wifi_mode);
            if (wifi_mode == 1) {
                esp8266_status = ESP8266_STATUS_LINK_OK;        //模式正确尝试获取本机IP
            } else {
                esp8266_status = ESP8266_STATUS_SET_MODE;
            }
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
        } else {
            if (get_sys_time_s() - wifi_timeout > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5) {
                    wifi_link_error_count++;
                } else {
                    uart3_send_str("AT\r\n");
                }
            }
//            esp8266_status = ESP8266_STATUS_WAIT_CHECK;
        }
        break;
    //先这样调试，等这个好了之后，增加模式查询状态，如果状态对不在走重启状态
    case ESP8266_STATUS_SET_MODE:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            //ibox_printf(ibox_wifi_debug, ("AT+CWMODE=3\r\n"));
            uart3_send_str("AT+CWMODE=1\r\n"); /*设置模式为Station*/
            esp8266_status = ESP8266_STATUS_WAIT_SET_MODE;
        } else {
            if (get_sys_time_s() - wifi_timeout > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5) {
                    wifi_link_error_count++;
                } else {
                    uart3_send_str("AT\r\n");
                }
            }
//            esp8266_status = ESP8266_STATUS_WAIT_CHECK;
        }
        break;
    case ESP8266_STATUS_WAIT_SET_MODE:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            uart3_send_str("AT+RST\r\n"); /*重启模块*/
            esp8266_status = ESP8266_STATUS_WAIT_RESET;
        } else {
            if (get_sys_time_s() - wifi_timeout > 3) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_str("AT+CWMODE=3\r\n");
                }
            }
//            esp8266_status = ESP8266_STATUS_WAIT_SET_MODE;
        }
        break;
        /*检测重启命令发送成功OK,不检测重启完成的ready*/
    case ESP8266_STATUS_WAIT_RESET: //重启完成后关闭回显
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            sys_delay_ms(2000);
            uart3_rx_buf_clear();
            uart3_send_str("ATE0\r\n");
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status          = ESP8266_STATUS_WAIT_CLOSE_ECHO;
        } else {
            if (get_sys_time_s() - wifi_timeout > 3) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5) {
                    wifi_link_error_count++;
                } else {
                    uart3_send_str("AT+RST\r\n");
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_RESET;
        }
        break;
    case ESP8266_STATUS_WAIT_CLOSE_ECHO:
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            memset(uart3_tx_buf, 0, UART3_TX_SIZE);
            sprintf((char *)uart3_tx_buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", ibox_config.wifi_ssid, ibox_config.wifi_password); 
            uart3_send_str((char *)uart3_tx_buf); /*连接*/
            esp8266_status = ESP8266_STATUS_WAIT_CONNECTED;
        } else {
            if (get_sys_time_s() - wifi_timeout > 3) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_str("AT\r\n");
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_CLOSE_ECHO;
        }
        break;
     case ESP8266_STATUS_WAIT_CONNECTED:
        if (strstr((char *) wifi_rx_temp, "WIFI CONNECTED") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status = ESP8266_STATUS_WAIT_GOT_IP;
        } else {
            if (get_sys_time_s() - wifi_timeout > 3) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5){
                    wifi_link_error_count++;
                } else {
                    uart3_send_str((char *)uart3_tx_buf); /*连接*/
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_CONNECTED;
        }
        break;
    case ESP8266_STATUS_WAIT_GOT_IP:
        if (strstr((char *) wifi_rx_temp, "WIFI GOT IP") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status = ESP8266_STATUS_LINK_OK;
        } else {
            if (get_sys_time_s() - wifi_timeout > 3) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5){
                    wifi_link_error_count++;
                } else {
                    uart3_send_str((char *)uart3_tx_buf); /*连接*/
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_GOT_IP;
        }
        break;
    case ESP8266_STATUS_LINK_OK:
       if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            uart3_send_str("AT+CIFSR\r\n"); /*查询IP*/
            esp8266_status = ESP8266_STATUS_GET_IP;
        } else {
            if (get_sys_time_s() - wifi_timeout > 3) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5){
                    wifi_link_error_count++;
                } else {
                    uart3_send_str((char *)uart3_tx_buf); /*连接*/
                }
            }
//            esp8266_status = ESP8266_STATUS_WAIT_LINK;
        }
        break;
    /*获得IP才是正真的连接*/
    case ESP8266_STATUS_GET_IP:
        if (strstr((char *) wifi_rx_temp, "+CIFSR:STAIP,") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            /*将MAC写入一个变量,认证的时候需要*/
            sscanf((char *) wifi_rx_temp, "%*[^\"]\"%[^\"]",ibox_config.wifi_ip);
            if(strcmp((char *)ibox_config.wifi_ip, "0.0.0.0") == 0) {
                esp8266_status = ESP8266_STATUS_WAIT_CLOSE_ECHO;
            }
            else {
                esp8266_status = ESP8266_STATUS_GET_MAC;
            }
        }
        else {
            if (get_sys_time_s() - wifi_timeout > 3) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5){
                    wifi_link_error_count++;
                }
            }
        }
        break;
        /*如果获取IP和MAC失败则可能是上一个的连接状态有问题，需要重新走一遍，这里需要注意一下*/
    case ESP8266_STATUS_GET_MAC:
        if (strstr((char *) wifi_rx_temp, "+CIFSR:STAMAC,") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            /*将MAC写入一个变量,认证的时候需要*/
            sscanf((char *) wifi_rx_temp, "%*[^\"]\"%[^\"]",ibox_config.wifi_mac);
            /*两种连接方式,域名连接方式命令验证*/
            //sprintf((char *)uart3_tx_buf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ibox_config.server_dsn, ibox_config.server_port); 
            uart3_rx_buf_clear();//要等待上面的状态完成,这个地方需要需要等待OK
            memset(uart3_tx_buf, 0, UART3_TX_SIZE);
            sprintf((char *)uart3_tx_buf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",ibox_config.server_ip, ibox_config.server_port); 
            uart3_send_str((char *)uart3_tx_buf); /*连接服务器*/
            esp8266_status = ESP8266_STATUS_WAIT_LINK_SERVER;
        } else {
            if (get_sys_time_s() - wifi_timeout > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_str("AT+CIFSR\r\n"); /*查询IP*/
                }
            }
            esp8266_status = ESP8266_STATUS_GET_MAC;
        }
        break;
    case ESP8266_STATUS_WAIT_LINK_SERVER:
        if (strstr((char *) wifi_rx_temp, "ERROR") != NULL) {

        }
        if (strstr((char *) wifi_rx_temp, "CONNECT") != NULL) {
            wifi_status_error_count = 0;
            wifi_timeout            = get_sys_time_s();
            esp8266_status = ESP8266_STATUS_COMMUNICATE;
        } else {
            if (get_sys_time_s() - wifi_timeout > 2) {
                wifi_timeout = get_sys_time_s();
                wifi_status_error_count++;
                if (wifi_status_error_count > 5)
                {
                    wifi_link_error_count++;
                } else {
                    uart3_send_str((char *)uart3_tx_buf); /*连接服务器*/
                }
            }
            esp8266_status = ESP8266_STATUS_WAIT_LINK_SERVER;
        }
        break;
    case ESP8266_STATUS_COMMUNICATE:
        // 判断是否有数据要发送
        if(wifi_tx_len)
        {
            esp8266_send_data();
            //需要有判断发送成功的标志，网络状态的标志。
            esp8266_status = ESP8266_STATUS_COMMUNICATE;
                /*ERROR
                [WIFI_REC]:AT+CIPSEND=20
                    [WIFI_REC]:link is not valid
            */
        }
        // 判断是否有数据接收
        if(wifi_rx_len)
        {
            //+IPD,11:list_thread
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
    sprintf((char *)temp, "AT+CIPSEND=%d\r\n", wifi_tx_len);
    uart3_send_str((char *)temp);
    sys_delay_ms(50);
    uart3_send_data(uart3_tx_buf, wifi_tx_len);
    wifi_tx_len = 0;
}
ESP8266_STATUS_ENUM get_esp8266_status(void)
{
    return esp8266_status;
}
#endif
