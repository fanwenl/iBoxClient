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


        
void sys_clk_init(void);
void sys_nvic_init(void);
void sys_delay_ms(uint32_t delay);        
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_LED_H*/
