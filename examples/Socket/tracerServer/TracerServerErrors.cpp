
#include "TracerServerErrors.h"



ErrMsgBase:: ErrMsgsInfo TracerServerErrorsStr = {


	// TracerService
	TS_TRACERSERVICE_ENVIRONMENTVARIABLE_NOTDEFINED,
		"The %s environment variable is not defined",
	TS_TRACERSERVICE_SIGNALRECEIVED,
		"Received the signal: %lu, shutdown...",

	// TracerSocketsPool
	TS_TRACERSOCKETSPOOL_EXCEPTIONONSOCKET,
		"Exception on the socket, SocketType: $ld",
	TS_TRACERSOCKETSPOOL_READFROMCLIENTSOCKET_FAILED,
		"The readFromClientSocket method of the TracerSocketsPool failed",

	/*
	// common
	CM_ACTIVATION_WRONG,
		"Activation wrong",
	CM_CONFIGITEMWRONG,
		"Configuration item value wrong. Section: %s, item: %s",
	CM_WRONGSOAPMETHODCONFIGURATION,
		"The '%s' plugin is configured using the wrong SOAP method",
	CM_SSCANF_FAILED,
		"sscanf failed",
	*/
	TS_NEW_FAILED,
		"new failed"

	// Insert here other errors...

} ;

