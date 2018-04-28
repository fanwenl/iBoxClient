/*
**************************************************************************************************
*文件：iBox_Ethernet.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-28
*描述：iBox iot Eth接口驱动。
* ************************************************************************************************
*/
#include "iBox_Ethernet.h"

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
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_9;

    GPIO_Init(GPIOD, &GPIO_InitStruct);
}
void w55000_reset(void)
{
    GPIO_ResetBits(GPIOD, GPIO_Pin_8);
    sys_delay_ms(1);
    GPIO_SetBits(GPIOD, GPIO_Pin_8);
}