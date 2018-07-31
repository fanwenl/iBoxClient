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

uint8_t is_wired_link_ok = 0;

void wired_thread_entry(void *parameter)
{
    uint8_t dhcp_status =0;
    uint8_t dns_status = 0;
    static uint8_t dsn_ip[4];

    sscanf(ibox_config.dns_ip, "%d:%d:%d:%d", &dsn_ip[0], &dsn_ip[1], &dsn_ip[2], &dsn_ip[3]);

    /*Eth PHY 有网线接入*/
    if (wizphy_getphylink() == PHY_LINK_ON)
    while(1)
    {
        /*DHCP 获取*/
        DHCP_run();

        /*DNS解析服务需要在DHCP之后运行吗？*/
        dns_status = DNS_run(dsn_ip, ibox_config.server_dsn, eth_msg_get.dns_sip);
        if(dns_status == 0)
        {
            printf("DNS Timeout or Parse error!\r\n");
        }
        else if(dns_status == 1)
        {
            ibox_printf(ibox_eth_debug, ("DNS success s_ip:%d.%d.%d.%d\r\n",eth_msg_get.dns_sip[0],eth_msg_get.dns_sip[1],eth_msg_get.dns_sip[2],eth_msg_get.dns_sip[3]));
        }

        rt_thread_delay(RT_TICK_PER_SECOND / 20);
    }
    
}
