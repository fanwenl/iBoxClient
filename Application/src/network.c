/*
**************************************************************************************************
*文件：network.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-06-06
*描述：ibox network中间件,对底层的网络驱动进行一定的封装(wifi、gms、eth)。
* ************************************************************************************************
*/
#include "network.h"
#include "ibox_board.h"
#include <stdio.h>
#include <string.h>

uint16_t net_tx_len = 0; //网络发送的数据长度
extern uint8_t DHCP_allocated_ip[];
extern uint16_t wifi_tx_len;
extern uint8_t uart3_tx_buf[];
uint32_t temp_timeout = 0;
void network_thread_entry(void *parameter)
{
    uint8_t dhcp_status = 0;
    int8_t dns_status = 10;
    static uint8_t temp_buf[128] = "hello esp8266!\r\n";
    uint16_t temp_len=0;

    while (1) {
        //     DHCP_FAILED = 0,  ///< Procssing Fail
        //   DHCP_RUNNING,     ///< Procssing DHCP proctocol
        //   DHCP_IP_ASSIGN,   ///< First Occupy IP from DHPC server      (if cbfunc == null, act as default
        //   default_ip_assign) DHCP_IP_CHANGED,  ///< Change IP address by new ip from DHCP (if cbfunc ==
        //   null, act as default default_ip_update) DHCP_IP_LEASED,   ///< Stand by DHCP_STOPPED
//         dhcp_status = DHCP_run();
        if (dhcp_status == DHCP_IP_ASSIGN) {
            printf("dhcp is assign\r\n");
            printf("ip:%d.%d.%d.%d\r\n", DHCP_allocated_ip[0], DHCP_allocated_ip[1], DHCP_allocated_ip[2],
                   DHCP_allocated_ip[3]);
        } else if (dhcp_status == DHCP_RUNNING) {
            printf("dhcp is runing\r\n");
        } else if (dhcp_status == DHCP_IP_LEASED) {
            printf("dhcp is leased\r\n");
        }
        /*DNS解析服务需要在DHCP之后运行吗？*/
//        dns_status = DNS_run(ibox_config.dns_ip, ibox_config.server_dsn, eth_msg_get.dns_sip);
        if(dns_status == -1)
        {
            printf("dns Domain name is too small!\r\n");
        }
        else if(dns_status == 0)
        {
            printf("DNS Timeout or Parse error!\r\n");
        }
        else if(dns_status == 1)
        {
            printf("DNS success!\r\n");
            printf("s_ip:%d.%d.%d.%d\r\n",eth_msg_get.dns_sip[0],eth_msg_get.dns_sip[1],eth_msg_get.dns_sip[2],eth_msg_get.dns_sip[3]);
        }
   #ifdef USE_WIFI
        esp8266_at_fsm();
        if(get_esp8266_status() == ESP8266_STATUS_COMMUNICATE)
        {
            

            if(get_sys_time_s() - temp_timeout> 30)
            {
                temp_timeout = get_sys_time_s();
                printf("wifi tx time:%d\r\n",temp_timeout);
                memcpy(uart3_tx_buf,temp_buf,20);
                wifi_tx_len = 20;
            }
            
        }
   #else
        gprs_at_fsm();
   #endif
        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
}

void net_send_data(uint8_t *buf, uint16_t len) {}
