/*
**************************************************************************************************
*文件：ibox_lora.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-26
*描述：ibox LoRa底层IO文件,模块：LSD4RF-2F717N30。
* 产品名称：LoRa SX1278 470M 100mW标准模块
* ************************************************************************************************
*/
#include "ibox_lora.h"
#include "ibox_spi.h"
/**
 * lora 相关的pin配置，包括lora的CS应交和LoRa模块DIO0引脚
 * DIO0引脚主要映射模块内部的RxDone，TxDone，CadDone等信号
 */
static void lora_pin_config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef  EXTI_InitStructure;
 	NVIC_InitTypeDef  NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(LORA_RST_CLK | LORA_DIO0_CLK | RCC_APB2Periph_AFIO, ENABLE);
	
	//RST,PF13
    GPIO_InitStructure.GPIO_Pin   = LORA_RST_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LORA_RST_PORT, &GPIO_InitStructure);

    LORA_RST_SET_H;             //LORA复位脚，拉高
    
    //DIO0 PG7
	GPIO_InitStructure.GPIO_Pin  = LORA_DIO0_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//输入下拉 
 	GPIO_Init(LORA_DIO0_PORT, &GPIO_InitStructure);
  /* EXTI interrupt init*/
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG,GPIO_PinSource7);

	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;//接收到信号时DIO0输出高电平
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure); 
}
void lora_init(void)
{
    lora_pin_config();
    spi_init(LOAR_SPI);
}
/**
 * LoRa模块DIO0中断服务函数
 */
void EXTI9_5_IRQHandler(void)
{
//	uint8_t lora_irq = 0;
	
	// if(EXTI_GetITStatus(EXTI_Line7) != RESET)
	// {
	// 	EXTI_ClearITPendingBit(EXTI_Line7);  //清除LINE7上的中断标志位 
	// 	SX1276Read(REG_LR_IRQFLAGS, &lora_irq);
		
	// 	if((lora_irq & RFLR_IRQFLAGS_TXDONE) == RFLR_IRQFLAGS_TXDONE)//发送中断
	// 	{
	// 		SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE);	
	// 		//WIFI_LED =! WIFI_LED;
	// 	}					
	// 	else if((lora_irq & RFLR_IRQFLAGS_RXDONE) == RFLR_IRQFLAGS_RXDONE)//接收中断
	// 	{
	// 		SX1276Write(REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXDONE); 
	// 		/*RF将接收到的数据存到缓冲数组里面*/

	// 		LSD_RF_RxPacket(Rxbuffer);//这里面清了中断标志
// #ifdef SPI_LORA_TX
// 		  UpdateLoraInfo(Rxbuffer);
// #endif/*SPI_LORA_TX*/	
				
// 			/*每次收到数据LORA_LED跳转一次*/
// 			//LORA_LED = !LORA_LED;
// #ifdef SPI_LORA_RX		
// 			SX1276_SendBack(Rxbuffer);
// 			while(EXTI_GetITStatus(EXTI_Line7) == RESET);//等待发送完毕，再进入接收模式，，，注意！
// 			EXTI_ClearITPendingBit(EXTI_Line7);  //清除LINE7上的中断标志位
// #endif/*SPI_LORA_RX*/	
//		}			
		
//		Rx_mode();	//接收完一包数据或发送完一包数据后，重新进入接收，可变模式下输入参数无效，固定模式下，接收固定参数长度数据
//	}//明天看下这里面的payload赋值
}
