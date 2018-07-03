/*
**************************************************************************************************
*文件：iBox_spi.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-29
*描述：iBox iot spi驱动。
*     SPI2用作W5500的驱动接口，SPI1用作LoRa模块,两个SPI接口均采用DMA传输。
*     SPI接在APB2总线，SPI最高速率为36MHz，W5500最高80MHz，支持mode0和mode3模式。
* ************************************************************************************************
*/

#include "ibox_spi.h"

void spi_init(SPI_typedef module)
{
    SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    switch (module) {
    case LOAR: {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; // MOSI MISO SCK复用
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;                      //复用推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;       // SPI_CS
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; //推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        LOAR_CS_H; //置为高位,失能flash器件

        //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_Mode      = SPI_Mode_Master; //设置SPI工作模式:设置为主SPI
        SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b; //设置SPI的数据大小:SPI发送接收8位帧结构
        SPI_InitStructure.SPI_CPOL      = SPI_CPOL_Low;
        //数据捕获于第2个时钟沿，也就是上升沿数据有效
        SPI_InitStructure.SPI_CPHA      = SPI_CPHA_1Edge;
        // SPI_NSS_Soft;//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
        SPI_InitStructure.SPI_NSS       = SPI_NSS_Soft;
        //经过测试，最小可以为8，预分频值为16
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
        //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
        SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
        SPI_InitStructure.SPI_CRCPolynomial     = 10; // CRC值计算的多项式

        SPI_Init(SPI1, &SPI_InitStructure); //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
        SPI_Cmd(SPI1, ENABLE);              //使能SPI外设
    } break;
    case ETHERNRT: {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

        GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP; //复用推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
        SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_Mode      = SPI_Mode_Master; //设置SPI工作模式:设置为主SPI
        SPI_InitStructure.SPI_DataSize  = SPI_DataSize_8b; //设置SPI的数据大小:SPI发送接收8位帧结构
        // spi mode = 0
        SPI_InitStructure.SPI_CPOL      = SPI_CPOL_Low;
        SPI_InitStructure.SPI_CPHA      = SPI_CPHA_1Edge;
        // SPI_NSS_Soft;//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
        SPI_InitStructure.SPI_NSS       = SPI_NSS_Hard;
        //定义波特率预分频的值:波特率预分频值为2.(36MHz)
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
        //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
        SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
        SPI_InitStructure.SPI_CRCPolynomial     = 7; // CRC值计算的多项式

        SPI_Init(SPI2, &SPI_InitStructure); //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器
        SPI_Cmd(SPI2, ENABLE);              //使能SPI外设
    } break;
		default:
			break;
    }
}

void w5500_cs_select(void)
{
      SPI_SSOutputCmd(SPI2, ENABLE);
}

void w5500_cs_deselect(void)
{
      SPI_SSOutputCmd(SPI2, DISABLE);
}
uint8_t w5500_spi_readbyte(void)
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);

    return SPI_I2S_ReceiveData(SPI2);
}

void w5500_spi_writebyte(uint8_t wb) 
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, wb);
}
void w5500_spi_readburst(uint8_t* pBuf, uint16_t len)
{
    while(len)
    {
        *pBuf++ = w5500_spi_readbyte();
        len--;
    }
}

void w5500_spi_writeburst(uint8_t* pBuf, uint16_t len)
{
    while(len)
    {
        w5500_spi_writebyte(*(pBuf++));
        len--;
    }
}