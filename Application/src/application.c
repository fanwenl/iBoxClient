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
static void network_thread_entry(void* parameter)
{
//    unsigned int count=0;

//    rt_hw_led_init();

    while (1)
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
        
        rt_thread_delay( RT_TICK_PER_SECOND/2 );
    }
}

extern int finsh_system_init(void);
extern int rt_trace_init(void);
void rt_init_thread_entry(void* parameter)
{
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif
//    finsh_system_init();
//    rt_trace_init();

}

void watchdog_thread_entry(void *parameter)
{
    while(1)
    {
        wdog_feed();
        rt_thread_delay(RT_TICK_PER_SECOND);
    }
    
}
int rt_application_init(void)
{
    rt_thread_t thread;

//    rt_err_t result;

    /* init led thread */
    thread = rt_thread_create("network_thread",
                            network_thread_entry,
                            RT_NULL,
                            4096, 4, 20);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    
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
                                   2048, 8, 20);
#else
    init_thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   2048, 80, 20);
#endif

    if (thread != RT_NULL)
        rt_thread_startup(thread);

    return 0;
}

/*@}*/
