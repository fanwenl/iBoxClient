#include "ibox_config.h"

int main(void)
{
	    uint8_t buf[] = "20180419225500";
	
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


    adc_init();
    dac_init();


    dac_set_vol(2.0);



    RTC_alarm_init();
    RTC_StrSetTime(buf);
    ibox_printf(1, ("system is runing....\r\n"));
    uart_init(UART3_GPRS_WIFI, 115200);
    wifi_init();
    wdog_init();
    while (1) {
        wdog_feed();
        led_toggle(LED_LORA);
        led_toggle(LED_NET);
        led_toggle(LED_SYS);
        sys_delay_ms(100);

        ibox_printf(1, ("[CPU:%d][ADC1:%d][ADC2:%d]\r\n", get_cpu_temperature(), get_adc_voltage(0),\
                        get_adc_voltage(1)));
        ibox_printf(1, ("[RTC:%ld]\r\n", RTC_GetCounter()));
    }
}
