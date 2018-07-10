/*
**************************************************************************************************
*文件：Application.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2017-11-18
*描述：main头文件。
* ************************************************************************************************
*/
#ifndef __APPLICATION_H
#define __APPLICATION_H

#ifdef __cplusplus
    extern "C" {
#endif

/*任务优先级定义*/
/*优先级0为RT timer使用,优先级1保留.*/
#define RT_CUS_THREAD_PRIORITY      1
#define INIT_THREAD_PRIINIT         RT_CUS_THREAD_PRIORITY + 2
#define WATCHDOG_THREAD_PRIINIT     RT_CUS_THREAD_PRIORITY + 1
#define NETWORK_THREAD_PRIINIT      RT_CUS_THREAD_PRIORITY + 3


/*任务堆栈大小*/
#define INIT_THREAD_STACK_SIZE         2048
#define WATCHDOG_THREAD_STACK_SIZE     256
#define NETWORK_THREAD_STACK_SIZE      4096


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__APPLICATION_H*/
