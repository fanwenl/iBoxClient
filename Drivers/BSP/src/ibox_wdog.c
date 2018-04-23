/*
**************************************************************************************************
*文件：iBox_wdog.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-18
*描述：iBox iot wdog驱动。
* ************************************************************************************************
*/
#include "ibox_wdog.h"

void wdog_init(void)
{
    /*PG6*/
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG, ENABLE);

    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOG, GPIO_Pin_6);
}

void wdog_feed(void)
{
    if (GPIO_ReadOutputDataBit(GPIOG, GPIO_Pin_6) == 1)
        GPIO_ResetBits(GPIOG, GPIO_Pin_6);
    else
        GPIO_SetBits(GPIOG, GPIO_Pin_6);
}
