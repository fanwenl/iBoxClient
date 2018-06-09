#include "ibox_config.h"
#include <rthw.h>
#include <rtthread.h>

IBOX_CONFIG ibox_config = {
    "192.168.199.120", // server ip
    "10000",        // port
    "fanwenl.top", // dsn
#ifdef USE_WIFI
    "ziroom102",  // wifi_ssid[];
    "4001001101", // wifi_password[];
    "0",
    "0",
#else
    0,
#endif
};
void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
}

/**
 * This function will initial STM32 board.
 */
void rt_hw_board_init(void)
{
    /* NVIC Configuration */
//    NVIC_Configuration();

    /* Configure the SysTick */
    SysTick_Config( SystemCoreClock / RT_TICK_PER_SECOND );

#if STM32_EXT_SRAM
    EXT_SRAM_Configuration();
#endif

//    rt_hw_usart_init();
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}
