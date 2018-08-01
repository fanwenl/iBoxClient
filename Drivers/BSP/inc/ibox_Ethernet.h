/*
**************************************************************************************************
*文件：iBox_Ethernet.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-28
*描述：iBox iot 以太网驱动文件。
* ************************************************************************************************
*/
#ifndef __IBOX_ETHERNET_H
#define __IBOX_ETHERNET_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"

typedef struct{
    uint8_t mac[6];             //本地以太网MAC
    uint8_t ip[4];              //本地以太网IP
    uint8_t sub[4];             //本地以太网掩码
    uint8_t gw[4];              //本地以太网网关
    uint8_t dns[4];             //本地以太网DNS

    uint8_t dns_sip[4];         //DNS解析到的server的ip
}ETH_MSG;

extern ETH_MSG eth_msg_get;

void w5500_hw_init(void);
void ethernet_init(void);
uint8_t ethernet_run(void);

#ifdef __cplusplus
    }
#endif /*__cplusplus*/

#endif /*__IBOX_ETHERNET_H*/
