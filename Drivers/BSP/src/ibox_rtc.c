/*
**************************************************************************************************
*文件：iBox_rtc.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-19
*描述：iBox iot rtc驱动。
* ************************************************************************************************
*/
#include "ibox_rtc.h"

// Table of month length (in days) for the Un-leap-year
static const u8 ULY[] = {0U, 31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
// Table of month length (in days) for the Leap-year
static const u8 LY[] = {0U, 31U, 29U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
// Number of days from begin of the non Leap-year
static const u16 MONTH_DAYS[] = {0U,   0U,   31U,  59U,  90U,  120U, 151U,
                                 181U, 212U, 243U, 273U, 304U, 334U};

/**
 * @function:RTC_alarm_init()
 * @param None
 * @retval None
 * @brief Config the RTC
 */
 
void RTC_alarm_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    uint32_t time = 0;
    /* Enable PWR and BKP clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    /* Allow access to BKP Domain */
    PWR_BackupAccessCmd(ENABLE);
    /* Reset Backup Domain */
    BKP_DeInit();
    /* RTC clock source configuration ----------------------------------------*/
    /* Enable the LSE OSC */
    RCC_LSEConfig(RCC_LSE_ON);
    /* Wait till LSE is ready */
    while ((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && (time < 5000)) {
        time++;
        sys_delay_ms(1);
    }

    if (time == 5000) {
        ibox_printf(1, ("RCC_FLAG_LSERDY error\r\n"));
        /* Enable the LSI OSC */
        RCC_LSICmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {
            ibox_printf(1, ("RCC_FLAG_LSIRDY error\r\n"));
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
            /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
        /* Set RTC prescaler: set RTC period to 1sec */
        RTC_SetPrescaler(400000);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    } else {
        /* Select the RTC Clock Source */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
    /* Wait for RTC registers synchronization */
    RTC_WaitForSynchro();
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();
        /* Set RTC prescaler: set RTC period to 1sec */
        RTC_SetPrescaler(32767);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    }

    /* 使能RTC闹钟中断*/
    RTC_ITConfig(RTC_IT_ALR, ENABLE);
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    //中断配置
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Line    = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    //设置闹钟中断的优先级
    NVIC_InitStructure.NVIC_IRQChannel                   = RTCAlarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 **@brief 设置RTC休眠时间
 **@param AlarmValue->休眠秒
 **@retval none
 **/
void go_stop(uint32_t AlarmValue)
{
    /* Wait till RTC Second event occurs */
    RTC_ClearFlag(RTC_FLAG_SEC);
    while (RTC_GetFlagStatus(RTC_FLAG_SEC) == RESET)
        ;

    RTC_SetAlarm(RTC_GetCounter() + AlarmValue); //当前时间+预警时间
    /* Wait until last write operation on RTC registers has finished */
    RTC_WaitForLastTask();

    /* Request to enter STOP mode with regulator in low power mode*/
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

// RTC闹钟中断
void RTCAlarm_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line17) != RESET) {
        EXTI_ClearITPendingBit(EXTI_Line17);
//        wakeup_source_flag = WAKEUP_FLAG_RTC;
    }
    if (RTC_GetITStatus(RTC_IT_ALR) != RESET) {
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
        /* Clear RTC Alarm interrupt pending bit */
        RTC_ClearITPendingBit(RTC_IT_ALR);
        /* Wait until last write operation on RTC registers has finished */
        RTC_WaitForLastTask();
    }
}
/**
 * @brief  由年月日计算出周数
 * @param[in]   year  年
 * @param[in]  month 月
 * @param[in]   days  日
 * @retval 返回计算出来的周期数
 */
int RTC_GetWeek(int year, int month, int days)
{
    static int mdays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30};
    int i, y = year - 1;
    for (i = 0; i < month; ++i)
        days += mdays[i];
    if (month > 2) {
        /* Increment date if this is a leap year after February */
        if (((year % 400) == 0) || ((year & 3) == 0 && (year % 100)))
            ++days;
    }
    return (y + y / 4 - y / 100 + y / 400 + days) % 7;
}

/**
 * @brief  由日期计算出秒
 * @param[in]  datetime 指向RTC时钟数据结构体的指针，详见rtc.h
 * @param[out] seconds 存放秒数据的地址
 * @retval None
 */
static void RTC_DateTimeToSecond(const RTC_DateTime_Type *datetime, u32 *seconds)
{
    /* Compute number of days from 1970 till given year*/
    *seconds = (datetime->year - 1970U) * DAYS_IN_A_YEAR;
    /* Add leap year days */
    *seconds += ((datetime->year / 4) - (1970U / 4));
    /* Add number of days till given month*/
    *seconds += MONTH_DAYS[datetime->month];
    /* Add days in given month*/
    *seconds += datetime->day;
    /* For leap year if month less than or equal to Febraury, decrement day counter*/
    if ((!(datetime->year & 3U)) && (datetime->month <= 2U)) {
        (*seconds)--;
    }

    *seconds = ((*seconds) * SECONDS_IN_A_DAY) + (datetime->hour * SECONDS_IN_A_HOUR) +
               (datetime->minute * SECONDS_IN_A_MIN) + datetime->second;
    (*seconds)++;
}

/**
 * @brief  由秒计算出日期
 * @param[in]  seconds    输入的秒（变量存储地址）
 * @param[out] datetime  计指向年月日等信息结构体指针
 * @retval None
 */
static void RTC_SecondToDateTime(const u32 *seconds, RTC_DateTime_Type *datetime)
{
    u32 x;
    u32 Seconds, Days, Days_in_year;
    const u8 *Days_in_month;
    /* Start from 1970-01-01*/
    Seconds = *seconds;
    /* days*/
    Days = Seconds / SECONDS_IN_A_DAY;
    /* seconds left*/
    Seconds = Seconds % SECONDS_IN_A_DAY;
    /* hours*/
    datetime->hour = Seconds / SECONDS_IN_A_HOUR;
    /* seconds left*/
    Seconds = Seconds % SECONDS_IN_A_HOUR;
    /* minutes*/
    datetime->minute = Seconds / SECONDS_IN_A_MIN;
    /* seconds*/
    datetime->second = Seconds % SECONDS_IN_A_MIN;
    /* year*/
    datetime->year = 1970;
    Days_in_year   = DAYS_IN_A_YEAR;

    while (Days > Days_in_year) {
        Days -= Days_in_year;
        datetime->year++;
        if (datetime->year & 3U) {
            Days_in_year = DAYS_IN_A_YEAR;
        } else {
            Days_in_year = DAYS_IN_A_LEAP_YEAR;
        }
    }

    if (datetime->year & 3U) {
        Days_in_month = ULY;
    } else {
        Days_in_month = LY;
    }

    for (x = 1U; x <= 12U; x++) {
        if (Days <= (*(Days_in_month + x))) {
            datetime->month = x;
            break;
        } else {
            Days -= (*(Days_in_month + x));
        }
    }
    datetime->day = Days + 1;
}

/**
 * @brief  获得RTC的时间
 * @code
 *      RTC_DateTime_Type ts;    //申请一个结构体
 *      RTC_GetTime(&ts);    //将日期存储到ts中
 * @endcode
 * @param  datetime  返回出来的年月日等信息结构体
 * @retval None
 */
void RTC_GetTime(RTC_DateTime_Type *datetime)
{
    u32 i;

    if (!datetime) {
        return;
    }
    i = RTC_GetCounter() + 60 * 60 * 8; //1970-01-01 8:00
    RTC_SecondToDateTime(&i, datetime);
}

/**
 * @brief  设置RTC的时间
 * @param[in]  timestr,时间字符串,如20160312073528, yyyymmddhhmmss
 * @retval None
 */
void RTC_StrSetTime(uint8_t *timestr)
{
    RTC_DateTime_Type datetime;

    datetime.year = (timestr[0] - '0') * 1000 + (timestr[1] - '0') * 100 + (timestr[2] - '0') * 10 +
                    (timestr[3] - '0');
    datetime.month  = (timestr[4] - '0') * 10 + (timestr[5] - '0');
    datetime.day    = (timestr[6] - '0') * 10 + (timestr[7] - '0');
    datetime.hour   = (timestr[8] - '0') * 10 + (timestr[9] - '0');
    datetime.minute = (timestr[10] - '0') * 10 + (timestr[11] - '0');
    datetime.second = (timestr[12] - '0') * 10 + (timestr[13] - '0');
    RTC_SetTime(&datetime);
}

/**
 * @brief  设置RTC的时间
 * @param[in]  datetime  指向时间的结构体指针
 * @retval None
 */
void RTC_SetTime(RTC_DateTime_Type *datetime)
{
    u32 i;
    if (!datetime) {
        return;
    }
    RTC_DateTimeToSecond(datetime, &i);
    RTC_SetCounter(i);
}

/**
 * @brief  获得RTC时间字符串
 * @param[in] rtcstr 字符串
 * @retval None
 */
void RTC_Get_str(uint8_t *rtcstr)
{
    RTC_DateTime_Type datetime;
    RTC_GetTime(&datetime);
    sprintf((char *) rtcstr, "%d%02d%02d%02d%02d%02d", datetime.year, datetime.month, datetime.day,
            datetime.hour, datetime.minute, datetime.second);
    return;
}
