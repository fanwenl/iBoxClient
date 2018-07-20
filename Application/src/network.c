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
uint16_t net_rx_len = 0;
uint8_t net_tx_buf[NET_TX_BUF_SIZE];   
uint8_t net_rx_buf[NET_RX_BUF_SIZE];
uint8_t net_rx_bottom_buf[NET_RX_BUF_SIZE];

/*创建net相关的信号量*/
static rt_sem_t net_rx_sem = RT_NULL;
static rt_sem_t net_tx_sem = RT_NULL;

/*创建network的事件*/
rt_event_t network_thread_event = RT_NULL;

extern uint8_t DHCP_allocated_ip[];
extern uint16_t wifi_tx_len;
extern uint8_t uart3_tx_buf[];
uint32_t temp_timeout = 0;

void network_thread_entry(void *parameter)
{
    rt_uint32_t opt = 0;

    while (1) {
        if(rt_event_recv(network_thread_event, NETWORK_THREAD_EVENT_ALL, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,RT_TICK_PER_SECOND * 10, &opt) == RT_EOK)
        {
            if(opt & NET_RX_BUF_WRITE_EVENT)
            {
                rt_event_send(main_thread_event, MAIN_RECEIVE_DATA_EVENT);
            }
            if(opt & NET_TX_BUF_WRITE_EVENT)
            {
                /*判断网络的连接状态，选择通道发送（有线或者是无线）*/

            }
            
        }

        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
}

void network_thread_init(void)
{
    net_rx_sem = rt_sem_create("NetRxS", 1, RT_IPC_FLAG_FIFO);
    if(net_rx_sem == RT_NULL)
    {
        ibox_printf(ibox_net_debug, ("net rx sem create fail!\r\n"));
    }
    net_tx_sem = rt_sem_create("NetTxS", 1, RT_IPC_FLAG_FIFO);
    if(net_tx_sem == RT_NULL)
    {
        ibox_printf(ibox_net_debug, ("net tx sem create fail!\r\n"));
    }

    network_thread_event = rt_event_create("NetworkEvent", RT_IPC_FLAG_FIFO);
    if(network_thread_event == RT_NULL)
    {
        ibox_printf(1, ("network event create fail!\r\n"));
    }
}
uint8_t net_tx_write(void *prt, uint16_t len)
{
    IBOX_ASSERT(prt == NULL);

    if(rt_sem_take(net_tx_sem, RT_TICK_PER_SECOND) != RT_EOK)
    {
        net_tx_len = len;
        memset(net_tx_buf, 0, sizeof(net_tx_buf));
        memcpy(net_tx_buf, prt, len);
        ibox_printf(ibox_net_debug, ("net tx write!\r\n"));
        rt_event_send(network_thread_event, NET_TX_BUF_WRITE_EVENT);
        return 1;
    }
    else{
        ibox_printf(ibox_net_debug, ("net tx write tiemout!\r\n"));
        return 0;
    }
}
void net_tx_sem_release(void)
{
    rt_sem_release(net_tx_sem);
}
uint8_t net_rx_write(void *prt, uint16_t len)
{
    IBOX_ASSERT(prt == NULL);

    if(rt_sem_take(net_rx_sem, RT_TICK_PER_SECOND) != RT_EOK)
    {
        net_rx_len = len;
        memset(net_rx_buf, 0, sizeof(net_rx_buf));
        memcpy(net_rx_buf, prt, len);
        ibox_printf(ibox_net_debug, ("net rx write!\r\n"));
        rt_event_send(network_thread_event, NET_RX_BUF_WRITE_EVENT);
        return 1;
    }
    else{
        ibox_printf(ibox_net_debug, ("net rx write tiemout!\r\n"));
        return 0;
    }
}
void net_rx_read(void *prt, uint16_t *len)
{
    IBOX_ASSERT(prt == NULL);
    IBOX_ASSERT(len == NULL);

    *len = net_rx_len;
    memcpy(prt, net_rx_buf, net_rx_len);
    ibox_printf(ibox_net_debug, ("net rx buf read!\r\n"));
    rt_sem_release(net_rx_sem);
}
void net_rx_sem_release(void)
{
    rt_sem_release(net_rx_sem);
}


void net_send_data(uint8_t *buf, uint16_t len) {}
