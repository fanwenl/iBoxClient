/*
**************************************************************************************************
*文件：iBox_rtc.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-19
*描述：iBox iot rtc驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_RTC_H
#define __IBOX_RTC_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"
#include "stdio.h"
#include "ibox_sys.h"

typedef struct 
{
   u16 year;    ///< 年Range from 200 to 2099.
   u16 month;   ///< 月Range from 1 to 12.
   u16 day;     ///< 日Range from 1 to 31 (depending on month).
   u16 hour;    ///< 时Range from 0 to 23.
   u16 minute;  ///< 分Range from 0 to 59.
   u8 second;   ///< 秒Range from 0 to 59.
}RTC_DateTime_Type;

#define SECONDS_IN_A_DAY (86400U)
#define SECONDS_IN_A_HOUR (3600U)
#define SECONDS_IN_A_MIN (60U)
#define DAYS_IN_A_YEAR (365U)
#define DAYS_IN_A_LEAP_YEAR (366U)


void RTC_alarm_init(void);
void go_stop(uint32_t AlarmValue);
int RTC_GetWeek(int year, int month, int days);
void RTC_GetTime(RTC_DateTime_Type *datetime);
void RTC_StrSetTime(uint8_t *timestr);
void RTC_Get_str(uint8_t *rtcstr);
void RTC_SetTime(RTC_DateTime_Type *datetime);

#ifdef __cplusplus
    }
#endif /*__cplusplus*/

#endif /*__IBOX_RTC_H*/
