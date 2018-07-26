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

/*一个上报的topic,订阅两个topic,一个下发配置(含RTC),一个执行动作*/
#define MQTT_WILLSUBTOPIC           "/mqtt/will"
#define MQTT_WILLPUBTOPIC           "/mqtt/will"
#define MQTT_WILLMSG                "Goodbye!"

#define MQTT_MSGPUBTOPIC            "mqtt/msg"
#define MQTT_DOSUBTOPIC             "mqtt/action"             
#define MQTT_CIGSUBTOPIC            "mqtt/config"

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
    // *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    // LOG_D("mqtt sub callback: %.*s %.*s",
    //            msg_data->topicName->lenstring.len,
    //            msg_data->topicName->lenstring.data,
    //            msg_data->message->payloadlen,
    //            (char *)msg_data->message->payload);

    // return;
}
static void mqtt_action_callback(MQTTClient *c, MessageData *msg_data)
{
    // *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    // LOG_D("mqtt sub callback: %.*s %.*s",
    //            msg_data->topicName->lenstring.len,
    //            msg_data->topicName->lenstring.data,
    //            msg_data->message->payloadlen,
    //            (char *)msg_data->message->payload);

    // return;
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
    /* init condata param by using MQTTPacket_connectData_initializer */
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;

    /* config connect param */
    memcpy(&client.condata, &condata, sizeof(condata));

    client.condata.clientID.cstring  = DEVICE_NAME(ibox_config.device_sn);
    client.condata.keepAliveInterval = 60;      //单位s
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

    if (cJSON_AddNumberToObject(root, "device_sn", 0000001) == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(root, "imei", 885869896892) == NULL)
    {
        goto end;
    }
    if (cJSON_AddStringToObject(root, "eth_mac", "0:0:0:0") == NULL)
    {
        goto end;
    }
    if (cJSON_AddStringToObject(root, "wifi_mac", "1:0:0:0") == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(root, "rtc", 15000399) == NULL)
    {
        goto end;
    }

    cJSON *data = cJSON_AddArrayToObject(root, "data");
    if (data == NULL)
    {
        goto end;
    }

    cJSON *adc = cJSON_CreateObject();
    if (cJSON_AddNumberToObject(adc, "temper", 440) == NULL)
    {
        goto end;
    }
    if (cJSON_AddNumberToObject(adc, "adc1", 440) == NULL)
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
void MQTT_msg_publish(void)
{
    char *msg_ptr = NULL;

    msg_ptr = create_msg();

    ibox_printf(1,("%s",msg_ptr));
        //     if (FD_ISSET(c->pub_pipe[0], &readset))
        // {
        //     MQTTMessage *message;
        //     MQTTString topic = MQTTString_initializer;

        //     //LOG_D("pub_sock FD_ISSET");

        //     len = read(c->pub_pipe[0], c->readbuf, c->readbuf_size);

        //     if (len < sizeof(MQTTMessage))
        //     {
        //         c->readbuf[len] = '\0';
        //         LOG_D("pub_sock recv %d byte: %s", len, c->readbuf);

        //         if (strcmp((const char *)c->readbuf, "DISCONNECT") == 0)
        //         {
        //             LOG_D("DISCONNECT");
        //             goto _mqtt_disconnect_exit;
        //         }

        //         continue;
        //     }

        //     message = (MQTTMessage *)c->readbuf;
        //     message->payload = c->readbuf + sizeof(MQTTMessage);
        //     topic.cstring = (char *)c->readbuf + sizeof(MQTTMessage) + message->payloadlen;
        //     //LOG_D("pub_sock topic:%s, payloadlen:%d", topic.cstring, message->payloadlen);

        //     len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id,
        //                                 topic, (unsigned char *)message->payload, message->payloadlen);
        //     if (len <= 0)
        //     {
        //         LOG_D("MQTTSerialize_publish len: %d", len);
        //         goto _mqtt_disconnect;
        //     }

        //     if ((rc = sendPacket(c, len)) != PAHO_SUCCESS) // send the subscribe packet
        //     {
        //         LOG_D("MQTTSerialize_publish sendPacket rc: %d", rc);
        //         goto _mqtt_disconnect;
        //     }
        // } /* pbulish sock handler. */
}

void main_thread_entry(void *parameter)
{
    rt_uint32_t opt = 0;
    int ret = -1;
    
    MQTT_init();

    ret = MQTTConnect(&client);
    if(ret != MQTT_SUCCESS)
    {
        ibox_printf(1, ("MQTT Connect Faild!\r\n"));
    }
    
    /*订阅*/
    ret = MQTTSubscribe(&client, MQTT_DOSUBTOPIC, QOS2, mqtt_action_callback);
    if(ret != MQTT_SUCCESS)
    {
        ibox_printf(1, ("MQTT \"%s\" subscribe Faild!\r\n", MQTT_DOSUBTOPIC));
    }    

    ret = MQTTSubscribe(&client, MQTT_CIGSUBTOPIC, QOS1, mqtt_config_callback);
    if(ret != MQTT_SUCCESS)
    {
        ibox_printf(1, ("MQTT \"%s\" subscribe Faild!\r\n", MQTT_CIGSUBTOPIC));
    }
    
    client.tick_ping = get_sys_time_ms();
    while (1) 
    {
        /*发送ping packet*/
        if(((get_sys_time_ms() - client.tick_ping) / RT_TICK_PER_SECOND) >  (client.condata.keepAliveInterval - 10))
        {
            keepalive(&client);
        }
        /*处理收到的topic*/
        if (!is_fifo_empty(&net_rx_fifo))
        {
            ret = MQTT_cycle(&client);
            // if (rc_t < 0)    goto _mqtt_disconnect;

            // continue;
        }

        /*判断时间间隔进行周期性的数据上报*/
        MQTT_msg_publish();

        rt_thread_delay(RT_TICK_PER_SECOND / 2);
    }
}