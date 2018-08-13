/*
**************************************************************************************************
*文件：application.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-07-18
*描述：ibox任务创建文件。
* ************************************************************************************************
*/
#include "ibox_board.h"
#include "cJSON.h"
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

extern void network_thread_entry(void* parameter);
extern void wired_thread_entry(void *parameter);
extern void wireless_thread_entry(void *parameter);
extern void lora_thread_entry(void *parameter);
extern void main_thread_entry(void *parameter);


static void timer_1s_timeout(void *parameter);

void rt_init_thread_entry(void* parameter)
{
    cJSON_Hooks cjson_hook_temp;
#ifdef RT_USING_COMPONENTS_INIT
    /* initialization RT-Thread Components */
    rt_components_init();
#endif
    /*线程初始化*/
    main_thread_init();
    
    network_thread_init();
    /*初始化ETH网络*/
    ethernet_init();
    
    /*初始化LoRa*/
    lora_init();

    /*配置cJSON的hook*/
    cjson_hook_temp.malloc_fn = (void *(*)(size_t))rt_malloc;
    cjson_hook_temp.free_fn = rt_free;
    cJSON_InitHooks(&cjson_hook_temp);
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
                            NETWORK_THREAD_STACK_SIZE,
                            NETWORK_THREAD_PRIINIT, 20);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    thread = rt_thread_create("wireless",
                             wireless_thread_entry,
                             RT_NULL,
                             WIRELESS_THREAD_STACK_SIZE,
                             WIRELESS_THREAD_PRIINIT, 20);
     if (thread != RT_NULL)
     {
         rt_thread_startup(thread);
     }
         thread = rt_thread_create("wired",
                             wired_thread_entry,
                             RT_NULL,
                             WIRED_THREAD_STACK_SIZE, 
                             WIRED_THREAD_PRIINIT, 20);
     if (thread != RT_NULL)
     {
         rt_thread_startup(thread);
     }
    thread = rt_thread_create("loRa",
                             lora_thread_entry,
                             RT_NULL,
                             LORA_THREAD_STACK_SIZE, 
                             LORA_THREAD_PRIINIT, 20);
     if (thread != RT_NULL)
     {
         rt_thread_startup(thread);
     }
    thread = rt_thread_create("main",
                             main_thread_entry,
                             RT_NULL,
                             MAIN_THREAD_STACK_SIZE, 
                             MAIN_THREAD_PRIINIT, 20);
     if (thread != RT_NULL)
     {
         rt_thread_startup(thread);
     }
    
    thread = rt_thread_create("watchdog_thread",
                            watchdog_thread_entry,
                            RT_NULL,
                            WATCHDOG_THREAD_STACK_SIZE,
                            WATCHDOG_THREAD_PRIINIT, 20);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }

    thread = rt_thread_create("init",
                                   rt_init_thread_entry, RT_NULL,
                                   INIT_THREAD_STACK_SIZE,
                                   INIT_THREAD_PRIINIT, 20);

    if (thread != RT_NULL)
        rt_thread_startup(thread);

    return 0;
}
extern uint8_t is_wired_link_ok;
extern uint8_t is_wireless_link_ok;
extern uint8_t is_lora_link_ok;

static void timer_1s_timeout(void *parameter)
{
    DHCP_time_handler();
    DNS_time_handler();
    led_toggle(LED_SYS);
    if(is_wired_link_ok || is_wireless_link_ok)
        led_toggle(LED_NET);

    if(is_lora_link_ok)
        led_toggle(LED_LORA);
}
