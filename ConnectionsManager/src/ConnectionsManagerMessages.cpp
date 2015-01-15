
#include "ConnectionsManagerMessages.h"


ErrMsgBase:: ErrMsgsInfo ConnectionsManagerMessagesStr = {

	// ConnectionsManagerService
	{ CM_CONNECTIONSMANAGER_INITIALIZING,
		"Service initializing..." },
	{ CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGEVENTSSET,
		"EventsSet initializing..." },
	{ CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGSCHEDULER,
		"Scheduler initializing..." },
	{ CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGPROCESSORS,
		"Processors (%lu) initializing..." },
	{ CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGPROCESSOR,
		"CM processor (%lu) initializing..." },
	{ CM_CONNECTIONSMANAGERSERVICE_INITIALIZED,
		"Service initialized" },
	{ CM_CONNECTIONSMANAGERSERVICE_RECEIVEDONSTOP,
		"Service received onStop event..." },
	{ CM_CONNECTIONSMANAGERSERVICE_STOPPING,
		"Service stopping..." },
	{ CM_CONNECTIONSMANAGERSERVICE_MAINPROCESSORSFINISHED,
		"Service. Main processors finished" },
	{ CM_CONNECTIONSMANAGERSERVICE_STOPPED,
		"Service stopped" },
	{ CM_CONNECTIONSMANAGERSERVICE_STARTING,
		"Service starting..." },
	{ CM_CONNECTIONSMANAGERSERVICE_PROCESSORSTARTED,
		"Processors started" },
	{ CM_CONNECTIONSMANAGERSERVICE_SCHEDULERSTARTED,
		"Scheduler started" },
	{ CM_CONNECTIONSMANAGERSERVICE_RUNNING,
		"Service running. Server listening on %s:%lu, Service version: %s, Operative System: %s" },
	{ CM_CONNECTIONSMANAGERSERVICE_SHUTDOWN,
		"Service is going down..." },
	{ CM_CONNECTIONSMANAGERSERVICE_NOTRUNNING,
		"Service is not running" },
	{ CM_CONNECTIONSMANAGERSERVICE_SCHEDULERSTOPPED,
		"Scheduler stopped" },
	{ CM_CONNECTIONSMANAGERSERVICE_MAINPROCESSORSTOPPED,
		"Main processors stopped" },

	// CMProcessor
	{ CM_CMPROCESSOR_EVENTHANDLER,
		"CMProcessor id.: %lu, Received event '%s'" },
	{ CM_CMPROCESSOR_RECEIVEDREQUEST,
		"SocketID: %lu. Event creation: %s, Received request. Client IP Address: %s, Client Port: %ld, URL: %s, HEADER: '%s', BODY: %s" },
	{ CM_CMPROCESSOR_STATISTICS,
		"STATISTICS. %s @%ld@ millicecs" },
	{ CM_CMPROCESSOR_FREECONNECTIONS,
		"Free connections: @%lu@" },
	{ CM_CMPROCESSOR_HTTPGETPROXY,
		"Initialization of %s thread. WebServerIPAddress: %s, WebServerPort: %lu, LocalIPAddress: %s, URI: %s, URL parameters: %s, Timeout: %lu" },
	{ CM_CMPROCESSOR_HTTPGETPROXYRESPONSETIME,
		"%s@%lu@" },

	// Session
	{ CM_SESSION_CONNECTIONARRIVED,
		"SocketID: %lu. Client Session arrived - Client IP address: %s, Client port: %ld, Start time: @%lu@" },
	{ CM_SESSION_CONNECTIONCLOSED,
		"SocketID: %lu. Client Session closed - Client IP address: %s, Client port: %ld, Start time: @%lu@" },
	{ CM_SESSION_ADDSOCKET,
		"SocketID: %lu. Add socket. Fd: %ld" },
	{ CM_SESSION_DELETESOCKET,
		"SocketID: %lu. Delete socket. Fd: %ld" },
	{ CM_SESSION_WROTERESPONSE,
		"SocketID: %lu. Sent response - Client IP Address: %s, Client Port: %lu, Response finished: %ld, HEADER (%lu): '%s', BODY (%lu): %s" },

	// SBEventsSet
	{ CM_CMEVENTSSET_ALLOCATEDMOREEVENT,
		"Allocated more %s events. Total number: %lu, total memory used %lu bytes" }

	// Insert here other errors...

} ;

