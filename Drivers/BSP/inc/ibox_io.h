/*
**************************************************************************************************
*文件：ibox_io.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18
*描述：iBox io输出口头文件。
* ************************************************************************************************
*/
#ifndef __IBOX_IO_H
#define __IBOX_IO_H

#ifdef __cplusplus
    extern "C" {
#endif

#include "stm32f10x.h"
#include "stdint.h"
typedef enum
{
    IO_IN1,
    IO_IN2,
}Input_TypeDef;
typedef enum
{
    IO_OUT1,
    IO_OUT2,
}Output_TypeDef;

#define IN1_PIN         GPIO_Pin_0             
#define IN2_PIN         GPIO_Pin_1
#define IN1_PORT        GPIOF
#define IN2_PORT        GPIOF

#define OUT1_PIN        GPIO_Pin_0
#define OUT2_PIN        GPIO_Pin_1
#define OUT1_PORT       GPIOE
#define OUT2_pORT       GPIOE 


void IO_init(void);
void IO_Output(Output_TypeDef pin,BitAction value);
uint8_t Read_IO(Input_TypeDef pin);

        
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IBOX_IO_H*/
