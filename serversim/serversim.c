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

    UA_VariableAttributes attrdua = UA_VariableAttributes_default;
    intku = 67;
    UA_Variant_setScalarCopy(&attrdua.value, &intku, &UA_TYPES[UA_TYPES_INT32]);
    attrdua.description = UA_LOCALIZEDTEXT_ALLOC("id-ID", "node ke-2");
    attrdua.displayName = UA_LOCALIZEDTEXT_ALLOC("id-ID", "tes node 2");
    attrdua.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId myIntegerNodeIdua = UA_NODEID_STRING_ALLOC(1, "node.kedua");
    UA_QualifiedName myIntegerNamedua = UA_QUALIFIEDNAME_ALLOC(1, "ngetes node kedua");
    UA_Server_addVariableNode(server, myIntegerNodeIdua, parentNodeId,
        parentReferenceNodeId, myIntegerNamedua,
        UA_NODEID_NULL, attrdua, NULL, NULL);
    UA_VariableAttributes_clear(&attrdua);
    UA_NodeId_clear(&myIntegerNodeIdua);
    UA_QualifiedName_clear(&myIntegerNamedua);

    UA_VariableAttributes attrtiga= UA_VariableAttributes_default;
    UA_Boolean mba = false;
    UA_Variant_setScalarCopy(&attrtiga.value, &mba, &UA_TYPES[UA_TYPES_BOOLEAN]);
    attrtiga.description = UA_LOCALIZEDTEXT_ALLOC("id-ID", "node boolean");
    attrtiga.displayName = UA_LOCALIZEDTEXT_ALLOC("id-ID", "tes node boolean");
    attrtiga.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId boolnode = UA_NODEID_STRING_ALLOC(1, "node.boolsa");
    UA_QualifiedName qboolnode = UA_QUALIFIEDNAME_ALLOC(1, "ngetes node boolean");
    UA_Server_addVariableNode(server, boolnode, parentNodeId,
        parentReferenceNodeId, qboolnode,
        UA_NODEID_NULL, attrtiga, NULL, NULL);
    UA_VariableAttributes_clear(&attrtiga);
    UA_NodeId_clear(&boolnode);
    UA_QualifiedName_clear(&qboolnode);

    UA_VariableAttributes attrem = UA_VariableAttributes_default;
    mba = true;
    UA_Variant_setScalarCopy(&attrem.value, &mba, &UA_TYPES[UA_TYPES_BOOLEAN]);
    attrem.description = UA_LOCALIZEDTEXT_ALLOC("id-ID", "node boolean dua");
    attrem.displayName = UA_LOCALIZEDTEXT_ALLOC("id-ID", "tes node boolean dua");
    attrem.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    UA_NodeId boolnodedua = UA_NODEID_STRING_ALLOC(1, "node.booldu");
    UA_QualifiedName qboolnodedu = UA_QUALIFIEDNAME_ALLOC(1, "ngetes node boolean dua");
    UA_Server_addVariableNode(server, boolnodedua, parentNodeId,
        parentReferenceNodeId, qboolnodedu,
        UA_NODEID_NULL, attrem, NULL, NULL);
    UA_VariableAttributes_clear(&attrem);
    UA_NodeId_clear(&boolnodedua);
    UA_QualifiedName_clear(&qboolnodedu);
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