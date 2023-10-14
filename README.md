# Branch client OPC UA untuk plant 2 tabung

[source code](https://github.com/AidaSuswantoUGM/lampiranskripsi/blob/2tabung/client/client.c)

```
```

## Source Code untuk 1 node OPC UA dan 1 topik MQTT

```
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include<open62541/client_subscriptions.h>
#include<MQTTClient.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "open62541.lib")
#pragma comment(lib, "paho-mqtt3c.lib")

const char* opcusrn = "testt";
const char* opcpswd = "1234";
const char* opcaddr = "opc.tcp://169.254.181.200:4840";
const char* mqttusrn = "asus";
const char* mqttpswd = "1234";
const char* mqttclid = "testopcmqtt";
const long mqtttimeout = 10000L;

const char* tp_startstop = "pilot_startstop";
//
const char* tp_s_startstop = "SW_StartStop";

static UA_Boolean opcrun = true;

MQTTClient_deliveryToken* mqtttoken;

static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    opcrun = false;
}

static void handler_events(UA_Client* client, UA_UInt32 subId, void* subContext, UA_UInt32 monId, void* monContext, UA_DataValue* value) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "data %s berubah", (char*)monContext);
    UA_Boolean tmpvar;
    if (UA_Variant_hasScalarType(&value->value, &UA_TYPES[UA_TYPES_BOOLEAN])) {
        tmpvar = *(UA_Boolean*)value->value.data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "nilai barunya : %d", tmpvar);
    }
    MQTTClient mqttcl = subContext;
    MQTTClient_message msg = MQTTClient_message_initializer;
    if (tmpvar) {
        msg.payload = "1";
    }
    else {
        msg.payload = "0";
    }
    msg.payloadlen = 1;
    msg.qos = 0;
    msg.retained = 0;
    int rtc = MQTTClient_publishMessage(mqttcl, monContext, &msg, mqtttoken);
    if (rtc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "error %d", rtc);
    }
}

static int mqttbutton(void* context, char* topic, int topiclen, MQTTClient_message* msg) {
    char bfr[17];
    snprintf(bfr, sizeof(bfr), "OPC.%s", topic);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s is pressed", bfr);
    UA_Boolean tmpb = true;
    UA_Variant* tmpvar = UA_Variant_new();
    UA_Variant_setScalarCopy(tmpvar, &tmpb, &UA_TYPES[UA_TYPES_BOOLEAN]);
    UA_Client_writeValueAttribute(context, UA_NODEID_STRING(2, bfr), tmpvar);
    tmpb = false;
    UA_Variant_setScalarCopy(tmpvar, &tmpb, &UA_TYPES[UA_TYPES_BOOLEAN]);
    UA_Client_writeValueAttribute(context, UA_NODEID_STRING(2, bfr), tmpvar);
    MQTTClient_freeMessage(&msg);
    return 1;
}

int main() {
    FILE* mqttfile;
    char mqttaddr[30];
    int mer;
    mer = fopen_s(&mqttfile, "mqtt.txt", "r");
    if (mer != 0) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "file mqtt.txt error");
    }
    fgets(mqttaddr, 30, mqttfile);
    fclose(mqttfile);
    UA_Client* opcclient = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(opcclient));
    UA_StatusCode retval = UA_Client_connectUsername(opcclient, opcaddr, opcusrn, opcpswd);
    if (retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(opcclient);
        return (int)retval;
    }

    MQTTClient mqttclient;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    conn_opts.username = mqttusrn;
    conn_opts.password = mqttpswd;
    conn_opts.keepAliveInterval = 30;
    conn_opts.cleansession = 1;

    rc = MQTTClient_create(&mqttclient, mqttaddr, mqttclid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        printf("failed to create client object %d\n", rc);
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to create mqttclient obj %d\n", rc);
        exit(EXIT_FAILURE);
    }
    rc = MQTTClient_setCallbacks(mqttclient, opcclient, NULL, mqttbutton, NULL);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to setcallback %d\n", rc);
    }
    rc = MQTTClient_connect(mqttclient, &conn_opts);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to connect %d\n", rc);
    }

    //UA_NodeID
    const UA_NodeId nodeId_startstop = UA_NODEID_STRING(2, "OPC.Pilot_StartStop");

    //create subscription to opc server
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(opcclient, request, mqttclient, NULL, NULL);

    if (response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        UA_Client_disconnect(opcclient);
        UA_Client_delete(opcclient);
        return EXIT_FAILURE;
    }
    UA_UInt32 subId = response.subscriptionId;
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "Create subscription succeeded, id %u", subId);

    //create subscription to nodeIDs
    //
    UA_MonitoredItemCreateRequest item_startstop = UA_MonitoredItemCreateRequest_default(nodeId_startstop);
    UA_MonitoredItemCreateResult item_response_startstop = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_startstop, (void*)tp_startstop, handler_events, NULL);
    if (item_response_startstop.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_StartStop");
    }

    rc = MQTTClient_subscribe(mqttclient, tp_s_startstop, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_startstop, rc);
    }

    while (opcrun) {
        retval = UA_Client_run_iterate(opcclient, 1000);
    }

    MQTTClient_unsubscribe(mqttclient, tp_s_startstop);
    /* Clean up */
    MQTTClient_disconnect(mqttclient, 10000);
    MQTTClient_destroy(&mqttclient);
    UA_Client_Subscriptions_deleteSingle(opcclient, subId);
    UA_Client_delete(opcclient); /* Disconnects the client internally */
    return EXIT_SUCCESS;
}```
