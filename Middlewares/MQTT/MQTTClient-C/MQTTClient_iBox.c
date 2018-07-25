/*
**************************************************************************************************
*文件：MQTTClient_iBox.c
*作者：fanwenl_
*版本：V0.0.1
*日期：2018-07-25
*描述：适用于iBox的硬件的MQTT客户端,基于network网络层定制(参考RTT的MQTTClient和paho的MQTTClinet)。
* ************************************************************************************************
*/
#include "MQTTClient_iBox.h"
#include "network.h"
#include "ibox_sys.h"

#include <stdio.h>
#include <string.h>

void timeout_init(timeout_t *timeout, uint32_t count)
{
    timeout->timeout_tick = get_sys_time_ms() + count;
}
void timeout_conundown(timeout_t *timeout, uint32_t count)
{
    timeout->timeout_tick += count;
}

uint8_t is_timeout(timeout_t *timeout)
{
    long left_time = timeout->timeout_tick - get_sys_time_ms();
    return (left_time < 0)? 0 : left_time;
}

static void NewMessageData(MessageData* md, MQTTString* aTopicName, MQTTMessage* aMessage) 
{
    md->topicName = aTopicName;
    md->message = aMessage;
}


static int getNextPacketId(MQTTClient *c) {
    return c->next_packetid = (c->next_packetid == MAX_PACKET_ID) ? 1 : c->next_packetid + 1;
}

/** 发送数据包
 * @arg length 写入数据的长度最大1460字节
 * @arg *c 客户端指针
 * @return 写入的结果。
 */ 
static int sendPacket(MQTTClient *c, int length)
{
    int rc = MQTT_FAILURE;
    
    if(net_tx_write(c->buf, length))
        rc = MQTT_SUCCESS; 
    return rc;
}
/**读取Packet
 * @arg *c MQTT客户端
 * @return 返回packet type
 */ 
static int readPacket(MQTTClient* c)
{
    return MQTTPacket_read(c->readbuf, c->readbuf_size, &net_fifo_read);
}

/**客户端初始化
 * 在应用程序中初始化
 */ 
// void MQTTClientInit(MQTTClient* c, unsigned int command_timeout_ms,
// 		unsigned char* sendbuf, size_t sendbuf_size, unsigned char* readbuf, size_t readbuf_size)
// {
//     int i;

//     for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
//         c->messageHandlers[i].topicFilter = 0;
//     c->command_timeout_ms = command_timeout_ms;
//     c->buf = sendbuf;
//     c->buf_size = sendbuf_size;
//     c->readbuf = readbuf;
//     c->readbuf_size = readbuf_size;
//     c->isconnected = 0;
//     c->cleansession = 0;
//     c->ping_outstanding = 0;
//     c->defaultMessageHandler = NULL;
// 	c->next_packetid = 1;
// }

// assume topic filter and name is in correct format
// # can only be at end
// + and # can only be next to separator
static char isTopicMatched(char* topicFilter, MQTTString* topicName)
{
    char* curf = topicFilter;
    char* curn = topicName->lenstring.data;
    char* curn_end = curn + topicName->lenstring.len;

    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}

/**传输消息
 */ 
int deliverMessage(MQTTClient* c, MQTTString* topicName, MQTTMessage* message)
{
    int i;
    int rc = MQTT_FAILURE;

    // we have to find the right message handler - indexed by topic
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != 0 && (MQTTPacket_equals(topicName, (char*)c->messageHandlers[i].topicFilter) ||
                isTopicMatched((char*)c->messageHandlers[i].topicFilter, topicName)))
        {
            if (c->messageHandlers[i].callback != NULL)
            {
                MessageData md;
                NewMessageData(&md, topicName, message);
                c->messageHandlers[i].callback(c, &md);
                rc = MQTT_SUCCESS;
            }
        }
    }

    if (rc == MQTT_FAILURE && c->defaultMessageHandler != NULL)
    {
        MessageData md;
        NewMessageData(&md, topicName, message);
        c->defaultMessageHandler(c, &md);
        rc = MQTT_SUCCESS;
    }

    return rc;
}


// int keepalive(MQTTClient* c)
// {
//     int rc = MQTT_SUCCESS;

//     if (c->keepAliveInterval == 0)
//         goto exit;

//     if (TimerIsExpired(&c->last_sent) || TimerIsExpired(&c->last_received))
//     {
//         if (c->ping_outstanding)
//             rc = MQTT_FAILURE; /* PINGRESP not received in keepalive interval */
//         else
//         {
//             Timer timer;
//             TimerInit(&timer);
//             TimerCountdownMS(&timer, 1000);
//             int len = MQTTSerialize_pingreq(c->buf, c->buf_size);
//             if (len > 0 && (rc = sendPacket(c, len, &timer)) == MQTT_SUCCESS) // send the ping packet
//                 c->ping_outstanding = 1;
//         }
//     }

// exit:
//     return rc;
// }

void MQTTCleanSession(MQTTClient* c)
{
    int i = 0;

    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
        c->messageHandlers[i].topicFilter = NULL;
}


void MQTTCloseSession(MQTTClient* c)
{
    c->ping_outstanding = 0;
    c->isconnected = 0;
    if (c->cleansession)
        MQTTCleanSession(c);
}


int MQTT_cycle(MQTTClient* c)
{
    int len = 0,
        rc = MQTT_SUCCESS;

    int packet_type = readPacket(c);     /* read the socket, see what work is due */

    switch (packet_type)
    {
        default:
            /* no more data to read, unrecoverable. Or read packet fails due to unexpected network error */
            rc = packet_type;
            goto exit;
        case 0: /* timed out reading packet */
            break;
        case CONNACK:
        case PUBACK:
        case SUBACK:
        case UNSUBACK:
            break;
        case PUBLISH:
        {
            MQTTString topicName;
            MQTTMessage msg;
            int intQoS;
            msg.payloadlen = 0; /* this is a size_t, but deserialize publish sets this as int */
            if (MQTTDeserialize_publish(&msg.dup, &intQoS, &msg.retained, &msg.id, &topicName,
               (unsigned char**)&msg.payload, (int*)&msg.payloadlen, c->readbuf, c->readbuf_size) != 1)
                goto exit;
            msg.qos = (enum QoS)intQoS;
            deliverMessage(c, &topicName, &msg);
            if (msg.qos != QOS0)
            {
                if (msg.qos == QOS1)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBACK, 0, msg.id);
                else if (msg.qos == QOS2)
                    len = MQTTSerialize_ack(c->buf, c->buf_size, PUBREC, 0, msg.id);
                if (len <= 0)
                    rc = MQTT_FAILURE;
                else
                    rc = sendPacket(c, len);
                if (rc == MQTT_FAILURE)
                    goto exit; // there was a problem
            }
            break;
        }
        case PUBREC:
        case PUBREL:
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = MQTT_FAILURE;
            else if ((len = MQTTSerialize_ack(c->buf, c->buf_size,
                (packet_type == PUBREC) ? PUBREL : PUBCOMP, 0, mypacketid)) <= 0)
                rc = MQTT_FAILURE;
            else if ((rc = sendPacket(c, len)) != MQTT_SUCCESS) // send the PUBREL packet
                rc = MQTT_FAILURE; // there was a problem
            if (rc == MQTT_FAILURE)
                goto exit; // there was a problem
            break;
        }

        case PUBCOMP:
            break;
        case PINGRESP:
            c->ping_outstanding = 0;
            break;
    }

    // if (keepalive(c) != MQTT_SUCCESS) {
    //     //check only keepalive MQTT_FAILURE status so that previous MQTT_FAILURE status can be considered as FAULT
    //     rc = MQTT_FAILURE;
    // }

exit:
    if (rc == MQTT_SUCCESS)
        rc = packet_type;
    else if (c->isconnected)
        MQTTCloseSession(c);
    return rc;
}


// int MQTTYield(MQTTClient* c, int timeout_ms)
// {
//     int rc = MQTT_SUCCESS;
//     Timer timer;

//     TimerInit(&timer);
//     TimerCountdownMS(&timer, timeout_ms);

// 	  do
//     {
//         if (cycle(c, &timer) < 0)
//         {
//             rc = MQTT_FAILURE;
//             break;
//         }
//   	} while (!TimerIsExpired(&timer));

//     return rc;
// }

int MQTTIsConnected(MQTTClient* client)
{
  return client->isconnected;
}

// void MQTTRun(void* parm)
// {
// 	Timer timer;
// 	MQTTClient* c = (MQTTClient*)parm;

// 	TimerInit(&timer);

// 	while (1)
// 	{
// #if defined(MQTT_TASK)
// 		MutexLock(&c->mutex);
// #endif
// 		TimerCountdownMS(&timer, 500); /* Don't wait too long if no traffic is incoming */
// 		cycle(c, &timer);
// #if defined(MQTT_TASK)
// 		MutexUnlock(&c->mutex);
// #endif
// 	}
// }




int waitfor(MQTTClient* c, int packet_type, timeout_t *timeout)
{
    int rc = MQTT_FAILURE;

    do
    {
        rc = readPacket(c);
    }
    while (rc != packet_type && is_timeout(timeout));

    return rc;
}

int MQTTConnectWithResults(MQTTClient* c, MQTTPacket_connectData* options, MQTTConnackData* data)
{
    int rc = MQTT_FAILURE;
    MQTTPacket_connectData default_options = MQTTPacket_connectData_initializer;
    int len = 0;

    timeout_t timeout;

	if (c->isconnected) /* don't send connect packet again if we are already connected */
		  goto exit;

    if (options == 0)
        options = &default_options; /* set default options if none were supplied */

    c->keepAliveInterval = options->keepAliveInterval;
    c->cleansession = options->cleansession;

    if ((len = MQTTSerialize_connect(c->buf, c->buf_size, options)) <= 0)
        goto exit;

    if ((rc = sendPacket(c, len)) != MQTT_SUCCESS)  // send the connect packet
        goto exit; // there was a problem

    timeout_init(&timeout, 500);
    // this will be a blocking call, wait for the connack
    if (waitfor(c, CONNACK, &timeout) == CONNACK)
    {
        data->rc = 0;
        data->sessionPresent = 0;
        if (MQTTDeserialize_connack(&data->sessionPresent, &data->rc, c->readbuf, c->readbuf_size) == 1)
            rc = data->rc;
        else
            rc = MQTT_FAILURE;
    }
    else
        rc = MQTT_FAILURE;

exit:
    if (rc == MQTT_SUCCESS)
    {
        c->isconnected = 1;
        c->ping_outstanding = 0;
    }

    return rc;
}


int MQTTConnect(MQTTClient* c)
{
    MQTTConnackData data;

    MQTTPacket_connectData *options = &c->condata; 

    return MQTTConnectWithResults(c, options, &data);
}


int MQTTSetMessageHandler(MQTTClient* c, const char* topicFilter, messageHandler messageHandler)
{
    int rc = MQTT_FAILURE;
    int i = -1;

    /* first check for an existing matching slot */
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
    {
        if (c->messageHandlers[i].topicFilter != NULL && strcmp(c->messageHandlers[i].topicFilter, topicFilter) == 0)
        {
            if (messageHandler == NULL) /* remove existing */
            {
                c->messageHandlers[i].topicFilter = NULL;
                c->messageHandlers[i].callback = NULL;
            }
            rc = MQTT_SUCCESS; /* return i when adding new subscription */
            break;
        }
    }
    /* if no existing, look for empty slot (unless we are removing) */
    if (messageHandler != NULL) {
        if (rc == MQTT_FAILURE)
        {
            for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i)
            {
                if (c->messageHandlers[i].topicFilter == NULL)
                {
                    rc = MQTT_SUCCESS;
                    break;
                }
            }
        }
        if (i < MAX_MESSAGE_HANDLERS)
        {
            c->messageHandlers[i].topicFilter = topicFilter;
            c->messageHandlers[i].callback = messageHandler;
        }
    }
    return rc;
}


int MQTTSubscribeWithResults(MQTTClient* c, const char* topicFilter, enum QoS qos,
       messageHandler messageHandler, MQTTSubackData* data)
{
    int rc = MQTT_FAILURE;
    timeout_t timeout;
    int len = 0;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;

	if (!c->isconnected)
		    goto exit;

    len = MQTTSerialize_subscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic, (int*)&qos);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len)) != MQTT_SUCCESS) // send the subscribe packet
        goto exit;             // there was a problem

    timeout_init(&timeout, 500);

    if (waitfor(c, SUBACK, &timeout) == SUBACK)      // wait for suback
    {
        int count = 0;
        unsigned short mypacketid;
        data->grantedQoS = QOS0;
        if (MQTTDeserialize_suback(&mypacketid, 1, &count, (int*)&data->grantedQoS, c->readbuf, c->readbuf_size) == 1)
        {
            if (data->grantedQoS != 0x80)
                rc = MQTTSetMessageHandler(c, topicFilter, messageHandler);
        }
    }
    else
        rc = MQTT_FAILURE;

exit:
    if (rc == MQTT_FAILURE)
        MQTTCloseSession(c);

    return rc;
}


int MQTTSubscribe(MQTTClient* c, const char* topicFilter, enum QoS qos,
       messageHandler messageHandler)
{
    MQTTSubackData data;
    return MQTTSubscribeWithResults(c, topicFilter, qos, messageHandler, &data);
}


int MQTTUnsubscribe(MQTTClient* c, const char* topicFilter)
{
    int rc = MQTT_FAILURE;
    timeout_t timeout;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicFilter;
    int len = 0;

	if (!c->isconnected)
		  goto exit;

    if ((len = MQTTSerialize_unsubscribe(c->buf, c->buf_size, 0, getNextPacketId(c), 1, &topic)) <= 0)
        goto exit;
    if ((rc = sendPacket(c, len)) != MQTT_SUCCESS) // send the subscribe packet
        goto exit; // there was a problem

    timeout_init(&timeout, c->command_timeout_ms);
    if (waitfor(c, UNSUBACK, &timeout) == UNSUBACK)
    {
        unsigned short mypacketid;  // should be the same as the packetid above
        if (MQTTDeserialize_unsuback(&mypacketid, c->readbuf, c->readbuf_size) == 1)
        {
            /* remove the subscription message handler associated with this topic, if there is one */
            MQTTSetMessageHandler(c, topicFilter, NULL);
        }
    }
    else
        rc = MQTT_FAILURE;

exit:
    if (rc == MQTT_FAILURE)
        MQTTCloseSession(c);

    return rc;
}


int MQTTPublish(MQTTClient* c, const char* topicName, MQTTMessage* message)
{
    int rc = MQTT_FAILURE;
    timeout_t timeout;
    MQTTString topic = MQTTString_initializer;
    topic.cstring = (char *)topicName;
    int len = 0;

	  if (!c->isconnected)
		    goto exit;


    if (message->qos == QOS1 || message->qos == QOS2)
        message->id = getNextPacketId(c);

    len = MQTTSerialize_publish(c->buf, c->buf_size, 0, message->qos, message->retained, message->id,
              topic, (unsigned char*)message->payload, message->payloadlen);
    if (len <= 0)
        goto exit;
    if ((rc = sendPacket(c, len)) != MQTT_SUCCESS) // send the subscribe packet
        goto exit; // there was a problem
    timeout_init(&timeout, 500);
    if (message->qos == QOS1)
    {
        if (waitfor(c, PUBACK, &timeout) == PUBACK)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = MQTT_FAILURE;
        }
        else
            rc = MQTT_FAILURE;
    }
    else if (message->qos == QOS2)
    {
        if (waitfor(c, PUBCOMP, &timeout) == PUBCOMP)
        {
            unsigned short mypacketid;
            unsigned char dup, type;
            if (MQTTDeserialize_ack(&type, &dup, &mypacketid, c->readbuf, c->readbuf_size) != 1)
                rc = MQTT_FAILURE;
        }
        else
            rc = MQTT_FAILURE;
    }

exit:
    if (rc == MQTT_FAILURE)
        MQTTCloseSession(c);
    return rc;
}


int MQTTDisconnect(MQTTClient* c)
{
    int rc = MQTT_FAILURE;
    int len = 0;

	len = MQTTSerialize_disconnect(c->buf, c->buf_size);
    if (len > 0)
        rc = sendPacket(c, len);            // send the disconnect packet
    MQTTCloseSession(c);
    return rc;
}
