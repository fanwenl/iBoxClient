/*
**************************************************************************************************
*文件：iBox_gprs.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-04-21
*描述：iBox iot gprs驱动。
* ************************************************************************************************
*/
#ifndef __IBOX_GPRS_H
#define __IBOX_GPRS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stm32f10x.h"

/*gprs reset控制*/
#define GPRS_RESET_PIN          GPIO_Pin_0
#define GPRS_RESET_PORT         GPIOG
#define GPRS_RESET_PIN_CLK      RCC_APB2Periph_GPIOG

/*低电平复位*/
#define GPRS_RESET_L        GPIO_ResetBits(GPRS_RESET_PORT, GPRS_RESET_PIN)
#define GPRS_RESET_H        GPIO_SetBits(GPRS_RESET_PORT, GPRS_RESET_PIN)

typedef enum{
    GPRS_STATUS_CHECK,       //模块检测关闭回显
    GPRS_STATUS_INIT,        //检查sim卡
    GPRS_STATUS_SET_MISC,    //设置一些参数
    GPRS_STATUS_CHECK_CSQ,   //检测当前信号强度
    GPRS_STATUS_CHECK_REG,   //检测当前网络注册状态
    GPRS_STATUS_CHECK_CGATT, //GPRS附着状态
    GPRS_STATUS_SET_APN,     //设定APN GPRS -> IP START 
    GPRS_STATUS_SET_CIICR,   //建立无线链路
    GPRS_STATUS_GET_IP,      //获取ip
    GPRS_STATUS_RESTART_TCP, //重新开始tcp连接
    GPRS_STATUS_START_TCP,   //开始建立tcp连接
    GPRS_STATUS_TCP_CONNECT, //tcp成功连接
    GPRS_STATUS_COMMUNICATE, //GPRS正常通信
    GPRS_STATUS_RX_PROC,     //接收处理
    GPRS_STATUS_FRAME_PROC,  //帧处理   
}GPRS_STATUS_ENUM;


void gprs_init(void);
void gprs_at_fsm(void);


#ifdef __cplusplus
}
#endif

#endif /*__IBOX_GPRS_H*/
