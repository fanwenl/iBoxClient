/*
**************************************************************************************************
*文件：wired_net.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-07-18
*描述：ibox ETH有线网络任务文件。
* ************************************************************************************************
*/
#include "ibox_board.h"
void wired_thread_entry(void *parameter)
{
    while(1)
    {
//                 //     DHCP_FAILED = 0,  ///< Procssing Fail
//         //   DHCP_RUNNING,     ///< Procssing DHCP proctocol
//         //   DHCP_IP_ASSIGN,   ///< First Occupy IP from DHPC server      (if cbfunc == null, act as default
//         //   default_ip_assign) DHCP_IP_CHANGED,  ///< Change IP address by new ip from DHCP (if cbfunc ==
//         //   null, act as default default_ip_update) DHCP_IP_LEASED,   ///< Stand by DHCP_STOPPED
// //         dhcp_status = DHCP_run();
//         if (dhcp_status == DHCP_IP_ASSIGN) {
//             printf("dhcp is assign\r\n");
//             printf("ip:%d.%d.%d.%d\r\n", DHCP_allocated_ip[0], DHCP_allocated_ip[1], DHCP_allocated_ip[2],
//                    DHCP_allocated_ip[3]);
//         } else if (dhcp_status == DHCP_RUNNING) {
//             printf("dhcp is runing\r\n");
//         } else if (dhcp_status == DHCP_IP_LEASED) {
//             printf("dhcp is leased\r\n");
//         }
//         /*DNS解析服务需要在DHCP之后运行吗？*/
// //        dns_status = DNS_run(ibox_config.dns_ip, ibox_config.server_dsn, eth_msg_get.dns_sip);
//         if(dns_status == -1)
//         {
//             printf("dns Domain name is too small!\r\n");
//         }
//         else if(dns_status == 0)
//         {
//             printf("DNS Timeout or Parse error!\r\n");
//         }
//         else if(dns_status == 1)
//         {
//             printf("DNS success!\r\n");
//             printf("s_ip:%d.%d.%d.%d\r\n",eth_msg_get.dns_sip[0],eth_msg_get.dns_sip[1],eth_msg_get.dns_sip[2],eth_msg_get.dns_sip[3]);
//         }
        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
    
}
