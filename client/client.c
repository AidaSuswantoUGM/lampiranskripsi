#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include<open62541/client_subscriptions.h>
#include<MQTTClient.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "open62541.lib")
#pragma comment(lib, "paho-mqtt3c.lib")
//open62541.lib
//paho - mqtt3c.lib

const char* opcusrn = "testt";
const char* opcpswd = "1234";
const char* opcaddr = "opc.tcp://169.254.181.200:4840";
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
const char* tp_hasil = "pilot_hasil";
//
const char* tp_s_startstop = "SW_StartStop";
const char* tp_s_downflow = "SW_DownFlow";
const char* tp_s_upflow = "SW_UpFlow";
const char* tp_s_serial = "SW_Serial";
const char* tp_s_paralel = "SW_Paralel";
const char* tp_s_v1 = "SW_V1";
const char* tp_s_v2 = "SW_V2";
const char* tp_s_hasil = "SW_Hasil";
//
const char* tp_v_a1 = "pilot_v_a1";
const char* tp_v_a2 = "pilot_v_a2";
//
const char* tp_v_b1 = "pilot_v_b1";
//
const char* tp_v_c1 = "pilot_v_c1";
const char* tp_v_c2 = "pilot_v_c2";
//
const char* tp_v_d1 = "pilot_v_d1";
const char* tp_v_d2 = "pilot_v_d2";
//
const char* tp_v_e1 = "pilot_v_e1";
const char* tp_v_e2 = "pilot_v_e2";
//
const char* tp_v_f1 = "pilot_v_f1";
const char* tp_v_f2 = "pilot_v_f2";
//
const char* tp_v_g2 = "pilot_v_g2";
//
const char* tp_v_h1 = "pilot_v_h1";
const char* tp_v_h2 = "pilot_v_h2";
//
const char* tp_v_s1 = "pilot_v_s1";
//end of topic

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
    const UA_NodeId nodeId_upflow = UA_NODEID_STRING(2, "OPC.Pilot_UpFlow");
    const UA_NodeId nodeId_downflow = UA_NODEID_STRING(2, "OPC.Pilot_DownFlow");
    const UA_NodeId nodeId_hasil = UA_NODEID_STRING(2, "OPC.Pilot_Hasil");
    const UA_NodeId nodeId_V1 = UA_NODEID_STRING(2, "OPC.Pilot_V1");
    const UA_NodeId nodeId_V2 = UA_NODEID_STRING(2, "OPC.Pilot_V2");
    const UA_NodeId nodeId_paralel = UA_NODEID_STRING(2, "OPC.Pilot_Paralel");
    const UA_NodeId nodeId_serial = UA_NODEID_STRING(2, "OPC.Pilot_Serial");
    //
    const UA_NodeId nodeId_V_A1 = UA_NODEID_STRING(2, "OPC.V_A1");
    const UA_NodeId nodeId_V_A2 = UA_NODEID_STRING(2, "OPC.V_A2");
    //
    const UA_NodeId nodeId_V_B1 = UA_NODEID_STRING(2, "OPC.V_B1");
    //
    const UA_NodeId nodeId_V_C1 = UA_NODEID_STRING(2, "OPC.V_C1");
    const UA_NodeId nodeId_V_C2 = UA_NODEID_STRING(2, "OPC.V_C2");
    //
    const UA_NodeId nodeId_V_D1 = UA_NODEID_STRING(2, "OPC.V_D1");
    const UA_NodeId nodeId_V_D2 = UA_NODEID_STRING(2, "OPC.V_D2");
    //
    const UA_NodeId nodeId_V_E1 = UA_NODEID_STRING(2, "OPC.V_E1");
    const UA_NodeId nodeId_V_E2 = UA_NODEID_STRING(2, "OPC.V_E2");
    //
    const UA_NodeId nodeId_V_F1 = UA_NODEID_STRING(2, "OPC.V_F1");
    const UA_NodeId nodeId_V_F2 = UA_NODEID_STRING(2, "OPC.V_F2");
    //
    const UA_NodeId nodeId_V_G2 = UA_NODEID_STRING(2, "OPC.V_G2");
    //
    const UA_NodeId nodeId_V_H1 = UA_NODEID_STRING(2, "OPC.V_H1");
    const UA_NodeId nodeId_V_H2 = UA_NODEID_STRING(2, "OPC.V_H2");
    //
    const UA_NodeId nodeId_V_S1 = UA_NODEID_STRING(2, "OPC.V_S1");
    //

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

    UA_MonitoredItemCreateRequest item_upflow = UA_MonitoredItemCreateRequest_default(nodeId_upflow);
    UA_MonitoredItemCreateResult item_response_upflow = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_upflow, (void*)tp_upflow, handler_events, NULL);
    if (item_response_upflow.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_UpFlow");
    }
    UA_MonitoredItemCreateRequest item_downflow = UA_MonitoredItemCreateRequest_default(nodeId_downflow);
    UA_MonitoredItemCreateResult item_response_downflow = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_downflow, (void*)tp_downflow, handler_events, NULL);
    if (item_response_downflow.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_DownFlow");
    }

    UA_MonitoredItemCreateRequest item_hasil = UA_MonitoredItemCreateRequest_default(nodeId_hasil);
    UA_MonitoredItemCreateResult item_response_hasil = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_hasil, (void*)tp_hasil, handler_events, NULL);
    if (item_response_hasil.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_Hasil");
    }

    UA_MonitoredItemCreateRequest item_v1 = UA_MonitoredItemCreateRequest_default(nodeId_V1);
    UA_MonitoredItemCreateResult item_response_v1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_v1, (void*)tp_v1, handler_events, NULL);
    if (item_response_v1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V1");
    }

    UA_MonitoredItemCreateRequest item_v2 = UA_MonitoredItemCreateRequest_default(nodeId_V2);
    UA_MonitoredItemCreateResult item_response_v2 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_v2, (void*)tp_v2, handler_events, NULL);
    if (item_response_v2.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V2");
    }
    
    UA_MonitoredItemCreateRequest item_paralel = UA_MonitoredItemCreateRequest_default(nodeId_paralel);
    UA_MonitoredItemCreateResult item_response_paralel = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_paralel, (void*)tp_paralel, handler_events, NULL);
    if (item_response_paralel.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_Paralel");
    }

    UA_MonitoredItemCreateRequest item_serial = UA_MonitoredItemCreateRequest_default(nodeId_serial);
    UA_MonitoredItemCreateResult item_response_serial = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_serial, (void*)tp_serial, handler_events, NULL);
    if (item_response_serial.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_Serial");
    }

    //
    UA_MonitoredItemCreateRequest item_V_A1 = UA_MonitoredItemCreateRequest_default(nodeId_V_A1);
    UA_MonitoredItemCreateResult item_response_V_A1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_A1, (void*)tp_v_a1, handler_events, NULL);
    if (item_response_V_A1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A1");
    }

    UA_MonitoredItemCreateRequest item_V_A2 = UA_MonitoredItemCreateRequest_default(nodeId_V_A2);
    UA_MonitoredItemCreateResult item_response_V_A2 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_A2, (void*)tp_v_a2, handler_events, NULL);
    if (item_response_V_A2.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A2");
    }
     
    //
    UA_MonitoredItemCreateRequest item_V_B1 = UA_MonitoredItemCreateRequest_default(nodeId_V_B1);
    UA_MonitoredItemCreateResult item_response_V_B1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_B1, (void*)tp_v_b1, handler_events, NULL);
    if (item_response_V_B1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A1");
    }
    
    //
    UA_MonitoredItemCreateRequest item_V_C1 = UA_MonitoredItemCreateRequest_default(nodeId_V_C1);
    UA_MonitoredItemCreateResult item_response_V_C1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_C1, (void*)tp_v_c1, handler_events, NULL);
    if (item_response_V_C1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A1");
    }

    UA_MonitoredItemCreateRequest item_V_C2 = UA_MonitoredItemCreateRequest_default(nodeId_V_C2);
    UA_MonitoredItemCreateResult item_response_V_C2 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_C2, (void*)tp_v_c2, handler_events, NULL);
    if (item_response_V_C2.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A2");
    }
    
    //
    UA_MonitoredItemCreateRequest item_V_D1 = UA_MonitoredItemCreateRequest_default(nodeId_V_D1);
    UA_MonitoredItemCreateResult item_response_V_D1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_D1, (void*)tp_v_d1, handler_events, NULL);
    if (item_response_V_D1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A1");
    }

    UA_MonitoredItemCreateRequest item_V_D2 = UA_MonitoredItemCreateRequest_default(nodeId_V_A2);
    UA_MonitoredItemCreateResult item_response_V_D2 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_D2, (void*)tp_v_d2, handler_events, NULL);
    if (item_response_V_D2.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A2");
    }
    
    //
    UA_MonitoredItemCreateRequest item_V_E1 = UA_MonitoredItemCreateRequest_default(nodeId_V_E1);
    UA_MonitoredItemCreateResult item_response_V_E1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_E1, (void*)tp_v_e1, handler_events, NULL);
    if (item_response_V_E1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A1");
    }

    UA_MonitoredItemCreateRequest item_V_E2 = UA_MonitoredItemCreateRequest_default(nodeId_V_E2);
    UA_MonitoredItemCreateResult item_response_V_E2 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_E2, (void*)tp_v_e2, handler_events, NULL);
    if (item_response_V_E2.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A2");
    }
    
    //
    UA_MonitoredItemCreateRequest item_V_F1 = UA_MonitoredItemCreateRequest_default(nodeId_V_F1);
    UA_MonitoredItemCreateResult item_response_V_F1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_F1, (void*)tp_v_f1, handler_events, NULL);
    if (item_response_V_F1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A1");
    }

    UA_MonitoredItemCreateRequest item_V_F2 = UA_MonitoredItemCreateRequest_default(nodeId_V_F2);
    UA_MonitoredItemCreateResult item_response_V_F2 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_F2, (void*)tp_v_f2, handler_events, NULL);
    if (item_response_V_F2.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A2");
    }
    
    //
    UA_MonitoredItemCreateRequest item_V_G2 = UA_MonitoredItemCreateRequest_default(nodeId_V_G2);
    UA_MonitoredItemCreateResult item_response_V_G2 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_G2, (void*)tp_v_g2, handler_events, NULL);
    if (item_response_V_G2.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A2");
    }
    
    //
    UA_MonitoredItemCreateRequest item_V_H1 = UA_MonitoredItemCreateRequest_default(nodeId_V_H1);
    UA_MonitoredItemCreateResult item_response_V_H1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_H1, (void*)tp_v_h1, handler_events, NULL);
    if (item_response_V_H1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A1");
    }

    UA_MonitoredItemCreateRequest item_V_H2 = UA_MonitoredItemCreateRequest_default(nodeId_V_H2);
    UA_MonitoredItemCreateResult item_response_V_H2 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_H2, (void*)tp_v_h2, handler_events, NULL);
    if (item_response_V_H2.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A2");
    }
    
    //
    UA_MonitoredItemCreateRequest item_V_S1 = UA_MonitoredItemCreateRequest_default(nodeId_V_S1);
    UA_MonitoredItemCreateResult item_response_V_S1 = UA_Client_MonitoredItems_createDataChange(opcclient, subId, UA_TIMESTAMPSTORETURN_BOTH, item_V_S1, (void*)tp_v_s1, handler_events, NULL);
    if (item_response_V_S1.statusCode == UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "monitoring OPC.Pilot_V_A1");
    }
    //end of subs

    rc = MQTTClient_subscribe(mqttclient, tp_s_startstop, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_startstop, rc);
    }
    rc = MQTTClient_subscribe(mqttclient, tp_s_downflow, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_downflow, rc);
    }
    rc = MQTTClient_subscribe(mqttclient, tp_s_upflow, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_upflow, rc);
    }
    rc = MQTTClient_subscribe(mqttclient, tp_s_serial, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_serial, rc);
    }
    rc = MQTTClient_subscribe(mqttclient, tp_s_paralel, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_paralel, rc);
    }
    rc = MQTTClient_subscribe(mqttclient, tp_s_v1, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_v1, rc);
    }
    rc = MQTTClient_subscribe(mqttclient, tp_s_v2, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_v2, rc);
    }
    rc = MQTTClient_subscribe(mqttclient, tp_s_hasil, 0);
    if (rc != MQTTCLIENT_SUCCESS) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "failed to subscribe to MQTT %s %d\n", tp_s_hasil, rc);
    }


    while (opcrun) {
        retval = UA_Client_run_iterate(opcclient, 1000);
    }

    MQTTClient_unsubscribe(mqttclient, tp_s_startstop);
    MQTTClient_unsubscribe(mqttclient, tp_s_downflow);
    MQTTClient_unsubscribe(mqttclient, tp_s_upflow);
    MQTTClient_unsubscribe(mqttclient, tp_s_serial);
    MQTTClient_unsubscribe(mqttclient, tp_s_paralel);
    MQTTClient_unsubscribe(mqttclient, tp_s_v1);
    MQTTClient_unsubscribe(mqttclient, tp_s_v2);
    MQTTClient_unsubscribe(mqttclient, tp_s_hasil);
    /* Clean up */
    MQTTClient_disconnect(mqttclient, 10000);
    MQTTClient_destroy(&mqttclient);
    UA_Client_Subscriptions_deleteSingle(opcclient, subId);
    UA_Client_delete(opcclient); /* Disconnects the client internally */
    return EXIT_SUCCESS;
}