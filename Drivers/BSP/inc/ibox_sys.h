/*
**************************************************************************************************
*文件：ibox_sys.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18
*描述：iBox 系统设置头文件。
* ************************************************************************************************
*/
#ifndef __IBOX_SYS_H
#define __IBOX_SYS_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"
#include "stdio.h"

#include <rtthread.h>

/*设置打印信息使用的函数*/
#ifdef USE_RTOS
#define ibox_printf(flag, message) \
do{ \
    if(flag) \
        rt_kprintf message; \
}while(0) 
#else
#define ibox_printf(flag, message) \
do{ \
    if(flag) \
        printf message; \
}while(0)
#endif
 
extern uint8_t ibox_debug_all;
extern uint8_t ibox_sys_debug;
extern uint8_t ibox_wifi_debug;
       
void sys_clk_init(void);
void sys_nvic_init(void);
void sys_delay_ms(uint32_t delay);
uint32_t get_sys_time_ms(void);
uint32_t get_sys_time_s(void);

uint32_t num_add_function(uint32_t a, uint32_t b);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_LED_H*/
