
#ifndef ConnectionsManagerMessages_h
	#define ConnectionsManagerMessages_h

	#include "Error.h"
	#include <iostream>


	//
	// Defined errors:
	//   
 
	enum ConnectionsManagerMessagesCodes {

		// ConnectionsManagerService
		CM_CONNECTIONSMANAGER_INITIALIZING,
		CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGEVENTSSET,
		CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGSCHEDULER,
		CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGPROCESSORS,
		CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGPROCESSOR,
		CM_CONNECTIONSMANAGERSERVICE_INITIALIZED,
		CM_CONNECTIONSMANAGERSERVICE_RECEIVEDONSTOP,
		CM_CONNECTIONSMANAGERSERVICE_STOPPING,
		CM_CONNECTIONSMANAGERSERVICE_MAINPROCESSORSFINISHED,
		CM_CONNECTIONSMANAGERSERVICE_STOPPED,
		CM_CONNECTIONSMANAGERSERVICE_STARTING,
		CM_CONNECTIONSMANAGERSERVICE_PROCESSORSTARTED,
		CM_CONNECTIONSMANAGERSERVICE_SCHEDULERSTARTED,
		CM_CONNECTIONSMANAGERSERVICE_RUNNING,
		CM_CONNECTIONSMANAGERSERVICE_SHUTDOWN,
		CM_CONNECTIONSMANAGERSERVICE_NOTRUNNING,
		CM_CONNECTIONSMANAGERSERVICE_SCHEDULERSTOPPED,
		CM_CONNECTIONSMANAGERSERVICE_MAINPROCESSORSTOPPED,

		// CMProcessor
		CM_CMPROCESSOR_EVENTHANDLER,
		CM_CMPROCESSOR_RECEIVEDREQUEST,
		CM_CMPROCESSOR_STATISTICS,
		CM_CMPROCESSOR_FREECONNECTIONS,
		CM_CMPROCESSOR_HTTPGETPROXY,
		CM_CMPROCESSOR_HTTPGETPROXYRESPONSETIME,

		// Session
		CM_SESSION_CONNECTIONARRIVED,
		CM_SESSION_CONNECTIONCLOSED,
		CM_SESSION_ADDSOCKET,
		CM_SESSION_DELETESOCKET,
		CM_SESSION_WROTERESPONSE,

		// SBEventsSet
		CM_CMEVENTSSET_ALLOCATEDMOREEVENT,

		// Insert here other errors...

		CM_MAX_MESSAGES

	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long',
		// possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomMessageClass (ConnectionsManagerMessages, CM_MAX_MESSAGES)

#endif

