#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include<open62541/client_subscriptions.h>
#include<MQTTClient.h>
#include<signal.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

/*const char* opcusrn = "testt";
const char* opcpswd = "1234";*/
const char* opcaddr = "opc.tcp://localhost:16664";
const char* mqttadr = "localhost:1883";
const char* mqttusrn = "asus";
const char* mqttpswd = "1234";
const char* mqttclid = "testopcmqtt";
const long mqtttimeout = 10000L;

//mqtt topic yg digunakan
const char* tp_startstop = "pilot_startstop";
const char* tp_downflow = "pilot_downflow";
const char* tp_upflow = "pilot_upflow";
const char* tp_serial = "pilot_serial";
const char* tp_paralel = "pilot_paralel";
const char* tp_v1 = "pilot_v1";
const char* tp_v2 = "pilot_v2";
const char* tp_v3 = "pilot_v3";
const char* tp_v4 = "pilot_v4";
const char* tp_hasil = "pilot_hasil";
//
const char* tp_s_startstop = "SW_StartStop";
const char* tp_s_downflow = "SW_DownFlow";
const char* tp_s_upflow = "SW_UpFlow";
const char* tp_s_serial = "SW_Serial";
const char* tp_s_paralel = "SW_Paralel";
const char* tp_s_v1 = "SW_V1";
const char* tp_s_v2 = "SW_V2";
const char* tp_s_v3 = "SW_V3";
const char* tp_s_v4 = "SW_V4";
const char* tp_s_hasil = "SW_Hasil";
//

static UA_Boolean opcrun = true;

MQTTClient_deliveryToken* mqtttoken;

static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    opcrun = false;
}

static void handler_events32(UA_Client* client, UA_UInt32 subId, void* subContext, UA_UInt32 monId, void* monContext, UA_DataValue* value) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "data %s berubah", (char*)monContext);
    UA_Int32 tmpvar;
    if (UA_Variant_hasScalarType(&value->value, &UA_TYPES[UA_TYPES_INT32])) {
        tmpvar = *(UA_Int32*)value->value.data;
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "nilai barunya : %d", tmpvar);
    }
    MQTTClient mqttcl = subContext;
    MQTTClient_message msg = MQTTClient_message_initializer;
    char bfr[10];
    snprintf(bfr, sizeof(bfr), "%d", tmpvar);
    msg.payload = bfr;
    msg.payloadlen = strlen(bfr);
    msg.qos = 0;
    msg.retained = 0;
    int rtc = MQTTClient_publishMessage(mqttcl, (char*)monContext, &msg, mqtttoken);
    if (rtc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "error %d", rtc);
    }
}

static void handler_eventsbool(UA_Client* client, UA_UInt32 subId, void* subContext, UA_UInt32 monId, void* monContext, UA_DataValue* value) {
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
    int rtc = MQTTClient_publishMessage(mqttcl, (char*)monContext, &msg, mqtttoken);
    if (rtc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "error %d", rtc);
    }
}

static int mqttbutton(void* context, char* topic, int topiclen, MQTTClient_message* msg) {
    //UA_StatusCode rv;
    char bfr[17];
    snprintf(bfr, sizeof(bfr), "OPC.%s", topic);
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%s is pressed", bfr);
    UA_Boolean tmpb = true;
    /*UA_WriteRequest wrq;
    UA_WriteRequest_init(&wrq);
    wrq.nodesToWrite = UA_WriteValue_new();
    wrq.nodesToWriteSize = 1;
    wrq.nodesToWrite[0].nodeId = UA_NODEID_STRING(1, "node.boolsa");
    wrq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
    wrq.nodesToWrite[0].value.hasValue = true;
    wrq.nodesToWrite[0].value.value.type = &UA_TYPES[UA_TYPES_BOOLEAN];
    wrq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE;
    wrq.nodesToWrite[0].value.value.data = &tmpb;
    UA_WriteResponse wresp = UA_Client_Service_write(context, wrq);
    if(wresp.responseHeader.serviceResult != UA_STATUSCODE_GOOD){
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "error %d", wresp.responseHeader.serviceResult);
    }
    UA_WriteResponse_clear(&wresp);
    tmpb = false;
    wrq.nodesToWrite[0].value.value.data = &tmpb;
    wresp = UA_Client_Service_write(context, wrq);
    if(wresp.responseHeader.serviceResult != UA_STATUSCODE_GOOD){
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "error %d", wresp.responseHeader.serviceResult);
    }
    UA_WriteResponse_clear(&wresp);
    UA_WriteRequest_clear(&wrq);*/
    UA_Variant* tmpvar = UA_Variant_new();
    UA_Variant_setScalarCopy(tmpvar, &tmpb, &UA_TYPES[UA_TYPES_BOOLEAN]);
    UA_Client_writeValueAttribute(context, UA_NODEID_STRING(1, "node.boolsa"), tmpvar);
    UA_Variant_delete(tmpvar);
    MQTTClient_freeMessage(&msg);
    return 1;
}

int main() {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Client* opcclient = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(opcclient));
    //UA_StatusCode retval = UA_Client_connectUsername(opcclient, opcaddr, opcusrn, opcpswd);
    UA_StatusCode retval = UA_Client_connect(opcclient, opcaddr);
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

    rc = MQTTClient_create(&mqttclient, mqttadr, mqttclid, MQTTCLIENT_PERSISTENCE_NONE, NULL);
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
    const UA_NodeId nodeId_startstop = UA_NODEID_STRING(1, "node.tes");
    const UA_NodeId nodeId_upflow = UA_NODEID_STRING(1, "node.kedua");
    
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
    UA_MonitoredItemCreateResult item_response_startstop = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_startstop, (void*)tp_startstop, handler_events32, NULL);
    if (item_response_startstop.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring node.tes");
    }

    UA_MonitoredItemCreateRequest item_upflow = UA_MonitoredItemCreateRequest_default(nodeId_upflow);
    UA_MonitoredItemCreateResult item_response_upflow = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_upflow, (void*)tp_upflow, handler_events32, NULL);
    if (item_response_upflow.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring node.kedua");
    }
    //end of subs

    const UA_NodeId nodeId_boolsa= UA_NODEID_STRING(1, "node.boolsa");
    const UA_NodeId nodeId_booldu= UA_NODEID_STRING(1, "node.booldu");

    UA_MonitoredItemCreateRequest item_booldu = UA_MonitoredItemCreateRequest_default(nodeId_booldu);
    UA_MonitoredItemCreateResult item_response_booldu= UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_booldu, (void*)tp_downflow, handler_eventsbool, NULL);
    if (item_response_booldu.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring node.boolsa");
    }

    UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_booldu, NULL, NULL, NULL);

    rc = MQTTClient_subscribe(mqttclient, tp_s_startstop, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT SW_StartStop %d\n", rc);
    }
    /*rc = MQTTClient_subscribe(mqttclient, tp_s_v1, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT SW_V1 %d\n", rc);
    }*/

    while (opcrun) {
        retval = UA_Client_run_iterate(opcclient, 1000);
    }

    rc = MQTTClient_unsubscribe(mqttclient, tp_s_startstop);
    rc = MQTTClient_unsubscribe(mqttclient, tp_s_v1);
    /* Clean up */
    MQTTClient_disconnect(mqttclient, 10000);
    MQTTClient_destroy(&mqttclient);
    UA_Client_Subscriptions_deleteSingle(opcclient, subId);
    UA_Client_delete(opcclient); /* Disconnects the client internally */
    return EXIT_SUCCESS;
}