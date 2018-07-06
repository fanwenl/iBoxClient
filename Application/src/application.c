/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include "ibox_board.h"
#include <rtthread.h>

#ifdef RT_USING_DFS
/* dfs filesystem:ELM filesystem init */
#include <dfs_elm.h>
/* dfs Filesystem APIs */
#include <dfs_fs.h>
#endif

#ifdef RT_USING_RTGUI
#include <rtgui/rtgui.h>
#include <rtgui/rtgui_server.h>
#include <rtgui/rtgui_system.h>
#include <rtgui/driver.h>
#include <rtgui/calibration.h>
#endif

//#include "led.h"

// ALIGN(RT_ALIGN_SIZE)
// static rt_uint8_t led_stack[ 512 ];
// static struct rt_thread led_thread;
extern void network_thread_entry(void* parameter);


static void timer_1s_timeout(void *parameter);

void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif
//    finsh_system_init();
//    rt_trace_init();
    ethernet_init();

}

void watchdog_thread_entry(void *parameter)
{
    while(1)
    {
        wdog_feed();
        led_toggle(LED_LORA);
        led_toggle(LED_NET);
        led_toggle(LED_SYS);
//        sys_delay_ms(100);
//        max485_send_data(buf);

//        ibox_printf(1, ("[CPU:%d][ADC1:%d][ADC2:%d]\r\n", get_cpu_temperature(), get_adc_voltage(0),
//                        get_adc_voltage(1)));
//        ibox_printf(1, ("[RTC:%ld]\r\n", RTC_GetCounter()));
//        
//        ibox_printf(1, ("%f\r\n", 3.1415926));
        rt_thread_delay(RT_TICK_PER_SECOND);
    }
    
}
int rt_application_init(void)
{
    rt_thread_t thread;
    rt_timer_t timer_1s;

//    rt_err_t result;

    /*创建一个软件timer 时间1S*/
    timer_1s = rt_timer_create("timer_1s",
                           timer_1s_timeout,
                           RT_NULL,
                           100,
                           RT_TIMER_FLAG_PERIODIC);
    if(timer_1s != RT_NULL)
    {
        rt_timer_start(timer_1s);
    }
    
    /* init network thread */
    thread = rt_thread_create("network_thread",
                            network_thread_entry,
                            RT_NULL,
                            4096, 4, 20);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    // thread = rt_thread_create("dhcp_thread",
    //                         dhcp_thread_entry,
    //                         RT_NULL,
    //                         4096, 4, 20);
    // if (thread != RT_NULL)
    // {
    //     rt_thread_startup(thread);
    // }
    
    thread = rt_thread_create("watchdog_thread",
                            watchdog_thread_entry,
                            RT_NULL,
                            256, 1, 20);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }

#if (RT_THREAD_PRIORITY_MAX == 32)
    thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 2, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (thread != RT_NULL)
        rt_thread_startup(thread);

    return 0;
}
static void timer_1s_timeout(void *parameter)
{
    DHCP_time_handler();
    DNS_time_handler();
}

/*@}*/
