/*
**************************************************************************************************
*文件：main_thread.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-07-18
*描述：ibox主业务线程。
* ************************************************************************************************
*/
#include "ibox_board.h"
#include "main_thread.h"

rt_event_t main_thread_event = RT_NULL;

void main_thread_entry(void *parameter)
{
    rt_uint32_t opt = 0;

    while (1) {
        if(rt_event_recv(main_thread_event, MAIN_THREAD_EVENT_ALL, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,RT_TICK_PER_SECOND * 10, &opt) == RT_EOK)
        {
            if(opt & MAIN_RECEIVE_DATA_EVENT)
            {
                /*收到数据进行cJSON解析。执行下发的数据和命令*/
            }
        }
        /*判断时间间隔进行周期性的数据上报*/

        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
}
void main_thread_init(void)
{
    main_thread_event = rt_event_create("MainEvent", RT_IPC_FLAG_FIFO);
    if(main_thread_event == RT_NULL)
    {
        ibox_printf(1, ("main thread event create fail!\r\n"));
    }
}