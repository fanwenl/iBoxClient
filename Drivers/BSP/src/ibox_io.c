#include "ibox_io.h"

uint32_t in_pin[2] = {IN1_PIN, IN2_PIN };
uint32_t out_pin[2] = {OUT1_PIN, OUT2_PIN};

GPIO_TypeDef * in_port[2] = {IN1_PORT, IN2_PORT};
GPIO_TypeDef * out_port[2] = {OUT1_PORT, OUT2_pORT};

void IO_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF, ENABLE);

    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin = OUT1_PIN;
    
    GPIO_Init(OUT1_PORT, &GPIO_InitStruct);
    
    /*默认为输出低电平*/
    GPIO_SetBits(OUT1_PORT, OUT1_PIN);
    
    GPIO_InitStruct.GPIO_Pin = OUT2_PIN;
    GPIO_Init(OUT1_PORT, &GPIO_InitStruct);
    /*默认为输出低电平*/
    GPIO_SetBits(OUT1_PORT, OUT1_PIN);
    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin = IN1_PIN;
    GPIO_Init(IN1_PORT, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = IN2_PIN;
    GPIO_Init(IN2_PORT, &GPIO_InitStruct);    
}
void IO_Output(Output_TypeDef pin,BitAction value)
{
    GPIO_WriteBit(out_port[pin],out_pin[pin],value);   
}
uint8_t Read_IO(Input_TypeDef pin)
{
    return GPIO_ReadInputDataBit(in_port[pin], in_pin[pin]);    
}
