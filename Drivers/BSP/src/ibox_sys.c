/*
**************************************************************************************************
*文件：ibox_sys.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18
*描述：iBox 系统设置。
* ************************************************************************************************
*/
#include "ibox_sys.h"

/*degug信息输出控制*/
uint8_t ibox_debug_all = 0;
uint8_t ibox_sys_debug = 1;
uint8_t ibox_wifi_debug = 1;
uint8_t ibox_gprs_debug = 1;
uint8_t ibox_net_debug = 1;
uint8_t ibox_lora_debug = 1;
uint8_t ibox_mqtt_debug = 1;

static void delay_ms(uint32_t delay);
static void delay_us(uint32_t delay);

void sys_clk_init(void)
{
    ErrorStatus HSEStartUpStatus;
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration -----------------------------*/   
    /* RCC system reset(for debug purpose) */
    RCC_DeInit();

    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    HSEStartUpStatus = RCC_WaitForHSEStartUp();

    if (HSEStartUpStatus == SUCCESS)
    {
        /* Enable Prefetch Buffer */
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        /* Flash 2 wait state */
        FLASH_SetLatency(FLASH_Latency_2);
 
        /* HCLK = SYSCLK*/
        RCC_HCLKConfig(RCC_SYSCLK_Div1); 
  
        /* PCLK2 = HCLK */
        RCC_PCLK2Config(RCC_HCLK_Div1); 

        /* PCLK1 = HCLK/2 */
        RCC_PCLK1Config(RCC_HCLK_Div2);

        /* PLLCLK = 12MHz / 2 * 12 = 72 MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_12);


        /* Enable PLL */ 
        RCC_PLLCmd(ENABLE);

        /* Wait till PLL is ready */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        /* Select PLL as system clock source */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* Wait till PLL is used as system clock source */
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else
    {   /* If HSE fails to start-up, the application will have wrong clock configuration.
        User can add here some code to deal with this error */    

        /* Go to infinite loop */
        while (1)
        {
        }
    }   
}
void sys_nvic_init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
#ifdef  VECT_TAB_RAM  
    /* Set the Vector Table base location at 0x20000000 */ 
    NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
#else  /* VECT_TAB_FLASH  */
    /* Set the Vector Table base location at 0x08000000 */ 
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif 

    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//2bit : 2bit

    /* Enable and configure RCC global IRQ channel */
    NVIC_InitStructure.NVIC_IRQChannel = RCC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
        
}
/**ms级别函数,使用RTOS后精度是RTOS的TICK
 * 本项目中RTOS的TICK为10ms
 */ 
void sys_delay_ms(uint32_t delay)
{
#ifdef USE_RTOS
    uint32_t delay_tick = 0;
    
    if(delay < 10)
        delay_tick = 1;
    else
        delay_tick = delay / (1000 / RT_TICK_PER_SECOND);
    /*判断系统是否运行*/
    if(rt_critical_level())
        rt_thread_delay(delay_tick);
    else
        delay_ms(delay);
#else
    delay_ms(delay);
#endif    
}
static void delay_ms(uint32_t delay)
{
   u16 i=0;  
   while(delay--)
   {
      i=12000;
      while(i--) ;    
   }   
    
}
/**
 * delay_us直接采用硬延时
 * 一般不使用该延时，如果有需求后续实现
 */ 
void sys_delay_us(uint32_t delay)
{
//#ifdef USE_RTOS
    
//#else
    delay_us(delay);
//#endif    
}
static void delay_us(uint32_t delay)
{  
   while(delay--)
   {  
   }   
    
}
/**
 * 返回单位为RTOS TICK单位的数
 * 本项目中返回的单位为10ms
 */ 
uint32_t get_sys_time_ms(void)
{
    return rt_tick_get();
}
/**
 * 返回单位为s数
 */ 
uint32_t get_sys_time_s(void)
{
    return RTC_GetCounter(); 
}


uint32_t num_add_function(uint32_t a, uint32_t b)
{
    return(a+b);
}
