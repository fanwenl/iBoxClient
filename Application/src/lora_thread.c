/*
**************************************************************************************************
*文件：lora_thread.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-07-18
*描述：ibox lora 任务文件。
* ************************************************************************************************
*/
#include "ibox_board.h"
#include "lora_thread.h"

uint8_t is_lora_link_ok = 0;

uint8_t lora_rx_buf[LORA_RX_SIZE];
uint16_t lora_rx_len = 0;

/**
 * @brief lora任务函数 
 * 
 * @param parameter 
 */
void lora_thread_entry(void *parameter)
{
    int lora_status = 0;
    char rssi[10] = {0};

    sprintf(rssi, "%f", SX1276ReadRssi());
    ibox_printf(1,("lora rssi is %s dBm\r\n", rssi));

    while(1)
    {   
        /*[RF_IDLE, RF_BUSY, RF_RX_DONE, RF_RX_TIMEOUT,RF_TX_DONE, RF_TX_TIMEOUT]*/
        lora_status = SX1276Process();
        ibox_printf(1, ("[LORA] status %d\r\n", lora_status));
        if(lora_status == RF_IDLE)
        {
            ibox_printf(1, ("[LORA]lora idle\r\n"));
        }
        else if (lora_status == RF_RX_DONE)
        {
            SX1276GetRxPacket(lora_rx_buf, &lora_rx_len);
            /*数据处理帧解析*/
        }
        else if (lora_status == RF_TX_DONE)
        {
            
        }

        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
    
}
