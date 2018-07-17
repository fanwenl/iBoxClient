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
#include "ibox_board.h"
#include "ibox_uart.h"
#include "network.h"
#include <string.h>

GPRS_STATUS_ENUM gprs_status = GPRS_STATUS_INIT;

uint8_t gprs_status_error_count = 0; // GPRS状态机错误计数器
uint8_t gprs_link_error_count   = 0; // GPRS连接错误计数器
uint8_t gprs_error_count        = 0; // GPRS错误计数器
uint32_t gprs_timeout           = 0; // GPRS连接超时
uint32_t gprs_tcp_error_count   = 0; // GPRS TCP错误计数器

uint8_t gprs_send_flag         = 0; // GPRS发送数据标志位
uint8_t gprs_send_timeout_flag = 0; // GPRS发送超时标志位
uint16_t gprs_tx_counter       = 0; // GPRS发送数据长度
uint16_t gprs_rx_counter       = 0; // GPRS介绍数据长度

char gprs_send_str[GPRS_SEND_STR_SIZE];

static void gprs_pin_config(void);
static void gprs_server_connect(void);
static void gprs_send_data(void);
/**
 * GPRS 初始化函数
 */
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
/**
 * GPRS状态机，GPRS配置为主动接收，单连接，HEX模式
 */
void gprs_at_fsm(void)
{
    static uint8_t gprs_rx_temp[UART3_RX_SIZE];
    uint16_t gprs_rx_len   = 0;
    char *p_strstr         = NULL; // strstr()函数返回值
    uint16_t gprs_sq       = 0;    //信号强度
    uint16_t gprs_countnum = 0;

    uint16_t count = 0;
    uint8_t  atoi_buf[3]={0,0,0};

    memset(gprs_rx_temp, 0, UART3_RX_SIZE);
    /*获取一帧数据*/
    gprs_rx_len = get_line_from_uart3(gprs_rx_temp);
    if (gprs_rx_len) {
        ibox_printf(ibox_gprs_debug, ("[GPRS_REC]:%s\r\n", gprs_rx_temp));
    }

    switch (gprs_status) {
    case GPRS_STATUS_CHECK: //检测模块；ATE0关闭回显
        uart3_rx_buf_clear();
        uart3_send_str("AT\r\n");
        gprs_status_error_count = 0;
        gprs_timeout            = get_sys_time_s();
        gprs_status             = GPRS_STATUS_INIT;
        break;
    case GPRS_STATUS_INIT: //检查SIM卡
        if (strstr((char *) gprs_rx_temp, "OK") != NULL) {
            gprs_status_error_count = 0;
            gprs_timeout            = get_sys_time_s();
            ibox_printf(ibox_gprs_debug, ("AT+CPIN?\r\n"));
            uart3_send_str("AT+CPIN?\r\n");
            gprs_status = GPRS_STATUS_SET_MISC;
        } else {
            if (get_sys_time_s() - gprs_timeout > 2) {
                gprs_timeout = get_sys_time_s();
                gprs_status_error_count++;
                if (gprs_status_error_count > 5) {
                    gprs_link_error_count++;
                } else {
                    uart3_send_str("AT\r\n");
                }
            }
        }
        break;
    case GPRS_STATUS_SET_MISC: //设置杂项
        if (strstr((char *) gprs_rx_temp, "READY") != NULL) {
            uart3_send_str("AT+CIPSHUT\r\n"); //关闭连接
            sys_delay_ms(1000);
            uart3_send_str("AT+CIPRXGET=1\r\n"); //接受数据模式改为主动接收
            sys_delay_ms(200);
            uart3_send_str("AT+CIPSPRT=0\r\n"); //删掉<号
            sys_delay_ms(200);
            uart3_rx_buf_clear();
            uart3_send_str("AT+CSQ\r\n");
            ibox_printf(ibox_gprs_debug, ("AT+CSQ\r\n"));
            gprs_status             = GPRS_STATUS_CHECK_CSQ;
            gprs_timeout            = get_sys_time_s();
            gprs_status_error_count = 0;
        } else {
            if (get_sys_time_s() - gprs_timeout > 2) {
                gprs_timeout = get_sys_time_s();
                gprs_status_error_count++;
                if (gprs_status_error_count > 5) {
                    gprs_link_error_count++;
                } else {
                    uart3_send_str("AT+CPIN?\r\n");
                }
            }
        }
        break;
    case GPRS_STATUS_CHECK_CSQ: //检测信号强度
        p_strstr = NULL;
        p_strstr = strstr((char *) gprs_rx_temp, "+CSQ:");
        if (p_strstr != NULL) {
            if (p_strstr[7] != ',') {
                gprs_sq = (p_strstr[6] - 0x30) * 10 + (p_strstr[7] - 0x30);
            } else {
                gprs_sq = (p_strstr[6] - 0x30);
            }
            if ((gprs_sq >= 10) && (gprs_sq <= 31)) {
                ibox_printf(ibox_gprs_debug, ("GPRS CSQ:%d", gprs_sq));
                uart3_send_str("AT+CREG?\r\n"); //检测当前注册状态
                ibox_printf(ibox_gprs_debug, ("AT Req -> AT+CREG?"));
                gprs_status             = GPRS_STATUS_CHECK_REG;
                gprs_timeout            = get_sys_time_s();
                gprs_status_error_count = 0;
            }
        } else {
            if (get_sys_time_s() - gprs_timeout > 2) {
                gprs_timeout = get_sys_time_s();
                gprs_status_error_count++;
                if (gprs_status_error_count > 5) {
                    gprs_link_error_count++;
                } else {
                    uart3_send_str("AT+CSQ\r\n");
                }
            }
        }
        break;
    case GPRS_STATUS_CHECK_REG:
        p_strstr = strstr((char *) gprs_rx_temp, "+CREG:");
        if (p_strstr != NULL) {
            if ((p_strstr[9] == '1') || (p_strstr[9] == '5')) //注册成功
            {
                ibox_printf(ibox_gprs_debug, ("CREG OK"));
                gprs_timeout            = get_sys_time_s();
                gprs_status_error_count = 0;
                // gprs附着
                // CLEAR_GSM_RECV_BUF();
                uart3_send_str("AT+CGATT?\r\n");
                ibox_printf(ibox_gprs_debug, ("AT Req -> AT+CGATT?"));
                gprs_status = GPRS_STATUS_CHECK_CGATT;
            }
        } else {
            if (get_sys_time_s() - gprs_timeout > 2) {
                gprs_timeout = get_sys_time_s();
                gprs_status_error_count++;
                if (gprs_status_error_count > 5) {
                    gprs_link_error_count++;
                } else {
                    uart3_send_str("AT+CREG?\r\n");
                }
            }
        }
        break;
    case GPRS_STATUS_CHECK_CGATT: //检测GPS附着
        if (strstr((char *) gprs_rx_temp, "+CGATT: 1") != NULL) {
            ibox_printf(ibox_gprs_debug, ("CGATT OK"));
            gprs_status_error_count = 0;
            memset(gprs_send_str, 0, GPRS_SEND_STR_SIZE);
            sprintf((char *) gprs_send_str, "AT+CSTT=\"%s\"\r\n", (char *) ibox_config.gsm_apn);
            uart3_send_str(gprs_send_str);
            gprs_status  = GPRS_STATUS_SET_APN;
            gprs_timeout = get_sys_time_s();
        } else {
            if (get_sys_time_s() - gprs_timeout > 2) {
                gprs_timeout = get_sys_time_s();
                gprs_status_error_count++;
                if (gprs_status_error_count > 5) {
                    gprs_link_error_count++;
                } else {
                    uart3_send_str("AT+CGATT?\r\n");
                }
            }
        }
        break;
    case GPRS_STATUS_SET_APN: // set apn
        if (strstr((char *) gprs_rx_temp, "OK") != NULL) {
            gprs_status_error_count = 0;
            gprs_timeout            = get_sys_time_s();
            // CLEAR_GSM_RECV_BUF();
            uart3_send_str("AT+CIICR\r\n");
            ibox_printf(ibox_gprs_debug, ("AT Req -> AT+CIICR"));
            gprs_status = GPRS_STATUS_SET_CIICR;

        } else if (strstr((char *) gprs_rx_temp, "ERROR") != NULL) {
            /*已经有错误状态的，不用再重发了*/
            gprs_status_error_count = 0;
            gprs_timeout            = get_sys_time_s();
            ibox_printf(ibox_gprs_debug, ("GPRS set APN ERROR!\r\n"));
            gprs_status = GPRS_STATUS_CHECK;
        } else {
        }
        break;
    case GPRS_STATUS_SET_CIICR: //建立无限链路
        if (strstr((char *) gprs_rx_temp, "OK") != NULL) {
            gprs_status_error_count = 0;
            gprs_timeout            = get_sys_time_s();
            // CLEAR_GSM_RECV_BUF();
            uart3_send_str("AT+CIFSR\r\n");
            ibox_printf(ibox_gprs_debug, ("AT Req -> AT+CIFSR"));
            gprs_status = GPRS_STATUS_GET_IP;
        } else if (strstr((char *) gprs_rx_temp, "ERROR") != NULL) {
            gprs_timeout = get_sys_time_s();
            gprs_status  = GPRS_STATUS_CHECK;
        } else {
        }
        break;
    case GPRS_STATUS_GET_IP: //获取IP地址
        if (strstr((char *) gprs_rx_temp, ".") != NULL) {
            ibox_printf(ibox_gprs_debug, ("Get Local IP OK"));
            gprs_status = GPRS_STATUS_START_TCP;
        } else if (strstr((char *) gprs_rx_temp, "ERROR") != NULL) {
            gprs_timeout = get_sys_time_s();
            gprs_status  = GPRS_STATUS_INIT;
        } else {
        }
        break;
    case GPRS_STATUS_START_TCP: //锚点
        // CLEAR_GSM_RECV_BUF();
        gprs_server_connect();
        gprs_timeout = get_sys_time_s();
        gprs_status  = GPRS_STATUS_TCP_CONNECT;
        break;
    case GPRS_STATUS_RESTART_TCP:
        ibox_printf(ibox_gprs_debug, ("TCP ERROR,restart tcp link"));
        gprs_tcp_error_count++;
        uart3_send_str("AT+CIPCLOSE\r\n"); //关闭TCP连接
        sys_delay_ms(1000);
        // CLEAR_GSM_RECV_BUF();
        gprs_status            = GPRS_STATUS_START_TCP;
        gprs_tx_counter        = 0;
        gprs_send_flag         = 0;
        gprs_send_timeout_flag = 0;
        break;
    case GPRS_STATUS_TCP_CONNECT: // tcp连接
        if (strstr((char *) gprs_rx_temp, "CONNECT OK") != NULL) {
            //reg_error_count = 0;
            gprs_timeout    = get_sys_time_s();
          ibox_printf(ibox_gprs_debug, ("SERVER CONNECT OK"));                
          gprs_status = GPRS_STATUS_COMMUNICATE;
          //uart3_send_str("AT+CDNSGIP=lot.zxbike.cc\r\n");
          //rt_event_send(&net_thread_event,NET_THREAD_EVENT_GPRS_CONNECT);
          ibox_printf(ibox_gprs_debug, ("send verify message"));
          gprs_send_flag = 1;
        }
        //已经连接上了
        else if (strstr((char *) gprs_rx_temp, "ALREADY CONNECT") != NULL) {
            gprs_timeout = get_sys_time_s();
            gprs_tcp_error_count++;
            ibox_printf(ibox_gprs_debug, ("SERVER ALREADY CONNECT, RECONNECT..."));
            gprs_status = GPRS_STATUS_INIT;
        }
        //连接失败
        else if (strstr((char *) gprs_rx_temp, "CONNECT FAIL") != NULL) {
            gprs_timeout = get_sys_time_s();
            gprs_tcp_error_count++;
            ibox_printf(ibox_gprs_debug, ("SERVER CONNECT FAIL"));
            gprs_status = GPRS_STATUS_INIT;
        }
        if (get_sys_time_s() - gprs_timeout > RT_TICK_PER_SECOND * 10)
            {
                gprs_tcp_error_count++;
                ibox_printf(ibox_gprs_debug, ("SERVER CONNECT timeout"));
                gprs_status = GPRS_STATUS_INIT;
            }
        break;
    case GPRS_STATUS_COMMUNICATE: // TCP正常通信的状态
        if (strstr((char *) gprs_rx_temp, "CLOSED") != NULL) {
            gprs_status = GPRS_STATUS_RESTART_TCP;
            break;
        }
        //失去上下文
        else if (strstr((char *) gprs_rx_temp, "+PDP DEACT") != NULL) {
            gprs_timeout = get_sys_time_s();
            ibox_printf(ibox_gprs_debug, ("GPRS context FaIL, Reactting..."));
            gprs_status = GPRS_STATUS_INIT;
            break;
        }
        //获取数据
        else if (strstr((char *) gprs_rx_temp, "+CIPRXGET: 1") != NULL) {
            //只有在正常获取数据之后tcp error才会清零
            gprs_tcp_error_count = 0;
            //数据处理
            gprs_status  = GPRS_STATUS_RX_PROC;
            gprs_timeout = get_sys_time_s();
            uart3_send_str("AT+CIPRXGET=3,730\r\n"); //设置hex数据格式，最长730
            break;
        }
        //发送成功
        else if (strstr((char *) gprs_rx_temp, "SEND OK") != NULL) {
            gprs_send_flag         = 1;
            gprs_send_timeout_flag = 0;
            gprs_tx_counter        = 0;
            // rt_event_send(&net_thread_event, NET_THREAD_EVENT_GPRS_SEND_OK);
            break;
        }
        //发送失败
        else if (strstr((char *) gprs_rx_temp, "SEND FAIL") != NULL) {
            gprs_timeout = get_sys_time_s();
            ibox_printf(ibox_gprs_debug, ("Send Fail"));
            gprs_status            = GPRS_STATUS_INIT;
            gprs_send_flag         = 1;
            gprs_send_timeout_flag = 0;
            break;
        }
        //出错了
        else if (strstr((char *) gprs_rx_temp, "ERROR") != NULL) // ERROR
        {
            gprs_status = GPRS_STATUS_RESTART_TCP;
            break;
        }
        //发送数据
        if (gprs_tx_counter && gprs_send_flag) {

            gprs_timeout = get_sys_time_s();
            ibox_printf(ibox_gprs_debug, ("Sending user data..."));
            gprs_countnum = 0;
            while (gprs_countnum != gprs_tx_counter) {
                ibox_printf(ibox_gprs_debug, ("<%02X>", uart3_tx_buf[gprs_countnum]));
                gprs_countnum++;
            }
            ibox_printf(ibox_gprs_debug, ("\r\n"));
            gprs_send_data();
            gprs_send_flag         = 0;
            gprs_send_timeout_flag = 1;

            break;
        }
        if ((get_sys_time_s() - gprs_timeout > RT_TICK_PER_SECOND * 10) && gprs_send_timeout_flag) {
        ibox_printf(ibox_gprs_debug, ("send time out"));
        gprs_status = GPRS_STATUS_RESTART_TCP;
        break;
        }
        break;
    case GPRS_STATUS_RX_PROC: //接收处理+CIPRXGET: 3,72,0
        p_strstr = NULL;
        p_strstr = strstr((char *) gprs_rx_temp, "+CIPRXGET: 3");
        if (p_strstr != NULL) {
            p_strstr += 13;
            gprs_rx_counter = 0;
            while (*p_strstr != ',') {
                gprs_rx_counter = gprs_rx_counter * 10 + (*p_strstr - 0x30);
                p_strstr++;
            }
            if (gprs_rx_counter) {
                gprs_status = GPRS_STATUS_FRAME_PROC;
            } else {
                ibox_printf(ibox_gprs_debug, ("gprs recv error len , restart tcp!"));
                gprs_status = GPRS_STATUS_RESTART_TCP;
            }
            gprs_timeout = get_sys_time_s();
            break;
        }
        if (get_sys_time_s() - gprs_timeout > RT_TICK_PER_SECOND * 10) {
            ibox_printf(ibox_gprs_debug, ("CIPRXGET No Answer!"));
            gprs_status = GPRS_STATUS_RESTART_TCP;
        break;
        }
        break;
    case GPRS_STATUS_FRAME_PROC:
        if (gprs_rx_len) { //如果收到了数据
            if (gprs_rx_len != (2 * gprs_rx_counter)) {
                gprs_status = GPRS_STATUS_COMMUNICATE;
                ibox_printf(ibox_gprs_debug, ("gprs receive error num\r\n"));
                break;
            }
            for (count = 0; count < gprs_rx_counter; count++) {
                atoi_buf[0]     = gprs_rx_temp[count * 2];
                atoi_buf[1]     = gprs_rx_temp[count * 2 + 1];
                net_rx_bottom_buf[count] = strtol((const char *) atoi_buf, NULL, 16);
            }
          ibox_printf(ibox_gprs_debug, ("\r\nnewdata is %d\r\n", gprs_rx_counter));
          gprs_status = GPRS_STATUS_COMMUNICATE;
          //net_rx_buf_write(rx_buff,gprs_rx_counter);
        } else { //需要延迟一段时间
            if (get_sys_time_s() - gprs_timeout > RT_TICK_PER_SECOND * 2) {
                gprs_status = GPRS_STATUS_COMMUNICATE;
                ibox_printf(ibox_gprs_debug, ("gprs receive error num\r\n"));
            }
        }
        break;
    default:
        break;
    }
}
/**
 * GPRS 发送数据接口函数
 */
static void gprs_send_data(void)
{
    memset(gprs_send_str, 0, GPRS_SEND_STR_SIZE);
    sprintf((char *) gprs_send_str, "AT+CIPSEND=%d\r\n", gprs_tx_counter);
    uart3_send_str(gprs_send_str);
    sys_delay_ms(50);
    uart3_send_data(uart3_tx_buf, gprs_tx_counter);
}

/**
 * 读取GPRS模块的IMEI编号
 * 保存在ibox_config结构体中,只在上电的时候读取一次,所以没有放到状态机中。
 */
void gprs_get_imei(void)
{
    uint8_t imei_buf[20], i = 0;

    sys_delay_ms(3000);
    uart3_send_str("ATE0\r\n");
    sys_delay_ms(1000);
    uart3_rx_buf_clear();
    uart3_send_str("AT+GSN\r\n");
    sys_delay_ms(1000);
    do{
        memset(imei_buf, 0, 20);
        i = get_line_from_uart3(imei_buf);
        if ( i== 15) {
            ibox_config.gprs_imei = atoll((const char *) imei_buf);
            break;
        }
    }while(i);

    uart3_rx_buf_clear();
}

/**
 * GPRS 连接服务器
 */
static void gprs_server_connect(void)
{
    char temp_buf[6];
    uart3_send_str("AT+CIPSTART=\"TCP\",\"");

    /* tcp错误计数超了 选用原有域名重新连接服务器.或者是默认使用IP连接服务器*/
    if ((ibox_config.use_dns == 0) || (gprs_tcp_error_count >= 3)) {
        ibox_printf(ibox_gprs_debug, ("tcp connect err out range || default use the ip \"%s\" port \"%d\"connect\r\n",
                                       ibox_config.server_ip, ibox_config.server_port));
        uart3_send_str(ibox_config.server_ip);
        uart3_send_str("\",\"");
    } else {
        ibox_printf(ibox_gprs_debug, ("tcp connect use the DNS \"%s\" port \"%d\"connect\r\n", ibox_config.server_dsn,
                                       ibox_config.server_port));
        uart3_send_str(ibox_config.server_dsn);
        uart3_send_str("\",\"");
    }
    memset((uint8_t *)temp_buf, 0, 6);
    sprintf(temp_buf, "%d", ibox_config.server_port);
    uart3_send_str(temp_buf);
    uart3_send_str("\"\r\n");
}
