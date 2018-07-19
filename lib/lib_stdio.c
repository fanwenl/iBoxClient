/*
**************************************************************************************************
*文件：lib_printf.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-05-30
*描述：printf函数的在不同编译器的实现GCC、ARMCC
* ************************************************************************************************
*/

#include <stdint.h>
#include <stdio.h>
#include "stm32f10x.h"

/*
 * Arm Compiler
 */
#if   defined ( __CC_ARM ) || ((__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))

#pragma import(__use_no_semihosting)  

//标准库需要的支持函数                   
struct __FILE  
{  
    int handle;  

};  
FILE __stdout;  

//定义_sys_exit()以避免使用半主机模式      
void _sys_exit(int x)  
{  
    x = x;  
} 
void _ttywrch(int ch)
{
    ch = ch;
}

//重映射fputc函数，此函数为多个输出函数的基础函数
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (unsigned char) ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
    }
    return (ch);
}

/*
 * GNU Compiler
 */
#elif defined ( __GNUC__ )

int _write(int fd, char *ptr, int len)
{
    int i = 0;

    /*
     * write "len" of char from "ptr" to file id "fd"
     * Return number of char written.
     *
    * Only work for STDOUT, STDIN, and STDERR
     */
    if (fd > 2)
    {
        return -1;
    }

    while (*ptr && (i < len))
    {
        USART_SendData(USART1, *ptr);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
        }

        if (*ptr == '\n')
        {
            USART_SendData(USART1, '\r');
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
            }
        }

        i++;
        ptr++;
    }

    return i;
}
#endif
