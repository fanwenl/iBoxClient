/*
**************************************************************************************************
*文件：ibox_gprs.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-06-06
*描述：GPRS接口驱动，SIM800L。
* ************************************************************************************************
*/
#include "ibox_gprs.h"
#include "ibox_uart.h"
#include <string.h>

//Rst WIFI_RTT_B
//URXD_B WIFI_RXB
//TXD_WIFI_TXB
//RING WIFI_GPIOB
//DTR WIFI_CH_PD_B

GPRS_STATUS_ENUM gprs_status = GPRS_STATUS_INIT;
uint8_t gprs_status_error_count = 0; // wifi状态机错误计数器
uint8_t gprs_link_error_count   = 0; // wifi连接错误计数器
uint8_t gprs_error_count        = 0; // wifi错误计数器
uint32_t gprs_timeout           = 0; // wifi连接超时

static void gprs_pin_config(void);
void gprs_init(void)
{
    gprs_pin_config();

    uart_init(UART3_GPRS_WIFI, 115200);

    /*复位GPRS模块*/
    GPRS_RESET_L;
    sys_delay_ms(2);
    GPRS_RESET_H;
    uart3_rx_buf_clear();   
}
static void gprs_pin_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(GPRS_RESET_PIN_CLK, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin   = GPRS_RESET_PIN;

    GPIO_Init(GPRS_RESET_PORT, &GPIO_InitStruct);
}
void gprs_at_fsm(void)
{
    static uint8_t gprs_rx_temp[UART3_RX_SIZE];

    uint16_t gprs_rx_len = 0;


    memset(gprs_rx_temp, 0, UART3_RX_SIZE);
    /*获取一帧数据*/
    gprs_rx_len = get_line_from_uart3(gprs_rx_temp);
    if (gprs_rx_len) {
        ibox_printf(ibox_wifi_debug, ("[GPRS_REC]:%s\r\n", gprs_rx_temp));
    } else {
//        return;
    }

    switch (gprs_status) {
    case GPRS_STATUS_INIT: //检测模块；ATE0关闭回显
        uart3_rx_buf_clear();
        uart3_send_str("AT\r\n");
        gprs_status_error_count = 0;
        gprs_timeout            = get_sys_time_s();
        gprs_status = GPRS_STATUS_INIT;
        break;
//    case ESP8266_STATUS_CHECK_MODE: //检测模块模式
//        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
//            wifi_status_error_count = 0;
//            wifi_timeout            = get_sys_time_s();
//            //ibox_printf(ibox_wifi_debug, ("AT+CWMODE?\r\n"));
//            uart3_send_str("AT+CWMODE?\r\n"); 
//            gprs_status = ESP8266_STATUS_WAIT_CHECK_MODE;
//        } else {
//            if (get_sys_time_s() - wifi_timeout > 2) {
//                wifi_timeout = get_sys_time_s();
//                wifi_status_error_count++;
//                if (wifi_status_error_count > 5) {
//                    wifi_link_error_count++;
//                } else {
//                    uart3_send_str("AT\r\n");
//                }
//            }
//            esp8266_status = ESP8266_STATUS_CHECK_MODE;
//        }
        break;
    default:
        break;
    }
}