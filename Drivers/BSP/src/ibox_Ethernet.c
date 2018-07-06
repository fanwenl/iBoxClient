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

ETH_MSG eth_msg_get;

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
    memcpy((char *)eth_msg_get.mac, (const char *)ibox_config.eth_mac, 6);
    setSHAR(ibox_config.eth_mac);
}
void set_network(void)
{
  uint8_t ip[4] = {192,168,123,100}; 
  uint8_t gw[4] = {192,168,123,1};
  uint8_t sub[4] = {255,255,255,0};
  uint8_t dns[4] = {192,168,123,1};
  setSHAR(eth_msg_get.mac);/*����Mac��ַ*/
  setSUBR(sub);/*������������*/
  setGAR(gw);/*����Ĭ������*/
  setSIPR(ip);/*����Ip��ַ*/

  setRTR(2000);/*�������ʱ��ֵ*/
  setRCR(3);/*����������·��ʹ���*/
 
  getSIPR (ip);
  printf("IP : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getSUBR(ip);
  printf("SN : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
  getGAR(ip);
  printf("GW : %d.%d.%d.%d\r\n", ip[0],ip[1],ip[2],ip[3]);
} 
/**
 * 以太网相关的初始化
 * 包括DNS解析、HDCP动态获取...
 */ 
void ethernet_init(void)
{
    set_w5500_mac();
    /*设置socket的大小*/
    wizchip_init(tx_socket_size, rx_socket_size);
    
    DHCP_init(DHCP_SOCKET, dhcp_dns_buf);

    /*dns 解析和dhcp共用一个buf*/
    DNS_init(DNS_SOCKET,dhcp_dns_buf);
    
    set_network();
    
    uint8_t server_ip[4] = {192,168,123,94}; 
    while(1)
    {
    		switch(getSn_SR(0))														// 获取socket0的状态
		{
			case SOCK_INIT:															// Socket处于初始化完成(打开)状态
                connect(0, server_ip,9000);
			break;
			case SOCK_ESTABLISHED:											// Socket处于连接建立状态

			break;
			case SOCK_CLOSE_WAIT:												// Socket处于等待关闭状态
					close(0);																// 关闭Socket0
			break;
			case SOCK_CLOSED:														// Socket处于关闭状态
					socket(0,Sn_MR_TCP,6000,Sn_MR_ND);		// 打开Socket0，并配置为TCP无延时模式，打开一个本地端口
			break;
		}
    }
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line9) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line9); //清除中断标志位
    }

    EXTI_ClearITPendingBit(EXTI_Line9);
}
