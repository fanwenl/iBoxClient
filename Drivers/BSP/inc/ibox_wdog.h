/*
**************************************************************************************************
*文件：iBox_wdog.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-18
*描述：iBox iot wdog驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_WDOG_H
#define __IBOX_WDOG_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"

void wdog_init(void);
void wdog_feed(void);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_WDOG_H*/
