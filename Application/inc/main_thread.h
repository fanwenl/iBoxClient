/*
**************************************************************************************************
*文件：main_thread.h
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-07-19
*描述：ibox 主业务文件。
* ************************************************************************************************
*/

#include <stdint.h>
#include <stdio.h>

#define MAIN_RECEIVE_DATA_EVENT              (1u << 0)

#define MAIN_THREAD_EVENT_ALL            MAIN_RECEIVE_DATA_EVENT

extern rt_event_t main_thread_event;

void main_thread_init(void);
void main_thread_entry(void *parameter);

