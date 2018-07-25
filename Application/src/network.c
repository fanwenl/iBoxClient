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
#include <stdbool.h>

uint16_t net_tx_len = 0; //网络发送的数据长度
uint16_t net_rx_len = 0;
uint8_t net_tx_buf[NET_TX_BUF_SIZE];   
uint8_t net_rx_buf[NET_RX_BUF_SIZE];
char net_buf[2048];
net_fifo_t net_rx_fifo;
//uint8_t net_rx_bottom_buf[NET_RX_BUF_SIZE];

/*创建net相关的信号量*/
static rt_sem_t net_rx_sem = RT_NULL;
static rt_sem_t net_tx_sem = RT_NULL;
static rt_mutex_t net_fifo_mutex = RT_NULL;

/*创建network的事件*/
rt_event_t network_thread_event = RT_NULL;

extern uint8_t DHCP_allocated_ip[];
extern uint16_t wifi_tx_len;
extern uint8_t uart3_tx_buf[];
uint32_t temp_timeout = 0;

static void fifo_init(net_fifo_t *fifo, char *buffer, uint16_t size );
static uint16_t fifo_next( net_fifo_t *fifo, uint16_t index );
static void fifo_push( net_fifo_t *fifo, char data );
static unsigned char fifo_pop( net_fifo_t *fifo );
static void fifo_flush(net_fifo_t *fifo );
static bool is_fifo_full(net_fifo_t *fifo );
static bool is_fifo_empty(net_fifo_t *fifo );
void push_data_to_net_fifo(void);


void network_thread_entry(void *parameter)
{
    rt_uint32_t opt = 0;

    /*初始化fifo*/
    fifo_init(&net_rx_fifo, net_buf, 2048);

    while (1) {
        if(rt_event_recv(network_thread_event, NETWORK_THREAD_EVENT_ALL, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,RT_TICK_PER_SECOND * 10, &opt) == RT_EOK)
        {
            if(opt & NET_RX_BUF_WRITE_EVENT)
            {
                push_data_to_net_fifo();
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
        ibox_printf(ibox_net_debug, ("network event create fail!\r\n"));
    }
    net_fifo_mutex = rt_mutex_create("NetFifoMutex", RT_IPC_FLAG_FIFO);
    if(net_fifo_mutex == RT_NULL)
    {
        ibox_printf(ibox_net_debug,("net fifo mutex creat fail!\r\n"));
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
/*将解析到的数据从net_rx_buf中copy到net_buf中*/
void push_data_to_net_fifo(void)
{
    uint16_t len = net_rx_len;

    if (rt_mutex_take(net_fifo_mutex, RT_TICK_PER_SECOND * 5) == RT_EOK)
    {
        do
        {
            fifo_push(&net_rx_fifo, net_rx_buf[net_rx_len - len]);
            len--;
            if (is_fifo_full(&net_rx_fifo))
            {
                ibox_printf(ibox_net_debug, ("net fifo is full!\r\n"));
                break;
            }
        } while (len > 0);
        rt_mutex_release(net_fifo_mutex);
    }
    else
    {
        ibox_printf(ibox_net_debug, ("net fifo write timeout!\r\n"));
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
/**
 * 从net buf中读取一个字节数据
 * *buf[]读取的数据buf
 * count要读取的数据个数
 * @ret 返回实际读取的数量
 */ 
int net_fifo_read(unsigned char* buf, int count)
{
    int rec_count = 0;
    if (is_fifo_empty(&net_rx_fifo))
    {
        return 0;
    }
    if (rt_mutex_take(net_fifo_mutex, RT_TICK_PER_SECOND * 5) == RT_EOK)
    {
        while (count > 0)
        {
            buf[rec_count++] = fifo_pop(&net_rx_fifo);
            count--;
            if (is_fifo_empty(&net_rx_fifo))
            {
                ibox_printf(ibox_net_debug, ("net fifo is empty!\r\n"));
                break;
            }
        }
        rt_mutex_release(net_fifo_mutex);
    }
    else
    {
        ibox_printf(ibox_net_debug, ("net fifo read timeout!\r\n"));
    }
    return rec_count;
}
void net_rx_sem_release(void)
{
    rt_sem_release(net_rx_sem);
}


void net_send_data(uint8_t *buf, uint16_t len) {}


static uint16_t fifo_next( net_fifo_t *fifo, uint16_t index )
{
	return ( index + 1 ) % fifo->Size;
}

static void fifo_init(net_fifo_t *fifo, char *buffer, uint16_t size )
{
	fifo->Begin = 0;
	fifo->End = 0;
	fifo->Data = buffer;
	fifo->Size = size;
}


static void fifo_push( net_fifo_t *fifo, char data )
{
	fifo->End = fifo_next( fifo, fifo->End );
	fifo->Data[fifo->End] = data;
}

static unsigned char fifo_pop( net_fifo_t *fifo )
{
	uint16_t data = fifo->Data[fifo_next( fifo, fifo->Begin )];

	fifo->Begin = fifo_next( fifo, fifo->Begin );
	return data;
}

static void fifo_flush(net_fifo_t *fifo )
{
	fifo->Begin = 0;
	fifo->End = 0;
}

static bool is_fifo_empty(net_fifo_t *fifo )
{
	return ( fifo->Begin == fifo->End );
}

static bool is_fifo_full(net_fifo_t *fifo )
{
	return ( fifo_next( fifo, fifo->End ) == fifo->Begin );
}
