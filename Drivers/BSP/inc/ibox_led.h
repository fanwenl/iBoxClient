/*
**************************************************************************************************
*文件：iBox_led.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18
*描述：iBox iot led驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_LED_H
#define __IBOX_LED_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"

typedef enum {
    LED_SYS = 0,
    LED_GREEN = LED_SYS,
    LED_LORA = 1,
    LED_YELLOW = LED_LORA,
    LED_NET = 2,
    LED_WHITE = LED_NET,
}led_typedef;

#define LED_SYS_PIN         GPIO_Pin_9
#define LED_NET_PIN         GPIO_Pin_12
#define LED_LORA_PIN        GPIO_Pin_10

#define LED_SYS_PORT        GPIOE
#define LED_NET_PORT        GPIOE
#define LED_LORA_PORT       GPIOE


void led_init(led_typedef led);
void led_on(led_typedef led);
void led_off(led_typedef led);
void led_toggle(led_typedef led);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_LED_H*/
