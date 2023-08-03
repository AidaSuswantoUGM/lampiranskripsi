#include<open62541/plugin/log_stdout.h>
#include<open62541/server.h>
#include<open62541/server_config_default.h>

#include<signal.h>
#include<stdlib.h>

#pragma comment (lib, "ws2_32")
#pragma comment (lib, "Iphlpapi")

static volatile UA_Boolean running = true;
static void stophandler(int sig) {
	UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl+c");
	running = false;
}

int main(void) {
	signal(SIGINT, stophandler);
	signal(SIGTERM, stophandler);

	UA_Server* server = UA_Server_new();
	UA_ServerConfig_setDefault(UA_ServerConfig_setMinimal(server, 16664, NULL));

	UA_VariableAttributes attr = UA_VariableAttributes_default;
	UA_Int32 intku= 42;
	UA_Variant_setScalarCopy(&attr.value, &intku, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", "node tes");
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "tes node");
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId myIntegerNodeId = UA_NODEID_STRING_ALLOC(1, "node.tes");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME_ALLOC(1, "ngetes node");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
        parentReferenceNodeId, myIntegerName,
        UA_NODEID_NULL, attr, NULL, NULL);

    /* allocations on the heap need to be freed */
    UA_VariableAttributes_clear(&attr);
    UA_NodeId_clear(&myIntegerNodeId);
    UA_QualifiedName_clear(&myIntegerName);
    /*
    UA_Int32 vrn;
    UA_Variant smntr;
    
    while (true) {
        vrn = rand() % 100;
        UA_Variant_setScalar(&smntr, &vrn, &UA_TYPES[UA_TYPES_INT32]);
        UA_Server_writeValue(server, UA_NODEID_STRING(1, "node.tes"), smntr);
        UA_sleep_ms(2000);
    }*/

    /*UA_Boolean waitInternal = true;
    while (running) {
        UA_Server_run_iterate(server, waitInternal);
    }

    UA_Server_run_shutdown(server);*/
    UA_StatusCode rv = UA_Server_run(server, &running);
    UA_Server_delete(server);
    //return 0;
}