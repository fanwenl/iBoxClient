/*
**************************************************************************************************
*文件：main_thread.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-07-18
*描述：ibox主业务线程。
* ************************************************************************************************
*/
#include "ibox_board.h"
#include "main_thread.h"
#include "MQTTClient_iBox.h"
#include "cJSON.h"

rt_event_t main_thread_event = RT_NULL;
uint32_t publish_time = 0;

/*一个上报的topic,订阅两个topic,一个下发配置(含RTC),一个执行动作*/
#define MQTT_WILLSUBTOPIC           "/mqtt/will"
#define MQTT_WILLPUBTOPIC           "/mqtt/will"
#define MQTT_WILLMSG                "Goodbye!"

#define MQTT_MSGPUBTOPIC            "mqtt/msg"
#define MQTT_DOALLSUBTOPIC          "mqtt/action_all"     //通用动作订阅
//#define MQTT_DOSUBTOPIC(sn)         "mqtt/action_"#sn""   //本机动作订阅
#define MQTT_CIGSUBTOPIC            "mqtt/config"         //通用配置信息
//#define MQTT_CIGSUBTOPIC(sn)        "mqtt/con_"#sn""      //本机配置

static MQTTClient client;

void main_thread_init(void)
{
    main_thread_event = rt_event_create("MainEvent", RT_IPC_FLAG_FIFO);
    if(main_thread_event == RT_NULL)
    {
        ibox_printf(1, ("main thread event create fail!\r\n"));
    }
}


static void mqtt_config_callback(MQTTClient *c, MessageData *msg_data)
{
    cJSON *root = NULL;

    root = cJSON_Parse(msg_data->message->payload);
    if(!root)
    {
        ibox_printf(1, ("config cJSON Parse fail!\r\n"));
    }

    cJSON *rtc = cJSON_GetObjectItem(root, "rtc");
    if(!rtc)
    {
        RTC_SetCounter((uint32_t) rtc->valueint);
        ibox_printf(1, ("set RTC counter:%d\r\n", rtc->valueint));
    }

    if(root)
    {
        cJSON_Delete(root);
    }
}
static void mqtt_action_callback(MQTTClient *c, MessageData *msg_data)
{
    cJSON *root = NULL;

    root = cJSON_Parse(msg_data->message->payload);
    if(!root)
    {
        ibox_printf(1, ("action cJSON Parse fail!\r\n"));
    }

    cJSON *dac = cJSON_GetObjectItem(root, "dac");
    if(!dac)
    {
        dac_set_vol((float)dac->valuedouble);
        ibox_printf(1, ("set DAC value:%lf\r\n", dac->valuedouble));
    }

    if(root)
    {
        cJSON_Delete(root);
    }
}

static void mqtt_sub_default_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    ibox_printf(1, ("mqtt sub default callback: %.*s %.*s",
               msg_data->topicName->lenstring.len,
               msg_data->topicName->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload));
    return;
}

static void MQTT_init(void)
{
    /*要使用指针，定义为static类型*/
    static char client_id[23];
    /* init condata param by using MQTTPacket_connectData_initializer */
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;

    /* config connect param */
    memcpy(&client.condata, &condata, sizeof(condata));
    
//    sprintf(client_id,"iBoxClient-%010d",ibox_config.device_sn);   
    sprintf(client_id,"iBoxClient-%010d",get_sys_time_s());

    client.condata.clientID.cstring  = client_id;
    client.condata.keepAliveInterval = 300;      //单位s
    client.condata.cleansession      = 1;       //清理会话
    client.condata.username.cstring  = ibox_config.mqtt_username;
    client.condata.password.cstring  = ibox_config.mqtt_password;

    /* config MQTT will param. */
    client.condata.willFlag               = 1;
    client.condata.will.qos               = 1;      //至少发一次
    client.condata.will.retained          = 0;
    client.condata.will.topicName.cstring = MQTT_WILLPUBTOPIC;
    client.condata.will.message.cstring   = MQTT_WILLMSG;

    /* malloc buffer. */
    client.buf_size = client.readbuf_size = 1024;
    client.buf                            = rt_malloc(client.buf_size);
    client.readbuf                        = rt_malloc(client.readbuf_size);
    if (!(client.buf && client.readbuf))
    {
        ibox_printf(1, ("MQTT init failed <no mem>!\r\n"));
        return;
    }

    /* set event callback function */
    client.connect_callback = NULL;
    client.online_callback  = NULL;
    client.offline_callback = NULL;

    /* set subscribe table and event callback */
    // client.messageHandlers[0].topicFilter = MQTT_SUBTOPIC;
    // client.messageHandlers[0].callback    = mqtt_sub_callback;
    // client.messageHandlers[0].qos         = QOS1;

    /* set default subscribe event callback */
    client.defaultMessageHandler = mqtt_sub_default_callback;
}
/**创建msg
 * 
*/
char *create_msg(void)
{
    char *string = NULL;

    cJSON *root = cJSON_CreateObject();

    if (cJSON_AddNumberToObject(root, "device_sn", ibox_config.device_sn) == NULL)
    {
        goto end;
    }
    if (cJSON_AddStringToObject(root, "eth_mac", ibox_config.eth_mac) == NULL)
    {
        goto end;
    }
#ifdef USE_WIFI
    if (cJSON_AddStringToObject(root, "wifi_mac", ibox_config.wifi_mac) == NULL)
    {
        goto end;
    }
#else
    if (cJSON_AddNumberToObject(root, "imei", ibox_config.gprs_imei) == NULL)
    {
        goto end;
    }
#endif
    if (cJSON_AddNumberToObject(root, "rtc", get_sys_time_s()) == NULL)
    {
        goto end;
    }

    cJSON *data = cJSON_AddArrayToObject(root, "data");
    if (data == NULL)
    {
        goto end;
    }

    cJSON *adc = cJSON_CreateObject();
    if (cJSON_AddNumberToObject(adc, "temper", get_cpu_temperature()) == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(adc, "adc1", get_adc_voltage(ADC1_INDEX)) == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(adc, "adc2", get_adc_voltage(ADC2_INDEX)) == NULL)
    {
        goto end;
    }
    cJSON_AddItemToArray(data, adc);

    cJSON *rs485 = cJSON_CreateObject();
    if (cJSON_AddNumberToObject(rs485, "485", 440) == NULL)
    {
        goto end;
    }
    cJSON_AddItemToArray(data, rs485);

    cJSON *lora = cJSON_CreateObject();
    if (cJSON_AddNumberToObject(lora, "lora1", 440) == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(lora, "lora2", 440) == NULL)
    {
        goto end;
    }
    cJSON_AddItemToArray(data, lora);


    string = cJSON_Print(root);
    if (string == NULL) {
        fprintf(stderr, "Failed to print monitor.\n");
    }

end:
    cJSON_Delete(root);
    return string;
}
/**
 * ibox 信息打包上传
 */ 
int MQTT_msg_publish(void)
{
    char *msg_ptr = NULL;
    uint16_t msg_len = 0;
    int rc = -1;
    MQTTMessage message;

    msg_ptr = create_msg();
    if(msg_ptr != NULL)
    {
        msg_len = strlen(msg_ptr);

        message.dup = 0; // 0第一次发送
        message.id  = (unsigned short) get_sys_time_s();

        message.retained   = 0; // 0不保留消息
        message.qos        = QOS1;
        message.payload    = msg_ptr;
        message.payloadlen = msg_len;

        rc = MQTTPublish(&client, MQTT_MSGPUBTOPIC, &message);

        rt_free(msg_ptr);

    }
    else
    {
        ibox_printf(1, ("msg create fail!\r\n"));
    }

    return rc;
}

void main_thread_entry(void *parameter)
{
    rt_uint32_t opt = 0;
    int ret = -1;
    char sub_temp[30];
    
    MQTT_init();

_mqtt_start:
    ret = MQTTConnect(&client);
    if(ret != MQTT_SUCCESS)
    {
        ibox_printf(1, ("MQTT Connect Faild!\r\n"));
        goto _mqtt_restart;
    }

    /*订阅统一动作主题*/
    ret = MQTTSubscribe(&client, MQTT_DOALLSUBTOPIC, QOS2, mqtt_action_callback);
    if(ret != MQTT_SUCCESS)
    {
        ibox_printf(1, ("MQTT \"%s\" subscribe Faild!\r\n", MQTT_DOALLSUBTOPIC));
        goto _mqtt_disconnect;
    }
    /*订阅本机动作主题*/
    sprintf(sub_temp,"mqtt/action-%010ld", ibox_config.device_sn);
    ret = MQTTSubscribe(&client, sub_temp, QOS2, mqtt_action_callback);
    if(ret != MQTT_SUCCESS)
    {
        ibox_printf(1, ("MQTT \"%s\" subscribe Faild!\r\n", sub_temp));
        goto _mqtt_disconnect;
    }        
    //订阅通用配置
    ret = MQTTSubscribe(&client, MQTT_CIGSUBTOPIC, QOS1, mqtt_config_callback);
    if(ret != MQTT_SUCCESS)
    {
        ibox_printf(1, ("MQTT \"%s\" subscribe Faild!\r\n", MQTT_CIGSUBTOPIC));
        goto _mqtt_disconnect;
    }
    //订阅本机配置
    sprintf(sub_temp,"mqtt/con-%010ld", ibox_config.device_sn);
    ret = MQTTSubscribe(&client, sub_temp, QOS1, mqtt_config_callback);
    if(ret != MQTT_SUCCESS)
    {
        ibox_printf(1, ("MQTT \"%s\" subscribe Faild!\r\n", sub_temp));
        goto _mqtt_disconnect;
    }

    client.tick_ping = get_sys_time_ms();
    while (1) 
    {
        /*发送ping packet*/
        if(((get_sys_time_ms() - client.tick_ping) / RT_TICK_PER_SECOND) >  (client.condata.keepAliveInterval - 10))
        {
            ret = keepalive(&client);
            if (ret != MQTT_SUCCESS)
            {
                ibox_printf(1, ("MQTT ping Faild!\r\n"));
                goto _mqtt_disconnect;
            }
            else
            {
                client.tick_ping = get_sys_time_s();
            }
        }
        /*处理收到的topic*/
        ret = MQTT_cycle(&client);
        if (ret < 0)
            goto _mqtt_disconnect;

        /*判断时间间隔进行周期性的数据上报*/
        if((get_sys_time_s() - publish_time) > ibox_config.period * 60)
        {
            ret = MQTT_msg_publish();
            if (ret != MQTT_SUCCESS)
            {
                ibox_printf(1, ("MQTT publish Faild!\r\n"));
                goto _mqtt_disconnect;
            }
            else
            {
                publish_time = get_sys_time_s();
            }
        }

        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }

_mqtt_disconnect:
    MQTTDisconnect(&client);

_mqtt_restart:
    rt_thread_delay(RT_TICK_PER_SECOND * 5);
    ibox_printf(1, ("mqtt restart!\n"));
    goto _mqtt_start;
}