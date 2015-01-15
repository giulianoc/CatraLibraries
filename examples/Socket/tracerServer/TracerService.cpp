
#include "TracerService.h"
#include "TracerServerMessages.h"
#include "ProcessUtility.h"
#include "FileIO.h"
#include <stdlib.h>
#include <signal.h>


void signalHandler (int iSignal);

Tracer_p		_gptTracer			= (Tracer_p) NULL;

TracerService_p	_ptsTracerService	= (TracerService_p) NULL;


void signalHandler (int iSignal)

{

	{
		Error err = TracerServerErrors (__FILE__, __LINE__,
			TS_TRACERSERVICE_SIGNALRECEIVED,
			1, (unsigned long) iSignal);
		_gptTracer -> trace (Tracer:: TRACER_LFTAL,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_gptTracer != (Tracer_p) NULL)
	{
		if (_gptTracer -> flushOfTraces () != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_FLUSHOFTRACES_FAILED);	
			std:: cerr << (const char *) err << std:: endl;

			// return 1;
		}
	}

	if (_ptsTracerService != (TracerService_p) NULL)
	{
		if (_ptsTracerService -> cancel () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_CANCEL_FAILED);	
			_gptTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return 1;
		}
	}

	exit (iSignal);
	// return 0;
}


TracerService:: TracerService (void): Service ()

{

}


TracerService:: ~TracerService (void)

{

}


Error TracerService:: init (const char *pServiceName,
	const char *pServiceVersion, const char *pConfigurationPathName,
	const char *pPidPathName)

{

	Error					errServiceInit;


	if ((_pConfigurationPathName = new char [
		strlen (pConfigurationPathName) + 1]) == (char *) NULL)
	{
		Error err = TracerServerErrors (__FILE__, __LINE__,
			TS_NEW_FAILED);

		return err;
	}
	strcpy (_pConfigurationPathName, pConfigurationPathName);

	if ((_pServiceVersion = new char [
		strlen (pServiceVersion) + 1]) == (char *) NULL)
	{
		Error err = TracerServerErrors (__FILE__, __LINE__,
			TS_NEW_FAILED);

		delete [] _pConfigurationPathName;

		return err;
	}
	strcpy (_pServiceVersion, pServiceVersion);

	if ((_pPidPathName = new char [
		strlen (pPidPathName) +
		1]) == (char *) NULL)
	{
		Error err = TracerServerErrors (__FILE__, __LINE__,
			TS_NEW_FAILED);

		delete [] _pServiceVersion;
		delete [] _pConfigurationPathName;

		return err;
	}
	strcpy (_pPidPathName, pPidPathName);

	#ifdef WIN32
		if ((errServiceInit = Service:: init (
			pServiceName, "",
			(const char *) NULL, (const char *) NULL)) != errNoError)
	#else
		if ((errServiceInit = Service:: init (
			pServiceName, pServiceName)) != errNoError)
	#endif
	{
		/*
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) errServiceInit, __FILE__, __LINE__);

		Error err = TracerServerErrors (__FILE__, __LINE__,
			TOOLS_SERVICE_INIT_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		delete [] _pServiceVersion;
		delete [] _pConfigurationPathName;
		delete [] _pPidPathName;


		return errServiceInit;
	}


	return errNoError;
}


Error TracerService:: finish (void)

{

	Error_t					errServiceFinish;


	if ((errServiceFinish = Service:: finish ()) != errNoError)
	{
		/*
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) errServiceFinish, __FILE__, __LINE__);

		Error err = TracerServerErrors (__FILE__, __LINE__,
			TOOLS_SERVICE_FINISH_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		return errServiceFinish;
	}

	delete [] _pServiceVersion;
	delete [] _pConfigurationPathName;
	delete [] _pPidPathName;


	return errNoError;
}


Error TracerService:: onInit (void)

{

	Error_t					errStartTracer;
	char					pConfigurationBuffer [TS_MAXLONGLENGTH];
	Error_t					errGetItemValue;


	{
		Error								errParseError;

		if ((errParseError = _cfConfiguration. init (
			_pConfigurationPathName,
			"Tracer Server")) != errNoError)
		{
			// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			// 	CFGFILE_CONFIGURATIONFILE_INIT_FAILED);

			return errParseError;
		}
	}

	#ifdef WIN32
	#else
		{
			Error								errSetUserID;


			if ((errGetItemValue = _cfConfiguration. getItemValue (
				"TracerServer", "UserName", pConfigurationBuffer,
				TS_MAXLONGLENGTH)) != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
			 		CFG_CONFIG_GETITEMVALUE_FAILED,
			 		2, "TracerServer", "UserName");

				if (_cfConfiguration. finish () != errNoError)
				{
					// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
					// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
					// std:: cerr << (const char *) err << std:: endl;
				}

				return err;
			}

			if ((errSetUserID = ProcessUtility:: setUserAndGroupID (
				pConfigurationBuffer)) != errNoError)
			{
				// Error err = ToolsErrors (__FILE__, __LINE__,
				// 	TOOLS_PROCESSUTILITY_SETUSERANDGROUPID_FAILED);

				if (_cfConfiguration. finish () != errNoError)
				{
					// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
					// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
					// std:: cerr << (const char *) err << std:: endl;
				}

				return errSetUserID;
			}
		}
	#endif

	if ((errStartTracer = startTracer (&_cfConfiguration,
		&_tTracer, "Tracer")) != errNoError)
	{
		// Error err = TracerServerErrors (__FILE__, __LINE__,
		// 	CM_STARTTRACER_FAILED);
		// std:: cerr << (const char *) err << std:: endl;

		if (_cfConfiguration. finish () != errNoError)
		{
			// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}

		return errStartTracer;
	}

	_gptTracer					= &_tTracer;
	_ptsTracerService			= this;

	signal(SIGSEGV, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGABRT, signalHandler);
	// signal(SIGBUS, signalHandler);

	{
		Message msg = TracerServerMessages (__FILE__, __LINE__,
			TS_TRACERSERVER_INITIALIZING);
		_tTracer. trace (Tracer:: TRACER_LINFO, (const char *) msg,
			__FILE__, __LINE__);
	}

	// server socket init
	{
		unsigned long		ulMaxClients;
		Error_t				errGetItemValue;
		Error_t				errServerSocketInit;
		SocketImpl_p		pServerSocketImpl;


		if ((errGetItemValue = _cfConfiguration. getItemValue ("TracerServer",
			"LocalIPAddress", _pLocalIPAddress,
			SCK_MAXIPADDRESSLENGTH)) != errNoError)
		{
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "TracerServer", "LocalIPAddress");
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (stopTracer (&_tTracer) != errNoError)
			{
				// Error err = TracerServerErrors (__FILE__, __LINE__,
				// 	CM_STOPTRACER_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			if (_cfConfiguration. finish () != errNoError)
			{
				// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if ((errGetItemValue = _cfConfiguration. getItemValue ("TracerServer",
			"LocalPort", pConfigurationBuffer,
			TS_MAXLONGLENGTH)) != errNoError)
		{
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "TracerServer", "LocalPort");
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (stopTracer (&_tTracer) != errNoError)
			{
				// Error err = TracerServerErrors (__FILE__, __LINE__,
				// 	CM_STOPTRACER_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			if (_cfConfiguration. finish () != errNoError)
			{
				// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}
		_ulServerPort			= atol (pConfigurationBuffer);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("TracerServer",
			"MaxClients", pConfigurationBuffer,
			TS_MAXLONGLENGTH)) != errNoError)
		{
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "TracerServer", "MaxClients");
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (stopTracer (&_tTracer) != errNoError)
			{
				// Error err = TracerServerErrors (__FILE__, __LINE__,
				// 	CM_STOPTRACER_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			if (_cfConfiguration. finish () != errNoError)
			{
				// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}
		ulMaxClients			= atol (pConfigurationBuffer);

		if ((errServerSocketInit = _ssServerSocket. init (
			_pLocalIPAddress, _ulServerPort, true,
			SocketImpl:: STREAM, 50, 0, ulMaxClients)) != errNoError)
		{
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errServerSocketInit,
				__FILE__, __LINE__);

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_INIT_FAILED);
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (stopTracer (&_tTracer) != errNoError)
			{
				// Error err = TracerServerErrors (__FILE__, __LINE__,
				// 	CM_STOPTRACER_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			if (_cfConfiguration. finish () != errNoError)
			{
				// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			return errServerSocketInit;
		}

		if (_ssServerSocket. getSocketImpl (&pServerSocketImpl) !=
			errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKET_GETSOCKETIMPL_FAILED);
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (stopTracer (&_tTracer) != errNoError)
			{
				// Error err = TracerServerErrors (__FILE__, __LINE__,
				// 	CM_STOPTRACER_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			if (_cfConfiguration. finish () != errNoError)
			{
				// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (pServerSocketImpl -> setBlocking (false) != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_SETBLOCKING_FAILED);
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (stopTracer (&_tTracer) != errNoError)
			{
				// Error err = TracerServerErrors (__FILE__, __LINE__,
				// 	CM_STOPTRACER_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			if (_cfConfiguration. finish () != errNoError)
			{
				// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}

		if (pServerSocketImpl -> setMaxReceiveBuffer (32 * 1024) !=
			errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETIMPL_SETMAXRECEIVEBUFFER_FAILED);
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (stopTracer (&_tTracer) != errNoError)
			{
				// Error err = TracerServerErrors (__FILE__, __LINE__,
				// 	CM_STOPTRACER_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			if (_cfConfiguration. finish () != errNoError)
			{
				// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
				// std:: cerr << (const char *) err << std:: endl;
			}

			return err;
		}
	}

	if (_tspTracerSocketsPool. init (&_tTracer) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_INIT_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_ssServerSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_FINISH_FAILED);
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if (stopTracer (&_tTracer) != errNoError)
		{
			// Error err = TracerServerErrors (__FILE__, __LINE__,
			// 	CM_STOPTRACER_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}

		if (_cfConfiguration. finish () != errNoError)
		{
			// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}

	{
		Message msg = TracerServerMessages (__FILE__, __LINE__,
			TS_TRACERSERVICE_INITIALIZED);
		_tTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}


	return errNoError;
}


Error TracerService:: onStop (void)

{

	{
		Message msg = TracerServerMessages (__FILE__, __LINE__,
			TS_TRACERSERVICE_RECEIVEDONSTOP);
		_tTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	// finishTheService method called by cancel () can take some seconds because
	// it has to stop the tracer (System and Subscriber for H3G_DAAC)
	#ifdef WIN32
		if (WinThread:: getSleep (7, 0) != errNoError)
	#else
		if (PosixThread:: getSleep (7, 0) != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETSLEEP_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}


	return errNoError;
}


Error TracerService:: finishTheService (void)

{
	{
		Message msg = TracerServerMessages (__FILE__, __LINE__,
			TS_TRACERSERVICE_STOPPING);
		_tTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	// The next trace message cannot be moved at the end of this method
	// because the tracer will be already stopped
	{
		Message msg = TracerServerMessages (__FILE__, __LINE__,
			TS_TRACERSERVICE_STOPPED);
		_tTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (_tspTracerSocketsPool. cancel () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_CANCEL_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_tspTracerSocketsPool. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_FINISH_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_ssServerSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_FINISH_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (stopTracer (&_tTracer) != errNoError)
	{
		// Error err = StreamingServerErrors (__FILE__, __LINE__,
		// 	SS_STOPTRACER_FAILED);
		// std:: cerr << (const char *) err << std:: endl;
	}

	if (_cfConfiguration. finish () != errNoError)
	{
		// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
		// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
		// std:: cerr << (const char *) err << std:: endl;
	}


	return errNoError;
}


Error TracerService:: onStart (void)

{

	Error_t					errJoin;


	{
		Message msg = TracerServerMessages (__FILE__, __LINE__,
			TS_TRACERSERVICE_STARTING);
		_tTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	_tServerStartTime						= time (NULL);

	#ifdef WIN32
		{
			Message msg = TracerServerMessages (__FILE__, __LINE__,
				TS_TRACERSERVICE_RUNNING,
				4, _pLocalIPAddress, _ulServerPort,
				// "Not available on Windows", "Windows");
				_pServiceVersion, "Windows");
			_tTracer. trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}
	#else
		{
			Message msg = TracerServerMessages (__FILE__, __LINE__,
				TS_TRACERSERVICE_RUNNING,
				4, _pLocalIPAddress, _ulServerPort,
				_pServiceVersion, "Unix/Linux");
			_tTracer. trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}
	#endif

	if (_tspTracerSocketsPool. addSocket (
		SocketsPool:: SOCKETSTATUS_READ |
		SocketsPool:: SOCKETSTATUS_EXCEPTION,
		TS_TRACERSOCKETPOOL_SERVERSOCKET,
		&_ssServerSocket, (void *) NULL) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_ADDSOCKET_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (_tspTracerSocketsPool. start () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (_tspTracerSocketsPool. join (&errJoin) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_JOIN_FAILED);
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	{
		Message msg = TracerServerMessages (__FILE__, __LINE__,
			TS_TRACERSERVICE_SHUTDOWN);
		_tTracer. trace (
			Tracer:: TRACER_LINFO, (const char *) msg,
			__FILE__, __LINE__);
	}

	#ifdef WIN32
		if (cancel () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_CANCEL_FAILED);
			_tTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}
	#endif

	{
		Message msg = TracerServerMessages (__FILE__, __LINE__,
			TS_TRACERSERVICE_NOTRUNNING);
		_tTracer. trace (
			Tracer:: TRACER_LINFO, (const char *) msg,
			__FILE__, __LINE__);
	}


	return errNoError;
}


Error TracerService:: cancel (void)

{

	if (finishTheService () != errNoError)
		;		// no trace available, all is closed


	return errNoError;
}


#ifdef WIN32
#else
Error TracerService:: appendStartScriptCommand (
	Buffer_p pbServiceScriptFile)

{

	if (pbServiceScriptFile -> append (
			"\ttouch "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\tchmod 776 "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\ttouch "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\tchmod 776 "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\t. /"
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			".sh" "\n"
			"\n"
			"# PID saved in "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\tdaemon "
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\techo \"Started\" >> "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\treturn 0"
			) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		/* _tTracer is not yet initialized
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		return err;
	}


	return errNoError;
}


Error TracerService:: appendStopScriptCommand (
	Buffer_p pbServiceScriptFile)

{

	if (pbServiceScriptFile -> append (
			"\ttouch "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\tchmod 776 "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\ttouch "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\tchmod 776 "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\t. /"
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			".sh" "\n"
			"\n"
			"\n"
			"\tif [ ! -f "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			" ] ; then"
			"\n"
			"\t\techo \"PID file not found. "
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			" is Not Running\"" "\n"
			"\t\techo \"PID file not found. "
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			" is Not Running\" >> "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\t\treturn 1" "\n"
			"\tfi" "\n"
			"\n"
			"\tpid=`cat "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"`" "\n"
			"\tkill -INT $pid" "\n"
			"\n"
			"\tif [ \"$?\" = \"1\" ]; then" "\n"
			"\t\techo \"kill failed. Error: $?\"" "\n"
			"\t\techo \"kill failed. Error: $?\" >> "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\t\treturn 1" "\n"
			"\tfi" "\n"
			"\n"
			"\tNUM_SECONDS=0" "\n"
			"\tMAX_SECONDS=60" "\n"
			"\tSERVICERUNNING=1" "\n"
			"\n"
			"\twhile [ \"$NUM_SECONDS\" -lt \"$MAX_SECONDS\" ] && [ \"$SERVICERUNNING\" = \"1\" ]; do" "\n"
			"\t\trhstatus" "\n"
			"\t\tif [ \"$SERVICERUNNING\" = \"1\" ]; then" "\n"
			"\t\t\tsleep 1" "\n"
			"\t\t\tNUM_SECONDS=`expr $NUM_SECONDS + 1`" "\n"
			"\t\tfi" "\n"
			"\tdone" "\n"
			"\tif [ ! \"$SERVICERUNNING\" = \"0\" ]; then" "\n"
			"\t\techo \"error: service is not stopped\" >> "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\t\treturn 1" "\n"
			"\tfi" "\n"
			"\n"
			"\techo \"service is stopped in $NUM_SECONDS seconds\" >> "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\trm -f "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\treturn 0"
			) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		/* _tTracer is not yet initialized
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		return err;
	}


	return errNoError;
}


Error TracerService:: appendStatusScriptCommand (
	Buffer_p pbServiceScriptFile)

{

	if (pbServiceScriptFile -> append (
			"\ttouch "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\tchmod 776 "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\ttouch "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\tchmod 776 "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\n"
			"\t. /"
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			".sh" "\n"
			"\n"
			"#\tpid=`ps -ef | grep \"/"
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\" | grep -v grep | awk '{print $2}'`" "\n"
			"\tif [ -e "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			" ]; then" "\n"
			"\t\tpid=`cat "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pPidPathName
			) != errNoError ||
		pbServiceScriptFile -> append (
			"`" "\n"
			"\telse" "\n"
			"\t\tpid=\"\"" "\n"
			"\tfi" "\n"
			"#\tif [ \"$pid\" = \"\" ]; then" "\n"
			"\tcheckpid $pid" "\n"
			"\tif [ \"$?\" = \"1\" ]; then" "\n"
			"\t\tSERVICERUNNING=0" "\n"
			"\t\techo \"Service is Not Running\"" "\n"
			"\t\techo \"Service is Not Running\" >> "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\telse" "\n"
			"\t\tSERVICERUNNING=1" "\n"
			"\t\techo \"Service (pid $pid) is running...\"" "\n"
			"\t\techo \"Service (pid $pid) is running...\" >> "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_pServiceDebugFile
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\n"
			"\tfi" "\n"
			"\n"
			"\treturn 0"
			) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		/* _tTracer is not yet initialized
		_tTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		return err;
	}


	return errNoError;
}
#endif


Error TracerService:: startTracer (
	ConfigurationFile_p pcfConfiguration,
	Tracer_p ptTracer, const char *pSectionName)

{

	char								pCacheSizeOfTraceFile [
		TS_MAXLONGLENGTH];
	char								pTraceDirectory [
		TS_MAXTRACEFILELENGTH];
	char								pTraceFileName [
		TS_MAXTRACEFILELENGTH];
	Error_t								errGetItemValue;
	Error_t								errCreateDir;
	char								pMaxTraceFileSize [
		TS_MAXLONGLENGTH];
	char								pTraceFilePeriodInSecs [
		TS_MAXLONGLENGTH];
	char								pCompressedTraceFile [
		TS_MAXLONGLENGTH];
	Boolean_t							bCompressedTraceFile;
	char								pTraceFilesNumberToMaintain [
		TS_MAXLONGLENGTH];
	long								lTraceFilesNumberToMaintain;
	char								pTraceOnTTY [
		TS_MAXLONGLENGTH];
	Boolean_t							bTraceOnTTY;
	char								pTraceLevel [
		TS_MAXLONGLENGTH];
	long								lTraceLevel;
	char								pListenTracePort [
		TS_MAXLONGLENGTH];
	unsigned long						ulListenTracePort;
	long								lSecondsBetweenTwoCheckTraceToManage;
	Error_t								errInit;


	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"CacheSizeOfTraceFile", pCacheSizeOfTraceFile,
		TS_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "CacheSizeOfTraceFile");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceDirectory", pTraceDirectory,
		TS_MAXTRACEFILELENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceDirectory");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceFileName", pTraceFileName,
		TS_MAXTRACEFILELENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceFileName");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	#ifdef WIN32
		if ((errCreateDir = FileIO:: createDirectory (pTraceDirectory,
			0, true, true)) != errNoError)
	#else
		if ((errCreateDir = FileIO:: createDirectory (pTraceDirectory,
			S_IRUSR | S_IWUSR | S_IXUSR |
			S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, true, true)) != errNoError)
	#endif
	{
		return errCreateDir;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"MaxTraceFileSize", pMaxTraceFileSize,
		TS_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "MaxTraceFileSize");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceFilePeriodInSecs", pTraceFilePeriodInSecs,
		TS_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceFilePeriodInSecs");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"CompressedTraceFile", pCompressedTraceFile,
		TS_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "CompressedTraceFile");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}
	if (!strcmp (pCompressedTraceFile, "true"))
		bCompressedTraceFile				= true;
	else
		bCompressedTraceFile				= false;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceFilesNumberToMaintain",
		pTraceFilesNumberToMaintain,
		TS_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceFilesNumberToMaintain");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	lTraceFilesNumberToMaintain			= atol (pTraceFilesNumberToMaintain);

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceOnTTY", pTraceOnTTY, TS_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceOnTTY");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}
	if (!strcmp (pTraceOnTTY, "true"))
		bTraceOnTTY							= true;
	else
		bTraceOnTTY							= false;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"TraceLevel", pTraceLevel, TS_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "TraceLevel");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	if (!strcmp (pTraceLevel, "LDBG1"))
		lTraceLevel				= 0;
	else if (!strcmp (pTraceLevel, "LDBG2"))
		lTraceLevel				= 1;
	else if (!strcmp (pTraceLevel, "LDBG3"))
		lTraceLevel				= 2;
	else if (!strcmp (pTraceLevel, "LDBG4"))
		lTraceLevel				= 3;
	else if (!strcmp (pTraceLevel, "LDBG5"))
		lTraceLevel				= 4;
	else if (!strcmp (pTraceLevel, "LDBG6"))
		lTraceLevel				= 5;
	else if (!strcmp (pTraceLevel, "LINFO"))
		lTraceLevel				= 6;
	else if (!strcmp (pTraceLevel, "LMESG"))
		lTraceLevel				= 7;
	else if (!strcmp (pTraceLevel, "LWRNG"))
		lTraceLevel				= 8;
	else if (!strcmp (pTraceLevel, "LERRR"))
		lTraceLevel				= 9;
	else if (!strcmp (pTraceLevel, "LFTAL"))
		lTraceLevel				= 10;
	else
		lTraceLevel				= 6;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (pSectionName,
		"ListenTracePort", pListenTracePort, TS_MAXLONGLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, pSectionName, "ListenTracePort");
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}

	ulListenTracePort		= atol (pListenTracePort);

	lSecondsBetweenTwoCheckTraceToManage		= 7;

	if ((errInit = ptTracer -> init (
		pSectionName,					// pName
		atol (pCacheSizeOfTraceFile),	// lCacheSizeOfTraceFile K-byte
		pTraceDirectory,				// pTraceDirectory
		pTraceFileName,					// pTraceFileName
		atol (pMaxTraceFileSize),		// lMaxTraceFileSize K-byte
		atol (pTraceFilePeriodInSecs),	// lTraceFilePeriodInSecs
		bCompressedTraceFile,			// bCompressedTraceFile
		false,							// bClosedFileToBeCopied
		(const char *) NULL,			// pClosedFilesRepository
		lTraceFilesNumberToMaintain,	// lTraceFilesNumberToMaintain
		true,							// bTraceOnFile
		bTraceOnTTY,					// bTraceOnTTY
		lTraceLevel,					// lTraceLevel
		lSecondsBetweenTwoCheckTraceToManage,
		3000,							// lMaxTracesNumber
		ulListenTracePort,				// lListenPort
		1000,							// lTracesNumberAllocatedOnOverflow
		1000)) != errNoError)			// lSizeOfEachBlockToGzip K-byte
	{
		// Error err = TracerErrors (__FILE__, __LINE__,
		// 	TRACER_TRACER_INIT_FAILED);	
		// std:: cerr << (const char *) err << std:: endl;

		return errInit;
	}

	/*
	{
		long		lStackSize;
		char		pBuff [1024];

		if (ptTracer -> getStackSize (&lStackSize) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_SETSTACKSIZE_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}
		sprintf (pBuff, "Stack size prima: %ld", lStackSize);
		// std:: cout << pBuff << std:: endl;

		lStackSize		= 20480 * 1024;
		if (ptTracer -> setStackSize (lStackSize) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_SETSTACKSIZE_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}

		if (ptTracer -> getStackSize (&lStackSize) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_SETSTACKSIZE_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}
		sprintf (pBuff, "Stack size dopo: %ld", lStackSize);
		// std:: cout << pBuff << std:: endl;
	}
	*/

	if (ptTracer -> start () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_START_FAILED);	
		// std:: cerr << (const char *) err << std:: endl;

		if (ptTracer -> finish (true) != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_FINISH_FAILED);	
			// std:: cerr << (const char *) err << std:: endl;
		}

		return err;
	}


	return errNoError;
}


Error TracerService:: stopTracer (Tracer_p ptTracer)

{

	if (ptTracer -> cancel () != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_CANCEL_FAILED);	
		ptTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);

		return err;
	}

	if (ptTracer -> finish (true) != errNoError)
	{
		Error err = TracerErrors (__FILE__, __LINE__,
			TRACER_TRACER_FINISH_FAILED);	
		// std:: cerr << (const char *) err << std:: endl;

		return err;
	}


	return errNoError;
}

