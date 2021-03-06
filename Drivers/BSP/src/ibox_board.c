#include "ibox_board.h"
#include <rthw.h>
#include <rtthread.h>

IBOX_CONFIG ibox_config = {
    888888888,            // device sn
    "192.168.199.138",  // server ip 121.40.104.4
    "lot.zxbike.cc",      // server dsn
    61613,               //server ip
    8000,               //local ip
    "CMNET",
    "00:7B:04:AA:58:69",     //eth mac
    "114.114.114.114",
    0,
    5,
    "ziroom102",  // wifi_ssid[];
    "4001001111", // wifi_password[];
    "0",
    "0",
    865691033833807,
    "admin",
    "password",
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
    RCC_ClocksTypeDef RCC_ClockFreq;
    /*系统时钟初始化*/
    sys_clk_init();

    /* This function fills the RCC_ClockFreq structure with the current
     frequencies of different on chip clocks (for debug purpose) */
    RCC_GetClocksFreq(&RCC_ClockFreq);

    /* Enable Clock Security System(CSS): this will generate an NMI exception
     when HSE clock fails */
    RCC_ClockSecuritySystemCmd(ENABLE);

    /* NVIC configuration ------------------------------------------------------*/
    sys_nvic_init();
    
    /* Configure the SysTick */
    SysTick_Config( SystemCoreClock / RT_TICK_PER_SECOND );

    led_init(LED_LORA);
    led_init(LED_NET);
    led_init(LED_SYS);
    uart_init(UART1_DEBUG, 115200);

    adc_init();
    dac_init();

    RTC_alarm_init();

    max485_init();
    reset_key_init();
    w5500_hw_init();

    spi_init(LOAR_SPI);
        
#ifdef USE_WIFI
    wifi_init();
#else
    gprs_init();
#endif
    
#if STM32_EXT_SRAM
    EXT_SRAM_Configuration();
#endif

#ifdef RT_USING_DEVICE
    rt_hw_usart_init();
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
    
    wdog_init();

#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
}

/**
  *rt_kprintf的实现方式有两种
  *1、通过封装rt_hw_console_output()函数如下,不能打印浮点数。
  *2、实现printf,修改宏定义rt_kprintf()为printf().
 */
#if !defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
void rt_hw_console_output(const char *str)
{
    uint32_t timeout = 0;

    while (*str) {
        timeout = get_sys_time_s();
        USART_SendData(USART1, (uint8_t) *str++);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
            if (get_sys_time_s() - timeout > 1)
                break;
        }
    }
}
#endif

/**
  *finsh组件中从串口读取一个字符
  *在不使用RTT的设备驱动的情况下使用。
 */
#if !defined(RT_USING_DEVICE) && defined(RT_USING_FINSH)
uint16_t rt_hw_console_getchar(char *ch)
{
    return get_char_form_uart1(ch);
}
#endif

void ibox_assert(const char *exp, const char *func, uint32_t line)
{
    ibox_printf(1,("(%s) assertion failed at function:%s, line number:%d \n", exp, func, line));
    while (1);
}
