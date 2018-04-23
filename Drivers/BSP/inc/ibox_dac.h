/*
**************************************************************************************************
*文件：iBox_dac.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-18
*描述：iBox iot dac驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_DAC_H
#define __IBOX_DAC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "stdint.h"
#include "stm32f10x.h"

void dac_init(void);
void dac_set_vol(float vol);

#ifdef __cplusplus
 {
#endif /*__cplusplus*/

#endif /*__IBOX_DAC_H*/
