
#ifndef TracerServerErrors_h
	#define TracerServerErrors_h

	#include "Error.h"
	#include <iostream>

	enum TracerServerErrorsCodes {

		// TracerService
		TS_TRACERSERVICE_ENVIRONMENTVARIABLE_NOTDEFINED,
		TS_TRACERSERVICE_SIGNALRECEIVED,

		// TracerSocketsPool
		TS_TRACERSOCKETSPOOL_EXCEPTIONONSOCKET,
		TS_TRACERSOCKETSPOOL_READFROMCLIENTSOCKET_FAILED,

		/*
		// common
		CM_ACTIVATION_WRONG,
		CM_CONFIGITEMWRONG,
		CM_WRONGSOAPMETHODCONFIGURATION,
		CM_SSCANF_FAILED,
		*/
		TS_NEW_FAILED,

		// Insert here other errors...

		TS_MAXERRORS
	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long',
		// possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomErrorClass (TracerServerErrors, TS_MAXERRORS)
   
#endif

