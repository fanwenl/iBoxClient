/*
**************************************************************************************************
*文件：iBox_led.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18s
*描述：iBox iot led驱动。
* ************************************************************************************************
*/
#include "iBox_led.h"

uint32_t led_pin[3]       = {LED_SYS_PIN, LED_LORA_PIN, LED_NET_PIN};
GPIO_TypeDef *led_port[3] = {LED_SYS_PORT, LED_LORA_PORT, LED_NET_PORT};

void led_init(led_typedef led)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin   = led_pin[led];

    GPIO_Init(led_port[led], &GPIO_InitStruct);

    /*默认为输出高电平(led off)*/
    GPIO_SetBits(led_port[led], led_pin[led]);
}
void led_on(led_typedef led) { GPIO_ResetBits(led_port[led], led_pin[led]); }
void led_off(led_typedef led) { GPIO_SetBits(led_port[led], led_pin[led]); }
void led_toggle(led_typedef led)
{
    if (GPIO_ReadOutputDataBit(led_port[led], led_pin[led]) == 1)
        GPIO_ResetBits(led_port[led], led_pin[led]);
    else
        GPIO_SetBits(led_port[led], led_pin[led]);
}
