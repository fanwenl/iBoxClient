/*
**************************************************************************************************
*文件：iBox_key.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-25
*描述：iBox iot key驱动。
* ************************************************************************************************
*/
#include "ibox_key.h"
#include "ibox_sys.h"

/**
 * @function reset_key_init()
 * @par:none
 * @revt:none
 * @bref:按键驱动采用中断的方式,有消斗和松手检测
 */
void reset_key_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /*PE15 复位按键*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin   = GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStruct);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*中断配置*/
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource15);

    //中断配置为下降沿触发
    EXTI_InitStructure.EXTI_Line    = EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_ClearITPendingBit(EXTI_Line15);
    EXTI_Init(&EXTI_InitStructure);
}

/*按键中断*/
void EXTI15_10_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line15) != RESET) {

                                             //关闭中断防止多次触发
        sys_delay_ms(10);
        if(GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_15) == 0)
        {
            NVIC_SystemReset();
        }
        EXTI_ClearITPendingBit(EXTI_Line15); //清除中断标志位
    }

    EXTI_ClearITPendingBit(EXTI_Line15);
}