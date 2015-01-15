
#ifndef TracerServerMessages_h
	#define TracerServerMessages_h

	#include "Error.h"
	#include <iostream>


	//
	// Defined errors:
	//   
 
	enum TracerServerMessagesCodes {

		// TracerService
		TS_TRACERSERVER_INITIALIZING,
		TS_TRACERSERVICE_INITIALIZED,
		TS_TRACERSERVICE_RECEIVEDONSTOP,
		TS_TRACERSERVICE_STOPPING,
		TS_TRACERSERVICE_STOPPED,
		TS_TRACERSERVICE_STARTING,
		TS_TRACERSERVICE_RUNNING,
		TS_TRACERSERVICE_SHUTDOWN,
		TS_TRACERSERVICE_NOTRUNNING,

		// TracerSocketsPool
		TS_TRACERSOCKETSPOOL_ADDEDCLIENTCONNECTION,
		TS_TRACERSOCKETSPOOL_REMOVEDCLIENTCONNECTION,

		// Insert here other errors...

		TS_MAX_MESSAGES

	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long',
		// possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomMessageClass (TracerServerMessages, TS_MAX_MESSAGES)

#endif

