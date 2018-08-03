/*
**************************************************************************************************
*文件：iBox_Ethernet.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-28
*描述：iBox iot Eth接口驱动。
* ************************************************************************************************
*/
#include "ibox_Ethernet.h"
#include "ibox_sys.h"
#include "ibox_spi.h"
#include "ibox_board.h"
#include "wizchip_conf.h"
#include "dhcp.h"
#include "dns.h"
#include "socket.h"

static uint8_t rx_socket_size[_WIZCHIP_SOCK_NUM_] = {2,2,2,2,2,2,2,2};
static uint8_t tx_socket_size[_WIZCHIP_SOCK_NUM_] = {2,2,2,2,2,2,2,2};
uint8_t dhcp_dns_buf[RIP_MSG_SIZE];

/*eth 网口发送数据长度*/
uint16_t eth_tx_len = 0;

ETH_MSG eth_msg_get;

void eth_ip_update(void);
void eth_ip_assign(void);
void eth_ip_conflict(void);

static void w5500_reset_pin_config(void)
{
    /*PD8,低电平有效(500us)*/
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_8;

    GPIO_Init(GPIOD, &GPIO_InitStruct);

    /*默认为输出高电平*/
    GPIO_SetBits(GPIOD, GPIO_Pin_8);
}
static void w5500_inth_pin_config(void)
{
    /*PD9,低电平有效，下降沿。*/
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_9;

    GPIO_Init(GPIOD, &GPIO_InitStruct);

    /*中断配置*/
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource9);

    //中断配置为下降沿触发
    EXTI_InitStructure.EXTI_Line    = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_ClearITPendingBit(EXTI_Line9);
    EXTI_Init(&EXTI_InitStructure);
}
void w55000_reset(void)
{
    GPIO_ResetBits(GPIOD, GPIO_Pin_8);
    sys_delay_us(10);
    GPIO_SetBits(GPIOD, GPIO_Pin_8);
    sys_delay_ms(1600);
}
void w5500_hw_init(void)
{
    w5500_reset_pin_config();
    w5500_inth_pin_config();
    spi_init(ETHERNET);
}
 void set_w5500_mac(void)
{   
    /*设备MAC地址出厂时设置保存,和设备号绑定*/
    sscanf(ibox_config.eth_mac, "%X:%X:%X:%X:%X:%X", (int *) &eth_msg_get.mac[0], (int *) &eth_msg_get.mac[1],
           (int *) &eth_msg_get.mac[2], (int *) &eth_msg_get.mac[3], (int *) &eth_msg_get.mac[4],
           (int *) &eth_msg_get.mac[5]);
    setSHAR(eth_msg_get.mac);
}
/**
 * 以太网相关的初始化
 * 包括DNS解析、HDCP动态获取...
 */ 
uint8_t buffer[2048];

void ethernet_init(void)
{
    set_w5500_mac();
    /*设置socket的大小*/
    wizchip_init(tx_socket_size, rx_socket_size);
    
    DHCP_init(CUS_DHCP_SOCKET, dhcp_dns_buf);
    
    /*DHCP 函数注册*/
    reg_dhcp_cbfunc(eth_ip_assign, eth_ip_update, eth_ip_conflict);

    /*dns 解析和dhcp共用一个buf*/
    DNS_init(CUS_DNS_SOCKET,dhcp_dns_buf);
}

uint8_t ethernet_run(void)
{
    static uint8_t sock_status;
    uint16_t arg = 0;
    uint16_t data_len = 0;
    uint8_t *data_ptr = NULL;

    /*获取socket0的状态*/
    if (getsockopt(CUS_COMM_SOCKET, SO_STATUS, &sock_status) != SOCK_OK)
    {
        return 0;
    }

    switch (sock_status)
    {
    case SOCK_INIT:
        /*设置SOCK keepalive 10s*/
        arg = 2;
        setsockopt(CUS_COMM_SOCKET, SO_KEEPALIVEAUTO, &arg);
        connect(CUS_COMM_SOCKET, eth_msg_get.dns_sip, ibox_config.server_port);
        break;
    case SOCK_ESTABLISHED: // Socket处于连接建立状态
        /*是否收到数据*/
        getsockopt(CUS_COMM_SOCKET, SO_RECVBUF, &arg);
        if (arg > 0)
        {
            data_ptr = rt_malloc(2048);
            data_len = recv(CUS_COMM_SOCKET, data_ptr, arg);
            if (data_len > 0)
            {
                net_rx_write(data_ptr, data_len);
            }
            rt_free(data_ptr);
        }
        /*发送数据*/
        if (eth_tx_len)
        {
            send(CUS_COMM_SOCKET, net_tx_buf, eth_tx_len);
            eth_tx_len = 0;
       }
        break;
    case SOCK_CLOSE_WAIT:
        /*Socket处于等待关闭状态*/
        close(CUS_COMM_SOCKET);
        break;
    case SOCK_CLOSED:
        /* 打开Socket，并配置为TCP无延时模式，打开一个本地端口*/
        socket(CUS_COMM_SOCKET, Sn_MR_TCP, ibox_config.local_port, Sn_MR_ND);
        break;
    default:
        break;
    }

    return sock_status;
}

// void EXTI9_5_IRQHandler(void)
// {
//     if (EXTI_GetITStatus(EXTI_Line9) != RESET) {

//         EXTI_ClearITPendingBit(EXTI_Line9); //清除中断标志位
//     }

//     EXTI_ClearITPendingBit(EXTI_Line9);
// }
/**
 * callback function for ip assign
 */
void eth_ip_assign(void)
{
    getIPfromDHCP(eth_msg_get.ip);
    getGWfromDHCP(eth_msg_get.gw);
    getSNfromDHCP(eth_msg_get.sub);

    /*设置网络*/
    setSUBR(eth_msg_get.sub);
    setGAR(eth_msg_get.gw);
    setSIPR(eth_msg_get.ip);

    setRTR(2000); /*设置超时时间*/
    setRCR(3);    /*设置最大重新发送次数*/

    ibox_printf(ibox_eth_debug, ("eth ip is update:%d.%d.%d.%d\r\n", eth_msg_get.ip[0], eth_msg_get.ip[1],
                                 eth_msg_get.ip[2], eth_msg_get.ip[3]));
}

/**
 * callback function for ip update
 */ 
void eth_ip_update(void)
{
    /* WIZchip Software Reset */
    setMR(MR_RST);
    getMR(); // for delay
    eth_ip_assign();
    setSHAR(eth_msg_get.mac);
}
/**
 * callback function for ip conflict
 */ 
void eth_ip_conflict(void)
{
    // WIZchip Software Reset
    setMR(MR_RST);
    getMR(); // for delay
    setSHAR(eth_msg_get.mac);
}
