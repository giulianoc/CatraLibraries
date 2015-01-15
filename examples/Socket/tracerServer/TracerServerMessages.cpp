
#include "TracerServerMessages.h"


ErrMsgBase:: ErrMsgsInfo TracerServerMessagesStr = {

	// TracerService
	TS_TRACERSERVER_INITIALIZING,
		"Service initializing...",
	TS_TRACERSERVICE_INITIALIZED,
		"Service initialized",
	TS_TRACERSERVICE_RECEIVEDONSTOP,
		"Service received onStop event...",
	TS_TRACERSERVICE_STOPPING,
		"Service stopping...",
	TS_TRACERSERVICE_STOPPED,
		"Service stopped",
	TS_TRACERSERVICE_STARTING,
		"Service starting...",
	TS_TRACERSERVICE_RUNNING,
		"Service running. Server listening on %s:%lu, Service version: %s, Operative System: %s",
	TS_TRACERSERVICE_SHUTDOWN,
		"Service is going down...",
	TS_TRACERSERVICE_NOTRUNNING,
		"Service is not running",

	// TracerSocketsPool
	TS_TRACERSOCKETSPOOL_ADDEDCLIENTCONNECTION,
		"Added a client connection. Remote IP address: %s, Remote port: %ld",
	TS_TRACERSOCKETSPOOL_REMOVEDCLIENTCONNECTION,
		"Removed a client connection. Remote IP address: %s, Remote port: %ld"

	// Insert here other errors...

} ;

