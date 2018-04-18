/*
**************************************************************************************************
*文件：iBox_adc.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-18
*描述：iBox iot adc驱动。
* ************************************************************************************************
*/
#include "stdint.h"
#include "stm32f10x.h"



#ifndef _adc_config_h
#define _adc_config_h

#define ADC1_DR_Address             ((u32)0x40012400+0x4c)

#define AD_TIMES                    10
#define TOTAL_AD_CHANNEL            6

#define BAT2_VOL_INDEX              0
#define BAT2_NTC_INDEX              1
#define MOTOR_CURRENT_INDEX         2
#define BAT1_NTC_INDEX              3
#define BAT1_VOL_INDEX              4
#define CPU_TEMP_INDEX              5

/**
  * @brief  禁止ADC模块.
  * @param  None
  * @retval None
  */
extern void adc_disable(void);

/**
  * @brief  ADC模块初始化.
  * @param  None
  * @retval None
  */
extern void adc_init(void);

extern uint8_t get_cpu_temperature(void);
extern uint8_t get_battery_temperature(uint8_t battery_No);
extern uint8_t get_battery_voltage(uint8_t battery_No);
extern uint32_t get_motor_current(void);


#endif
