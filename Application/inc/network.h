/*
**************************************************************************************************
*文件：network.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-06-06
*描述：ibox network中间件,对底层的网络驱动进行一定的封装(wifi、gms、eth)。
* ************************************************************************************************
*/
#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdint.h>
#include <stdio.h>
#include <rtthread.h>
#include <stdbool.h>

#define NET_TX_BUF_SIZE 1460
#define NET_RX_BUF_SIZE 1460

#define NET_RX_BUF_WRITE_EVENT              (1u << 0)
#define NET_TX_BUF_WRITE_EVENT              (1u << 1)

#define NETWORK_THREAD_EVENT_ALL            NET_RX_BUF_WRITE_EVENT|\
                                            NET_TX_BUF_WRITE_EVENT

typedef struct _net_fifo
{
	uint16_t Begin;
	uint16_t End;
	unsigned char *Data;
    uint16_t Size;
}net_fifo_t;

extern net_fifo_t net_rx_fifo;
extern uint8_t net_rx_bottom_buf[];
extern rt_event_t network_thread_event;


void network_thread_init(void);
uint8_t net_tx_write(void *prt, uint16_t len);
void net_tx_sem_release(void);
uint8_t net_rx_write(void *prt, uint16_t len);
void net_rx_read(void *prt, uint16_t *len);
void net_rx_sem_release(void);
int net_fifo_read(unsigned char* buf, int count);
bool is_fifo_empty(net_fifo_t *fifo );

#endif /*__NETWORK_H__*/
