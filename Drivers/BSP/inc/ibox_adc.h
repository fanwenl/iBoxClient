/*
**************************************************************************************************
*文件：iBox_adc.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-18
*描述：iBox iot adc驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_ADC_H
#define __IBOX_ADC_H

#include "stdint.h"
#include "stm32f10x.h"

#ifdef __cplusplus
    extern "C" {
#endif

#define ADC1_DR_Address             ((u32)0x40012400+0x4c)

#define AD_TIMES                    10
#define TOTAL_AD_CHANNEL            3

#define ADC1_INDEX                  0
#define ADC2_INDEX                  1
#define CPU_TEMP_INDEX              2


extern void adc_disable(void);
extern void adc_init(void);
extern uint8_t get_cpu_temperature(void);
extern uint16_t get_adc_voltage(uint8_t adc_channel);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_ADC_H*/
