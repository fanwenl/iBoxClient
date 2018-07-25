/*
**************************************************************************************************
*文件：wireles_net.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-07-18
*描述：ibox无线网络任务文件(WIFI、GPRS)。
* ************************************************************************************************
*/
#include "ibox_board.h"
extern uint16_t wifi_tx_len;
void wireless_thread_entry(void *parameter)
{
#ifndef USE_WIFI
    gprs_get_imei();
#endif
    while(1)
    {
#ifdef USE_WIFI
        esp8266_at_fsm();
        wifi_tx_len = 10;
#else
        gprs_at_fsm();
#endif
        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
    
}
