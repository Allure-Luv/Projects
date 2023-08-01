#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <board.h>

#include <rtthread.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME    "mqtt.sample"
#define DBG_LEVEL           DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include "paho_mqtt.h"
#include "key.h"

#define MQTT_URI                "tcp://mqtt.ditexi.com:1883"
#define MQTT_USERNAME           "admin"
#define MQTT_PASSWORD           "dmaBssnzn770Od"
#define MQTT_SUBTOPIC           "rtt-sub"
#define MQTT_PUBTOPIC           "rtt-pub"
#define MQTT_WILLMSG            "Goodbye!"

#define LED1_PIN                GET_PIN(F, 10)
#define BEEP_PIN                GET_PIN(F, 8)

static MQTTClient client;
static int is_started = 0;

static void mqtt_sub_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("mqtt sub callback: %.*s %.*s",
               msg_data->topicName->lenstring.len,
               msg_data->topicName->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload);
    if (!strcmp((char *)msg_data->message->payload, "LED1 ON"))
    {
        rt_pin_write(LED1_PIN, PIN_LOW);
    }
    if (!strcmp((char *)msg_data->message->payload, "LED1 OFF"))
    {
        rt_pin_write(LED1_PIN, PIN_HIGH);
    }
    if (!strcmp((char *)msg_data->message->payload, "BEEP OFF"))
    {
        rt_pin_write(BEEP_PIN, PIN_LOW);
    }
    if (!strcmp((char *)msg_data->message->payload, "BEEP ON"))
    {
        rt_pin_write(BEEP_PIN, PIN_HIGH);
    }
}

static void mqtt_sub_default_callback(MQTTClient *c, MessageData *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("mqtt sub default callback: %.*s %.*s",
               msg_data->topicName->lenstring.len,
               msg_data->topicName->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload);
}

static void mqtt_connect_callback(MQTTClient *c)
{
    LOG_D("inter mqtt_connect_callback!");
}

static void mqtt_online_callback(MQTTClient *c)
{
    LOG_D("inter mqtt_online_callback!");
}

static void mqtt_offline_callback(MQTTClient *c)
{
    LOG_D("inter mqtt_offline_callback!");
}

static void mqtt_start(void *parameter)
{
    MQTTPacket_connectData condata = MQTTPacket_connectData_initializer;
    static char cid[20] = { 0 };

    if (is_started)
    {
        LOG_E("mqtt client is already connected.");
        return;
    }

    /* client config */
    client.isconnected = 0;
    client.uri = MQTT_URI;

    /* generate random client ID */
    rt_snprintf(cid, sizeof(cid), "rtthread%d", rt_tick_get());

    /* connect config */
    rt_memcpy(&client.condata, &condata, sizeof(condata));
    client.condata.clientID.cstring = cid;
    client.condata.keepAliveInterval = 30;
    client.condata.cleansession = 1;
    client.condata.username.cstring = MQTT_USERNAME;
    client.condata.password.cstring = MQTT_PASSWORD;

    /* will config */
    client.condata.willFlag = 1;
    client.condata.will.qos = 1;
    client.condata.will.retained = 0;
    client.condata.will.topicName.cstring = MQTT_PUBTOPIC;
    client.condata.will.message.cstring = MQTT_WILLMSG;

    /* buffer config */
    client.buf_size = client.readbuf_size = 1024;
    client.buf = rt_calloc(1, client.buf_size);
    client.readbuf = rt_calloc(1, client.readbuf_size);
    if (!(client.buf && client.readbuf))
    {
        LOG_E("no memory for MQTT client buffer!");
        return;
    }

    /* set event callback function */
    client.connect_callback = mqtt_connect_callback;
    client.online_callback = mqtt_online_callback;
    client.offline_callback = mqtt_offline_callback;

    /* set subscribe table and event callback */
    client.messageHandlers[0].topicFilter = rt_strdup(MQTT_SUBTOPIC);
    client.messageHandlers[0].callback = mqtt_sub_callback;
    client.messageHandlers[0].qos = QOS1;

    /* set default subscribe event callback */
    client.defaultMessageHandler = mqtt_sub_default_callback;

    paho_mqtt_start(&client);
    is_started = 1;

    rt_uint8_t keyval = 0;

    while(1)
    {
        keyval = key_scan(0);

        if(keyval)
        {
            switch (keyval)
            {
                case KEY0_PRES:
                    rt_kprintf("key0 pressed\r\n");
                    paho_mqtt_publish(&client, QOS1, MQTT_PUBTOPIC, "KEY0 is pressed");
                    rt_thread_mdelay(10);
                    break;

                case KEY1_PRES:
                    rt_kprintf("key1 pressed\r\n");
                    paho_mqtt_publish(&client, QOS1, MQTT_PUBTOPIC, "KEY1 is pressed");
                    rt_thread_mdelay(10);
                    break;

                case KEY2_PRES:
                    rt_kprintf("key2 pressed\r\n");
                    paho_mqtt_publish(&client, QOS1, MQTT_PUBTOPIC, "KEY2 is pressed");
                    rt_thread_mdelay(10);
                    break;

                case WKUP_PRES:
                    rt_kprintf("key_up pressed\r\n");
                    paho_mqtt_publish(&client, QOS1, MQTT_PUBTOPIC, "KEY_UP is pressed");
                    rt_thread_mdelay(10);
                    break;

                default: break;

            }
        }
        else rt_thread_sleep(10);
    }
}

static int paho_init(void)
{
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(BEEP_PIN, PIN_MODE_OUTPUT);
    rt_pin_write(LED1_PIN, PIN_HIGH);
    rt_pin_write(BEEP_PIN, PIN_LOW);

    key_init();

    rt_err_t ret = RT_EOK;
    rt_thread_t thread = rt_thread_create("paho_t", mqtt_start, RT_NULL, 2048, 10, 10);

    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }
    else
    {
        ret = RT_ERROR;
    }
    return ret;
}

static int mqtt_stop(int argc, char **argv)
{
    if (argc != 1)
    {
        rt_kprintf("mqtt_stop    --stop mqtt worker thread and free mqtt client object.\n");
    }

    is_started = 0;

    return paho_mqtt_stop(&client);
}

static int mqtt_publish(int argc, char **argv)
{
    if (is_started == 0)
    {
        LOG_E("mqtt client is not connected.");
        return -1;
    }

    if (argc == 2)
    {
        paho_mqtt_publish(&client, QOS1, MQTT_PUBTOPIC, argv[1]);
        //rt_kprintf("the result of publish: %x", ret);
    }
    else if (argc == 3)
    {
        paho_mqtt_publish(&client, QOS1, argv[1], argv[2]);
    }
    else
    {
        rt_kprintf("mqtt_publish <topic> [message]  --mqtt publish message to specified topic.\n");
        return -1;
    }

    return 0;
}

static void mqtt_new_sub_callback(MQTTClient *client, MessageData *msg_data)
{
    *((char *)msg_data->message->payload + msg_data->message->payloadlen) = '\0';
    LOG_D("mqtt new subscribe callback: %.*s %.*s",
               msg_data->topicName->lenstring.len,
               msg_data->topicName->lenstring.data,
               msg_data->message->payloadlen,
               (char *)msg_data->message->payload);
}


static int mqtt_subscribe(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("mqtt_subscribe [topic]  --send an mqtt subscribe packet and wait for suback before returning.\n");
        return -1;
    }

    if (is_started == 0)
    {
        LOG_E("mqtt client is not connected.");
        return -1;
    }

    return paho_mqtt_subscribe(&client, QOS1, argv[1], mqtt_new_sub_callback);
}

static int mqtt_unsubscribe(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("mqtt_unsubscribe [topic]  --send an mqtt unsubscribe packet and wait for suback before returning.\n");
        return -1;
    }

    if (is_started == 0)
    {
        LOG_E("mqtt client is not connected.");
        return -1;
    }

    return paho_mqtt_unsubscribe(&client, argv[1]);
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(paho_init, startup mqtt client);
MSH_CMD_EXPORT(mqtt_stop, stop mqtt client);
MSH_CMD_EXPORT(mqtt_publish, mqtt publish message to specified topic);
MSH_CMD_EXPORT(mqtt_subscribe,  mqtt subscribe topic);
MSH_CMD_EXPORT(mqtt_unsubscribe, mqtt unsubscribe topic);
#endif /* FINSH_USING_MSH */
