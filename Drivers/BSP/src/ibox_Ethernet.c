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
    sys_delay_ms(1);
    GPIO_SetBits(GPIOD, GPIO_Pin_8);
}
void w5500_init(void)
{
    w5500_reset_pin_config();
    w5500_inth_pin_config();
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line9) != RESET) {

        EXTI_ClearITPendingBit(EXTI_Line9); //清除中断标志位
    }

    EXTI_ClearITPendingBit(EXTI_Line9);
}
