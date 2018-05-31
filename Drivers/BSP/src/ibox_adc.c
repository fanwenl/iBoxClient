/*
**************************************************************************************************
*文件：iBox_adc.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-18
*描述：iBox iot adc驱动。
* ************************************************************************************************
*/
#include "ibox_adc.h"

static uint16_t ADC_ConvertedValue[TOTAL_AD_CHANNEL * AD_TIMES];

/**
 * @brief  ADC模块初始化.
 * @param  None
 * @retval None
 */
void adc_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
    
    /*ADC GPIO配置*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    

    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    /* Enable clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE);

    /*复位ADC*/
    ADC_DeInit(ADC1);
    
    /*打开温度传感器*/
    ADC_TempSensorVrefintCmd(ENABLE);
    
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;      //独立模式
    ADC_InitStructure.ADC_ScanConvMode       = ENABLE;                    //连续多通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                    //连续转换
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None; //转换不受外界决定
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;       //右对齐
    ADC_InitStructure.ADC_NbrOfChannel       = TOTAL_AD_CHANNEL;          //扫描通道数
    ADC_Init(ADC1, &ADC_InitStructure);
    // ADC1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, ADC1_INDEX + 1, ADC_SampleTime_28Cycles5);
    // ADC2
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, ADC2_INDEX + 1, ADC_SampleTime_28Cycles5);
    // CPU Temperature
    ADC_RegularChannelConfig(ADC1, ADC_Channel_16, CPU_TEMP_INDEX + 1, ADC_SampleTime_239Cycles5);
    
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);
    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE); //使能或者失能指定的ADC
    /* Enable ADC1 reset calibaration register */
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC1))
        ;
    // ad_calibration = ADC_GetConversionValue(ADC1);
    //ADC_GetConversionValue(ADC1);
    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    /* DMA channel1 configuration */
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;                // ADC地址
    DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t)&ADC_ConvertedValue; //内存地址
    DMA_InitStructure.DMA_DIR                = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize         = TOTAL_AD_CHANNEL * AD_TIMES;
    DMA_InitStructure.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;       //外设地址固定
    DMA_InitStructure.DMA_MemoryInc          = DMA_MemoryInc_Enable;            //内存地址固定
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //半字
    DMA_InitStructure.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode               = DMA_Mode_Circular; //循环传输
    DMA_InitStructure.DMA_Priority           = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M                = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    /* Enable DMA channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);
}

/**
 * @brief  禁止ADC模块.
 * @param  None
 * @retval None
 */
void adc_disable(void)
{
    DMA_Cmd(DMA1_Channel1, DISABLE);
    ADC_Cmd(ADC1, DISABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
}

/***
 *@brief 获取CPU温度
 *@param none
 *@retval 温度值+100
 */
uint8_t get_cpu_temperature(void)
{
    uint32_t i;
    uint32_t temp = 0;
    uint8_t cpu_temp;
    float temp_vol;

    // 2. get the adc value
    for (i = 0; i < AD_TIMES; i++) {
        temp += ADC_ConvertedValue[CPU_TEMP_INDEX + i * TOTAL_AD_CHANNEL];
    }

    // 3. caculate
    temp /= AD_TIMES; //求平均
    temp_vol = 3.3 * (1.0 * temp / 4096);
    cpu_temp = (1.43 - temp_vol) / 4.3 * 1000 + 125;
    return cpu_temp;
}

/**
 * @brief 获取ADC的电压值
 * @param adc_channel通道选择：
 *       @arg ADC1_INDEX 
 *       @rag ADC2_INDEX
 * @retval 返回值为实际电压乘以100,0是一个错误值;
 **/
uint16_t get_adc_voltage(uint8_t adc_channel)
{
    uint32_t temp = 0;
    uint8_t i;
    float f_temp;

    for (i = 0; i < AD_TIMES; i++) {
        if (adc_channel == ADC1_INDEX)
            temp += ADC_ConvertedValue[ADC1_INDEX + i * TOTAL_AD_CHANNEL];
        else if (adc_channel == ADC2_INDEX)
            temp += ADC_ConvertedValue[ADC2_INDEX + i * TOTAL_AD_CHANNEL];
        else
            return 0;
    }
 
    /*计算温度值*/
    temp /= AD_TIMES;
    if (temp > 4000)
        return 0;
    else {
        f_temp = temp * 1.0 / 4096.0 * 330;

        return (uint16_t)f_temp;
    }
}
