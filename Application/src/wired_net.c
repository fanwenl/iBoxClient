#include "ibox_board.h"
void wired_thread_entry(void *parameter)
{
    while(1)
    {
        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
    
}