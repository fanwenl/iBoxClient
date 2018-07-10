/*
**************************************************************************************************
*文件：ibox_gprs.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-06-06
*描述：GPRS接口驱动，SIM800L。
* ************************************************************************************************
*/
#include "ibox_gprs.h"
#include "ibox_uart.h"
#include <string.h>

//Rst WIFI_RTT_B
//URXD_B WIFI_RXB
//TXD_WIFI_TXB
//RING WIFI_GPIOB
//DTR WIFI_CH_PD_B

GPRS_STATUS_ENUM gprs_status = GPRS_STATUS_INIT;
uint8_t gprs_status_error_count = 0; // wifi状态机错误计数器
uint8_t gprs_link_error_count   = 0; // wifi连接错误计数器
uint8_t gprs_error_count        = 0; // wifi错误计数器
uint32_t gprs_timeout           = 0; // wifi连接超时

static void gprs_pin_config(void);
void gprs_init(void)
{
    gprs_pin_config();

    uart_init(UART3_GPRS_WIFI, 115200);

    /*复位GPRS模块*/
    GPRS_RESET_L;
    sys_delay_ms(2);
    GPRS_RESET_H;
    uart3_rx_buf_clear();   
}

static void gprs_pin_config(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(GPRS_RESET_PIN_CLK, ENABLE);

    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStruct.GPIO_Pin   = GPRS_RESET_PIN;

    GPIO_Init(GPRS_RESET_PORT, &GPIO_InitStruct);
}

void gprs_at_fsm(void)
{
  #if 0
    static uint8_t gprs_rx_temp[UART3_RX_SIZE];

    uint16_t gprs_rx_len = 0;


    memset(gprs_rx_temp, 0, UART3_RX_SIZE);
    /*获取一帧数据*/
    gprs_rx_len = get_line_from_uart3(gprs_rx_temp);
    if (gprs_rx_len) {
        ibox_printf(ibox_wifi_debug, ("[GPRS_REC]:%s\r\n", gprs_rx_temp));
    } else {
//        return;
    }

    switch (gprs_status) {
    case GPRS_STATUS_CHECK: //检测模块；ATE0关闭回显
        uart3_rx_buf_clear();
        uart3_send_str("AT\r\n");
        gprs_status_error_count = 0;
        gprs_timeout            = get_sys_time_s();
        gprs_status = GPRS_STATUS_INIT;
        break;
    case GPRS_STATUS_INIT://检查SIM卡
        if (strstr((char *) wifi_rx_temp, "OK") != NULL) {
//            wifi_status_error_count = 0;
//            wifi_timeout            = get_sys_time_s();
//            //ibox_printf(ibox_wifi_debug, ("AT+CWMODE?\r\n"));
            uart3_send_str("AT+CPIN?\r\n"); 
            gprs_status = ESP8266_STATUS_WAIT_CHECK_MODE;
//        } else {
//            if (get_sys_time_s() - wifi_timeout > 2) {
//                wifi_timeout = get_sys_time_s();
//                wifi_status_error_count++;
//                if (wifi_status_error_count > 5) {
//                    wifi_link_error_count++;
//                } else {
//                    uart3_send_str("AT\r\n");
//                }
//            }
//            esp8266_status = ESP8266_STATUS_CHECK_MODE;
//        }
    case GPRS_STATUS_SET_MISC://设置杂项
        if (strstr((char *) wifi_rx_temp, "READY") != NULL) {
     SEND_STR_2_GSM("AT+CIPSHUT\r\n");
      rt_thread_delay(RT_TICK_PER_SECOND);
      SEND_STR_2_GSM("AT+CIPRXGET=1\r\n");//接受数据模式改为主动接收
      rt_thread_delay(RT_TICK_PER_SECOND/5);
      SEND_STR_2_GSM("AT+CIPSPRT=0\r\n");//删掉<号
      rt_thread_delay(RT_TICK_PER_SECOND/5);
      #if GPRS_SLEEP_MODE_DTR
      SEND_STR_2_GSM("AT+CFGRI=1\r\n");//设置振铃模式
      rt_thread_delay(RT_TICK_PER_SECOND/10);
      #endif
      CLEAR_GSM_RECV_BUF(); 
      SEND_STR_2_GSM("AT+CSQ\r\n");
      GSMDebugLog("AT Req -> AT+CSQ");
      gprs_state = GPRS_STATUS_CHECK_CSQ;
      sGprsATTimer = rt_tick_get();
      sGprsErrorCount = 0; 
        break;
    case GPRS_STATUS_CHECK_CSQ://检测信号强度 
         pGprsATHeader = strstr((char *)sGprsATResp, "+CSQ:");
        SEND_STR_2_GSM("AT+CREG?\r\n");//检测当前注册状态 
         GSMDebugLog("AT Req -> AT+CREG?");
         gprs_state = GPRS_STATUS_CHECK_REG;
    case GPRS_STATUS_CHECK_REG:
         pGprsATHeader = strstr((char *)sGprsATResp, "+CREG:");
        if (pGprsATHeader != NULL)
        {
          if ((pGprsATHeader[9] == '1') || (pGprsATHeader[9] == '5'))//注册成功
          {
            GSMDebugLog("CREG OK");  
            sGprsATTimer = rt_tick_get();
            sGprsErrorCount = 0;
            //gprs附着
            CLEAR_GSM_RECV_BUF(); 
            SEND_STR_2_GSM("AT+CGATT?\r\n");
            GSMDebugLog("AT Req -> AT+CGATT?");
            gprs_state = GPRS_STATUS_CHECK_CGATT;
    case GPRS_STATUS_CHECK_CGATT://检测GPS附着
            pGprsATHeader = strstr((char *)sGprsATResp, "+CGATT: 1");
        if (pGprsATHeader != NULL){
                      memset(rec_buff,0,NOR_TMP_BUF_LEN);
          sprintf((char *)rec_buff, "AT+CSTT=\"%s\"\r\n", (char *)bike_state.svr_apn);            
          SEND_STR_2_GSM(rec_buff);
          gprs_state = GPRS_STATUS_SET_APN;
    case  GPRS_STATUS_SET_APN://set apn
         pGprsATHeader = strstr((char *)sGprsATResp, "OK");
        if (pGprsATHeader != NULL){
          sGprsErrorCount = 0;
          sGprsATTimer = rt_tick_get();
          CLEAR_GSM_RECV_BUF();
          SEND_STR_2_GSM("AT+CIICR\r\n");
          GSMDebugLog("AT Req -> AT+CIICR");
          gprs_state = GPRS_STATUS_SET_CIICR; 
    case GPRS_STATUS_SET_CIICR://建立无限链路
     if(mGprsRecvLen){
        pGprsATHeader = NULL;
        pGprsATHeader = strstr((char *)sGprsATResp, "OK");
        if (pGprsATHeader != NULL){
          sGprsErrorCount = 0;
          sGprsATTimer = rt_tick_get();
          CLEAR_GSM_RECV_BUF();
          SEND_STR_2_GSM("AT+CIFSR\r\n");
          GSMDebugLog("AT Req -> AT+CIFSR");
          gprs_state = GPRS_STATUS_GET_IP;  
          break;
        }
        pGprsATHeader = strstr((char *)sGprsATResp, "ERROR");
        if (pGprsATHeader != NULL){
          sGprsATTimer = rt_tick_get();
          gprs_state = GPRS_STATUS_INIT;  
          break;
        }         
      }
      if (rt_tick_get() - sGprsATTimer > RT_TICK_PER_SECOND*10) {
        reg_error_count++;
        sGprsATTimer = rt_tick_get();
        gprs_state =  GPRS_STATUS_INIT; 
      }
      break;
    case GPRS_STATUS_GET_IP://获取IP地址
      if(mGprsRecvLen){
        pGprsATHeader = NULL;
        pGprsATHeader = strstr((char *)sGprsATResp, ".");
        if (pGprsATHeader != NULL){
          GSMDebugLog("Get Local IP OK");  
          gprs_state = GPRS_STATUS_START_TCP;    
          break;
        }
        pGprsATHeader = strstr((char *)sGprsATResp, "ERROR");
        if (pGprsATHeader != NULL){
          sGprsATTimer = rt_tick_get();
          gprs_state = GPRS_STATUS_INIT;  
          break;
        }         
      }
      if (rt_tick_get() - sGprsATTimer > RT_TICK_PER_SECOND*10) {
        reg_error_count++;
        sGprsATTimer = rt_tick_get();
        gprs_state =  GPRS_STATUS_INIT;
      }
      break;
    case GPRS_STATUS_RESTART_TCP:
        GSMDebugLog("TCP Error,restart tcp link");
        tcp_error_count++;
        SEND_STR_2_GSM("AT+CIPCLOSE\r\n");
        rt_thread_delay(RT_TICK_PER_SECOND);
        CLEAR_GSM_RECV_BUF();
        gprs_state = GPRS_STATUS_START_TCP;
        tx_counter = 0;
        sendflag = 0;
        send_timeout_flag = 0;
        break;
    case GPRS_STATUS_START_TCP://锚点
      CLEAR_GSM_RECV_BUF();
      tcpconnect();
      sGprsATTimer = rt_tick_get();
      gprs_state = GPRS_STATUS_TCP_CONNECT;  
      break;
    case GPRS_STATUS_TCP_CONNECT://tcp连接
      if(mGprsRecvLen){
        //成功连接上了
        if (strstr((char *)sGprsATResp, "CONNECT OK") != NULL){
          reg_error_count = 0;
          sGprsATTimer = rt_tick_get();
          GSMDebugLog("SERVER CONNECT OK");                
          gprs_state = GPRS_STATUS_COMMUNICATE;
          SEND_STR_2_GSM("AT+CDNSGIP=lot.zxbike.cc\r\n");
          rt_event_send(&net_thread_event,NET_THREAD_EVENT_GPRS_CONNECT);
          GSMDebugLog("send verify message");
          sendflag = 1;
          break;
        }
        //已经连接上了
        if (strstr((char *)sGprsATResp, "ALREADY CONNECT") != NULL)
        {
          sGprsATTimer = rt_tick_get();
          tcp_error_count++;
          GSMDebugLog("SERVER ALREADY CONNECT, RECONNECT...");                
          gprs_state = GPRS_STATUS_INIT;
          break;                
        }
        //连接失败
        if (strstr((char *)sGprsATResp, "CONNECT FAIL") != NULL)
        {
          sGprsATTimer = rt_tick_get();
          tcp_error_count++;
          GSMDebugLog("SERVER CONNECT FAIL");
          gprs_state = GPRS_STATUS_INIT;
          break;                
        }
      }
      if (rt_tick_get() - sGprsATTimer > RT_TICK_PER_SECOND*10)
      {
        tcp_error_count++;
        GSMDebugLog("SERVER CONNECT timeout");
        gprs_state = GPRS_STATUS_INIT;
        break;
      }
      break;
    case GPRS_STATUS_COMMUNICATE://TCP正常通信的状态
      // GPRS数据上传后在一定时间内占用电源资源,后释放资源
      if((oPowerSemCheck(EBIKE_POWER_GSM) == RT_EOK) && 
         (rt_tick_get() - sGprsATTimer > RT_TICK_PER_SECOND) /*&& send_timeout_flag*/){
        oPowerSemRelease();                 // 释放"_power_sem"资源
      }

      if(mGprsRecvLen){
        if (strstr((char *)sGprsATResp, "CLOSED") != NULL)
        {
          gprs_state = GPRS_STATUS_RESTART_TCP;
          break;
        }
        //失去上下文
        if (strstr((char *)sGprsATResp, "+PDP DEACT") != NULL)
        {
          sGprsATTimer = rt_tick_get();
          GSMDebugLog("GPRS context FaIL, Reactting...");
          gprs_state = GPRS_STATUS_INIT;
          break;
        }
        //获取数据
        if (strstr((char *)sGprsATResp, "+CIPRXGET: 1") != NULL)
        {
          //只有在正常获取数据之后tcp error才会清零
          tcp_error_count = 0;
          //数据处理
          gprs_state = GPRS_STATUS_RX_PROC;
          sGprsATTimer = rt_tick_get();
          SEND_STR_2_GSM("AT+CIPRXGET=3,730\r\n"); 
          break;
        }
        //发送成功
        if (strstr((char*)sGprsATResp, "SEND OK") != NULL)
        {
          sendflag = 1;
          send_timeout_flag = 0;
          tx_counter = 0;
          rt_event_send(&net_thread_event,NET_THREAD_EVENT_GPRS_SEND_OK);
          break;
        }
        //发送失败
        if (strstr((char*)sGprsATResp, "SEND FAIL") != NULL)
        {
          sGprsATTimer = rt_tick_get();
          GSMDebugLog("Send Fail");
          gprs_state = GPRS_STATUS_INIT;
          sendflag = 1;
          send_timeout_flag = 0;
          break;                                
        }
        //出错了 
        if (strstr((char*)sGprsATResp, "ERROR") != NULL)//ERROR
        {
          gprs_state = GPRS_STATUS_RESTART_TCP;
          break;                                
        }
      }
      //发送数据
      if (tx_counter && sendflag)           // 想要发送数据?需要等待其他任务是空闲的;
      {
        if(oPowerSemTake(EBIKE_POWER_GSM,RT_WAITING_NO) == RT_EOK){ // @T.Y 上电?需要等待其他任务是空闲的
          sGprsATTimer = rt_tick_get();
          GSMDebugLog("Sending user data...");
          countnum = 0;
          while(countnum != tx_counter)
          {
            EBIKE_DEBUG_LOG(ebike_debug_gprs,("<%02X>", tx_buff[countnum]));
            countnum++;
          }
          EBIKE_DEBUG_LOG(ebike_debug_gprs,("\r\n"));
          send_data();
          sendflag = 0;
          send_timeout_flag = 1;
        }
        break;
      }
      if((rt_tick_get() - sGprsATTimer > RT_TICK_PER_SECOND*10) && send_timeout_flag)
      {
        GSMDebugLog("send time out");
        gprs_state = GPRS_STATUS_RESTART_TCP;
        break;   // 在这里添加中断,主要是这个状态默认不释放资源,除非有特殊情况,就像现在
      }
      if((rt_tick_get() - sGprsATTimer > RT_TICK_PER_SECOND*10) && (send_timeout_flag==0)){
        sGprsATTimer = rt_tick_get();
        sGprsErrorCount = 0;
        #if GPRS_SLEEP_MODE_DTR
        SEND_STR_2_GSM("AT+CSCLK=1\r\n");
        GSMDebugLog("AT Req -> AT+CSCLK=1");
        GPRS_DTR_HIGH;
        #else
        SEND_STR_2_GSM("AT+CSCLK=2\r\n");
        GSMDebugLog("AT Req -> AT+CSCLK=2");
        #endif     
        if(oPowerSemCheck(EBIKE_POWER_GSM) == RT_EOK){
          oPowerSemRelease();                 // 释放"_power_sem"资源
        }
        gprs_state = GPRS_STATUS_ENTER_SLEEP;
        break;   // 在这里添加中断,主要是这个状态默认不释放资源,除非有特殊情况,就像现在
      }
      break;
    case GPRS_STATUS_RX_PROC://接收处理
      pGprsATHeader = NULL;
      pGprsATHeader = strstr((char*)sGprsATResp, "+CIPRXGET: 3");
      if (pGprsATHeader != NULL)
      {
        //
        pGprsATHeader += 13;
        mGprsRxDataLen = 0;
        while (*pGprsATHeader != ',')//
        {
          mGprsRxDataLen = mGprsRxDataLen * 10 + (*pGprsATHeader - 0x30);
          pGprsATHeader++;
        }
        tmplen = mGprsRxDataLen;
        if(mGprsRxDataLen){//
          gprs_state = GPRS_STATUS_FRAME_PROC;
        } else {//
          GSMDebugLog("gprs recv error len , restart tcp!");
          gprs_state = GPRS_STATUS_RESTART_TCP;
        }
        sGprsATTimer = rt_tick_get();
        break;
      }
      if (rt_tick_get() - sGprsATTimer > RT_TICK_PER_SECOND*10) 
      {
        GSMDebugLog("CIPRXGET No Answer!");
        gprs_state = GPRS_STATUS_RESTART_TCP;
        break;
      }
      break;
    case GPRS_STATUS_FRAME_PROC:
      if(mGprsRecvLen){//如果收到了数据
          if(mGprsRecvLen != (2*tmplen))
          {
            gprs_state = GPRS_STATUS_COMMUNICATE;
            GSMDebugLog("gprs receive error num\r\n");
            break;
          }
          for(mCount = 0; mCount < tmplen; mCount++)
          {
            atoi_buf[0]=sGprsATResp[mCount*2];
            atoi_buf[1]=sGprsATResp[mCount*2+1];
            rx_buff[mCount]=strtol((const char *)atoi_buf, NULL, 16);
          }
          EBIKE_DEBUG_LOG(ebike_debug_gprs,("\r\nnewdata is %d\r\n", tmplen));
          gprs_state = GPRS_STATUS_COMMUNICATE;
          net_rx_buf_write(rx_buff,tmplen);
      } else {//需要延迟一段时间
        if(rt_tick_get() - sGprsATTimer > RT_TICK_PER_SECOND*2){
            gprs_state = GPRS_STATUS_COMMUNICATE;
            GSMDebugLog("gprs receive error num\r\n");
        }
      }
      break;
    default:
        break;
    }
    #endif
}
