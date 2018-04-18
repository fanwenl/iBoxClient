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

    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    /* Enable DMA clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    ADC_TempSensorVrefintCmd(ENABLE);
    // ADC_DeInit(ADC1);//温度异常
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;      //独立模式
    ADC_InitStructure.ADC_ScanConvMode       = ENABLE;                    //连续多通道模式
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;                    //连续转换
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None; //转换不受外界决定
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;       //右对齐
    ADC_InitStructure.ADC_NbrOfChannel       = TOTAL_AD_CHANNEL;          //扫描通道数
    ADC_Init(ADC1, &ADC_InitStructure);
    // BATT2_Voltage
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, BAT2_VOL_INDEX + 1, ADC_SampleTime_28Cycles5);
    // BATT2_NTC
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, BAT2_NTC_INDEX + 1, ADC_SampleTime_28Cycles5);
    // Motor_Current
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, MOTOR_CURRENT_INDEX + 1,
                             ADC_SampleTime_28Cycles5);
    // BATT1_NTC
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, BAT1_NTC_INDEX + 1, ADC_SampleTime_28Cycles5);
    // BATT1_Voltage
    ADC_RegularChannelConfig(ADC1, ADC_Channel_15, BAT1_VOL_INDEX + 1, ADC_SampleTime_28Cycles5);
    // Temperature
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
    ADC_GetConversionValue(ADC1);
    /* Start ADC1 Software Conversion */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    /* DMA channel1 configuration */
    DMA_DeInit(DMA1_Channel1);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;                // ADC地址
    DMA_InitStructure.DMA_MemoryBaseAddr     = (uint32_t) &ADC_ConvertedValue; //内存地址
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
    uint16_t temp_buf[AD_TIMES];
    uint32_t i;
    uint32_t temp;
    uint8_t cpu_temp;
    float temp_vol;

    // 2. get the adc value
    for (i = 0; i < AD_TIMES; i++) {
        temp_buf[i] = ADC_ConvertedValue[CPU_TEMP_INDEX + i * TOTAL_AD_CHANNEL];
    }

    // 3. caculate
    temp = 0;
    for (i = 0; i < AD_TIMES; i++) {
        temp += temp_buf[i];
    }
    temp /= AD_TIMES; //求平均
    temp_vol = 3.1 * (1.0 * temp / 4096);
    cpu_temp = (1.43 - temp_vol) / 4.3 * 1000 + 125;
    return cpu_temp;
}

/**
 * @brief 获取电池的温度
 * @param battery_num 电池编号，1或者2
 * @retval 返回值为电池实际温度+100;
 **/
uint8_t get_battery_temperature(uint8_t battery_No)
{
    uint16_t temp = 0;
    uint8_t i;
    float f_temp;

    /*使能NTC电阻,并等待采样*/
 //   BATT_NTC_POWER_ON;
 //   delay_us(1000);
    /*计算ADC采样值*/
    for (i = 0; i < AD_TIMES; i++) {
        if (battery_No == 1)
            temp += ADC_ConvertedValue[BAT1_NTC_INDEX + i * TOTAL_AD_CHANNEL];
        else if (battery_No == 2)
            temp += ADC_ConvertedValue[BAT2_NTC_INDEX + i * TOTAL_AD_CHANNEL];
        else
            return 0;
    }
    /*关闭NTC*/
//    BATT_NTC_POWER_OFF;
    /*计算温度值*/
    temp /= AD_TIMES;
    if (temp > 4000)
        return get_cpu_temperature();
    else {
        f_temp = temp * 1.0 / 4096.0 * 3.1;
        f_temp = f_temp / ((3.1 - f_temp) / 10000.0);
        /*RTC温度计算公式*/
//        temp = (1.0 / (log(f_temp / 10000.0) / 3435 + (1.0 / 298.15))) - 173.15;
        return (uint8_t) temp;
    }
}
/**
 *@brief 获取电池电压
 *@param battery_num 电池编号，1或者2
 *@retval 获取的电压值*10
 **/
uint8_t get_battery_voltage(uint8_t battery_No)
{
#define VOLTAGE_TIMES 10

    uint16_t temp_buf[AD_TIMES];
    uint32_t i;
    uint32_t vol;
    float voltage;
    // 1. enable battery voltage detect
//    BATT_VOLTAGE_DETECT_ON;
    //延时，等待adc采样稳定
//    delay_us(1000);
    // 2. get the adc value
    for (i = 0; i < AD_TIMES; i++) {
        if (battery_No == 1)
            temp_buf[i] = ADC_ConvertedValue[BAT1_VOL_INDEX + i * TOTAL_AD_CHANNEL];
        else if (battery_No == 2)
            temp_buf[i] = ADC_ConvertedValue[BAT2_VOL_INDEX + i * TOTAL_AD_CHANNEL];
        else
            return 0;
    }
//    BATT_VOLTAGE_DETECT_OFF;
    // 3. caculate the voltage
    vol = 0;
    for (i = 0; i < AD_TIMES; i++) {
        vol += temp_buf[i];
    }

    vol /= AD_TIMES;
    voltage = VOLTAGE_TIMES * 21 * 3.1 * vol / 4096;

    if (voltage >= 41.5)
        voltage = 42; //当电压大于4.15时认为充满

    vol = (uint32_t) voltage;
    // 4. return the voltage value
    return (uint8_t) vol;
}

/**
 *@brief 根据电压计算电机负载电流
 *@param none
 *@retval none
 **/
uint32_t get_motor_current(void)
{
    uint16_t temp_buf[AD_TIMES];
    uint32_t i;
    uint32_t current;

//    if (IS_BATT_NTC_POWER_ON) {
//        BATT_NTC_POWER_OFF;      // ADC通道件互相干扰，必须关掉输出
//        if (rt_critical_level()) //延时，等待adc采样稳定//延时，等待adc采样稳定
////            delay_ms(500);
//        else
////            rt_thread_delay(RT_TICK_PER_SECOND / 2);
 //   }
    // 2. get the adc value
    for (i = 0; i < AD_TIMES; i++) {
        temp_buf[i] = ADC_ConvertedValue[MOTOR_CURRENT_INDEX + i * TOTAL_AD_CHANNEL];
    }
    // 3. caculate the voltage
    current = 0;
    for (i = 0; i < AD_TIMES; i++) {
        current += temp_buf[i];
    }
    current /= AD_TIMES;
    // 4. return the voltage value
    return current;
}
