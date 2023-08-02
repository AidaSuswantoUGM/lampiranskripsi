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
	UA_Int32 tmposa= 42;
	UA_Variant_setScalarCopy(&attr.value, &tmposa, &UA_TYPES[UA_TYPES_INT32]);
}