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
#include <string.h>
#include <stdio.h>
#include "ibox_board.h"

uint16_t net_tx_len = 0; //网络发送的数据长度
extern uint8_t DHCP_allocated_ip[];
void network_thread_entry(void* parameter)
{
    uint8_t dhcp_status = 0;
    while (1)
    {         
//     DHCP_FAILED = 0,  ///< Procssing Fail
//   DHCP_RUNNING,     ///< Procssing DHCP proctocol
//   DHCP_IP_ASSIGN,   ///< First Occupy IP from DHPC server      (if cbfunc == null, act as default default_ip_assign)
//   DHCP_IP_CHANGED,  ///< Change IP address by new ip from DHCP (if cbfunc == null, act as default default_ip_update)
//   DHCP_IP_LEASED,   ///< Stand by 
//   DHCP_STOPPED 
//        dhcp_status = DHCP_run();
        if(dhcp_status == DHCP_IP_ASSIGN)
        {
            printf("dhcp is assign\r\n");
            printf("ip:%d.%d.%d.%d\r\n",DHCP_allocated_ip[0],DHCP_allocated_ip[1],DHCP_allocated_ip[2],DHCP_allocated_ip[3]);
        }
        else if(dhcp_status == DHCP_RUNNING)
        {
            printf("dhcp is runing\r\n");
        }
        else if(dhcp_status == DHCP_IP_LEASED)
        {
            printf("dhcp is leased\r\n");
        }
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}


void net_send_data(uint8_t *buf, uint16_t len)
{

}