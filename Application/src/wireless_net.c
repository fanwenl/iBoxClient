
#include "ibox_board.h"
void wireless_thread_entry(void *parameter)
{
    gprs_get_imei();
    while(1)
    {
        gprs_at_fsm();
        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
    
}