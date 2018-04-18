#include "ibox_config.h"

int main (void)
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
    
    led_init(LED_LORA);
    led_init(LED_NET);
    led_init(LED_SYS);
    uart_init(UART1_DEBUG, 115200);
    
    printf("system is runing....\r\n");
    
    while(1)
    {
        led_toggle(LED_LORA);
        led_toggle(LED_NET);
        led_toggle(LED_SYS);
        sys_delay_ms(500);
        
        printf("hello iBox....\r\n");
    }
    
}
