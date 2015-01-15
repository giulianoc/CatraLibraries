
#include "ConnectionsManagerService.h"
#include "ConnectionsManagerMessages.h"
#include "FileReader.h"
#include "FileIO.h"
#include "DateTime.h"
#include "StringTokenizer.h"
#include "ProcessUtility.h"
#include <vector>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>


void signalHandler (int iSignal);

Tracer_p					_gptSystemTracer					=
	(Tracer_p) NULL;

ConnectionsManagerService_p	_gpcmConnectionsManagerService		=
	(ConnectionsManagerService_p) NULL;


void signalHandler (int iSignal)

{

	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_SIGNALRECEIVED,
			1, (unsigned long) iSignal);
		_gptSystemTracer -> trace (Tracer:: TRACER_LFTAL,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_gptSystemTracer != (Tracer_p) NULL)
	{
		if (_gptSystemTracer -> flushOfTraces () != errNoError)
		{
			Error err = TracerErrors (__FILE__, __LINE__,
				TRACER_TRACER_FLUSHOFTRACES_FAILED);	
			std:: cerr << (const char *) err << std:: endl;

			// return 1;
		}
	}

	if (_gpcmConnectionsManagerService != (ConnectionsManagerService_p) NULL)
	{
		if (_gpcmConnectionsManagerService -> cancel () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_CANCEL_FAILED);	
			_gptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// return 1;
		}
	}

	exit (iSignal);
	// return 0;
}


ConnectionsManagerService:: ConnectionsManagerService (void): Service ()

{

}


ConnectionsManagerService:: ~ConnectionsManagerService (void)

{

}


Error ConnectionsManagerService:: init (const char *pServiceName,
	const char *pServiceVersion, const char *pConfigurationPathName,
	const char *pPidPathName)

{

	Error					errServiceInit;


	#ifdef VECTORFREESESSIONS
	#else
		if ((_phHasher = new IntHasher_t) == (IntHasher_p) NULL)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_NEW_FAILED);

			return err;
		}

		if ((_pcComparer = new IntCmp_t) == (IntCmp_p) NULL)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NEW_FAILED);

			delete _phHasher;

			return err;
		}

		if ((_phmFreeSessions = new SessionsHashMap_t (
			100, *_phHasher, *_pcComparer)) ==
			(SessionsHashMap_p) NULL)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NEW_FAILED);

			delete _pcComparer;
			delete _phHasher;

			return err;
		}
	#endif

	if ((_pConfigurationPathName = new char [
		strlen (pConfigurationPathName) + 1]) == (char *) NULL)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_NEW_FAILED);

		#ifdef VECTORFREESESSIONS
		#else
			delete _phmFreeSessions;
			delete _pcComparer;
			delete _phHasher;
		#endif

		return err;
	}
	strcpy (_pConfigurationPathName, pConfigurationPathName);

	if ((_pServiceVersion = new char [
		strlen (pServiceVersion) + 1]) == (char *) NULL)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_NEW_FAILED);

		delete [] _pConfigurationPathName;
		#ifdef VECTORFREESESSIONS
		#else
			delete _phmFreeSessions;
			delete _pcComparer;
			delete _phHasher;
		#endif


		return err;
	}
	strcpy (_pServiceVersion, pServiceVersion);

	if ((_pPidPathName = new char [
		strlen (pPidPathName) +
		1]) == (char *) NULL)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_NEW_FAILED);

		delete [] _pServiceVersion;
		delete [] _pConfigurationPathName;
		#ifdef VECTORFREESESSIONS
		#else
			delete _phmFreeSessions;
			delete _pcComparer;
			delete _phHasher;
		#endif


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
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) errServiceInit, __FILE__, __LINE__);

		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			TOOLS_SERVICE_INIT_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		delete [] _pServiceVersion;
		delete [] _pConfigurationPathName;
		delete [] _pPidPathName;
		#ifdef VECTORFREESESSIONS
		#else
			delete _phmFreeSessions;
			delete _pcComparer;
			delete _phHasher;
		#endif



		return errServiceInit;
	}


	return errNoError;
}


Error ConnectionsManagerService:: finish (void)

{

	Error_t					errServiceFinish;


	if ((errServiceFinish = Service:: finish ()) != errNoError)
	{
		/*
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) errServiceFinish, __FILE__, __LINE__);

		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			TOOLS_SERVICE_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		return errServiceFinish;
	}

	delete [] _pServiceVersion;
	delete [] _pConfigurationPathName;
	delete [] _pPidPathName;

	#ifdef VECTORFREESESSIONS
	#else
		_phmFreeSessions -> clear ();

		delete _phmFreeSessions;
		delete _pcComparer;
		delete _phHasher;
	#endif


	return errNoError;
}


Error ConnectionsManagerService:: onInit (void)

{

	Error_t					errStartTracer;
	char					pConfigurationBuffer [CM_MAXLONGLENGTH];
	Error_t					errGetItemValue;


	{
		Error								errParseError;

		if ((errParseError = _cfConfiguration. init (
			_pConfigurationPathName,
			"Connections Manager")) != errNoError)
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
				(const char *) _bServiceName, "UserName", pConfigurationBuffer,
				CM_MAXLONGLENGTH)) != errNoError)
			{
				Error err = ConfigurationErrors (__FILE__, __LINE__,
			 		CFG_CONFIG_GETITEMVALUE_FAILED,
			 		2, (const char *) _bServiceName, "UserName");

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

	if ((errStartTracer = BaseProcessor:: startTracer (&_cfConfiguration,
		&_tSystemTracer, "SystemLogs")) != errNoError)
	{
		// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

	_gptSystemTracer					= &_tSystemTracer;
	_gpcmConnectionsManagerService		= this;

	signal(SIGSEGV, signalHandler);
	signal(SIGINT, signalHandler);
	signal(SIGABRT, signalHandler);
	// signal(SIGBUS, signalHandler);

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGER_INITIALIZING);
		_tSystemTracer. trace (
			Tracer:: TRACER_LINFO, (const char *) msg,
			__FILE__, __LINE__);
	}

	if (onInitEventsSet (&_pesEventsSet, &_tSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_ONINITEVENTSSET_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);

		if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
		{
			// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

	// CM sessions initialization
	{
		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"MaxConnections", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "MaxConnections");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulMaxConnections		= strtoul (pConfigurationBuffer,
			(char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"TimeoutToWaitRequestInSeconds", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "TimeoutToWaitAnswerInSeconds");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulTimeoutToWaitRequestInSeconds		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"AdditionalMicrosecondsToWait", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "AdditionalMicrosecondsToWait");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulAdditionalMicrosecondsToWait		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		{
			long			lConnectionIdentifier;


			if ((_psSessions = new Session_t [
				_ulMaxConnections]) == (Session_p) NULL)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_NEW_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (onFinishEventsSet (&_pesEventsSet,
					&_tSystemTracer) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
					errNoError)
				{
					// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
				_vFreeSessions. reserve (_ulMaxConnections);
			#else
				_phmFreeSessions -> clear ();
			#endif

			for (lConnectionIdentifier = 0;
				lConnectionIdentifier < (long) _ulMaxConnections;
				lConnectionIdentifier++)
			{
				if ((_psSessions [lConnectionIdentifier]). init (
					lConnectionIdentifier, &_spSocketsPool,
					_pesEventsSet, _ulTimeoutToWaitRequestInSeconds,
					_ulAdditionalMicrosecondsToWait,
					&_tSystemTracer) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_INIT_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					while (--lConnectionIdentifier >= 0)
					{
						if ((_psSessions [lConnectionIdentifier]).
							finish () != errNoError)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_SESSION_FINISH_FAILED);
							_tSystemTracer. trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}
					}

					delete [] _psSessions;

					if (onFinishEventsSet (&_pesEventsSet,
						&_tSystemTracer) != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
						CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
						errNoError)
					{
						// Error err = ConnectionsManagerErrors (
						// 	__FILE__, __LINE__,
						// 	CM_STOPTRACER_FAILED);
						// std:: cerr << (const char *) err << std:: endl;
					}

					if (_cfConfiguration. finish () != errNoError)
					{
						// Error err = ConfigurationFileErrors (
						//	__FILE__, __LINE__,
						// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
						// std:: cerr << (const char *) err << std:: endl;
					}

					return err;
				}

				#ifdef VECTORFREESESSIONS
					_vFreeSessions. insert (_vFreeSessions. end (),
						&(_psSessions [lConnectionIdentifier]));
				#else
					int			iDidInsert;


					_phmFreeSessions -> InsertWithoutDuplication (
						(int) lConnectionIdentifier,
						&(_psSessions [lConnectionIdentifier]),
						&iDidInsert);

					if (iDidInsert == 0)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_CMPROCESSOR_INSERTONFREESESSIONSFAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);

						if ((_psSessions [lConnectionIdentifier]).
							finish () != errNoError)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_SESSION_FINISH_FAILED);
							_tSystemTracer. trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						while (--lConnectionIdentifier >= 0)
						{
							if ((_psSessions [lConnectionIdentifier]).
								finish () != errNoError)
							{
								Error err = ConnectionsManagerErrors (
									__FILE__, __LINE__,
									CM_SESSION_FINISH_FAILED);
								_tSystemTracer. trace (Tracer:: TRACER_LERRR,
									(const char *) err, __FILE__, __LINE__);
							}
						}

						delete [] _psSessions;

						if (onFinishEventsSet (&_pesEventsSet,
							&_tSystemTracer) != errNoError)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
						CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
							_tSystemTracer. trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}

						if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
							errNoError)
						{
							// Error err = ConnectionsManagerErrors (
							// 	__FILE__, __LINE__,
							// 	CM_STOPTRACER_FAILED);
							// std:: cerr << (const char *) err << std:: endl;
						}

						if (_cfConfiguration. finish () != errNoError)
						{
							// Error err = ConfigurationFileErrors (
							//	__FILE__, __LINE__,
							// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
							// std:: cerr << (const char *) err << std:: endl;
						}

						return err;
					}
				#endif
			}
		}

		if (_mtFreeSessions. init (
			PMutex:: MUTEX_RECURSIVE) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif

			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

	// server socket init
	{
		unsigned long		ulMaxClients;
		Error_t				errGetItemValue;
		Error_t				errServerSocketInit;
		SocketImpl_p		pServerSocketImpl;


		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"LocalIPAddress", _pLocalIPAddress,
			SCK_MAXIPADDRESSLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "LocalIPAddress");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"LocalPort", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "LocalPort");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulServerPort			=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"MaxClients", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "MaxClients");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		ulMaxClients			=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"TimeoutToAcceptNewConnectionsInSeconds", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "TimeoutToAcceptNewConnectionsInSeconds");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulTimeoutToAcceptNewConnectionsInSeconds		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		if ((errServerSocketInit = _ssServerSocket. init (
			_pLocalIPAddress, _ulServerPort, true,
			SocketImpl:: STREAM, _ulTimeoutToAcceptNewConnectionsInSeconds, 0,
			ulMaxClients)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errServerSocketInit,
				__FILE__, __LINE__);

			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_INIT_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

	// SocketsPool
	if (_spSocketsPool. init (_pesEventsSet,
		&_tSystemTracer) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_INIT_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_ssServerSocket. finish () != errNoError)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SERVERSOCKET_FINISH_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if (_mtFreeSessions. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		#ifdef VECTORFREESESSIONS
			_vFreeSessions. clear ();
		#else
			_phmFreeSessions -> clear ();
		#endif


		{
			long				lConnectionIdentifier;

			for (lConnectionIdentifier = 0;
				lConnectionIdentifier < (long) _ulMaxConnections;
				lConnectionIdentifier++)
			{
				if ((_psSessions [lConnectionIdentifier]).
					finish () != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_SESSION_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}
		}

		delete [] _psSessions;

		if (onFinishEventsSet (&_pesEventsSet,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
		{
			// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

	// EventsSet
	{
		if (_pesEventsSet -> addDestination ("CMProcessor") != errNoError)
		{
			Error err = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_ADDDESTINATION_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGSCHEDULER);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	// initialization of the scheduler, _cspCheckSocketsPoolTimes
	//	and _cssCheckServerSocketTimes
	{
		if ((errGetItemValue = _cfConfiguration. getItemValue ("Scheduler",
			"SchedulerSleepTimeInMilliSecs", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "Scheduler", "SchedulerSleepTimeInMilliSecs");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulSchedulerSleepTimeInMilliSecs		= strtoul (pConfigurationBuffer,
			(char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("Scheduler",
			"CheckServerSocketPeriodInMilliSecs", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "Scheduler", "CheckServerSocketPeriodInMilliSecs");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulCheckServerSocketPeriodInMilliSecs		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("Scheduler",
			"CheckSocketsPoolPeriodInMilliSecs", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "Scheduler", "CheckSocketsPoolPeriodInMilliSecs");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulCheckSocketsPoolPeriodInMilliSecs		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("Scheduler",
			"CheckUnusedSocketsPeriodInSeconds", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "Scheduler", "CheckUnusedSocketsPeriodInSeconds");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		_ulCheckUnusedSocketsPeriodInSeconds	=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);
		if (_ulCheckUnusedSocketsPeriodInSeconds < 5)
			_ulCheckUnusedSocketsPeriodInSeconds		= 5;

		if (_scScheduler. init (
			_ulSchedulerSleepTimeInMilliSecs) != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_INIT_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

		if (_cspCheckSocketsPoolTimes. init (
			_ulCheckSocketsPoolPeriodInMilliSecs, _pesEventsSet,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CHECKSOCKETSPOOLTIMES_INIT_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

		if (_cssCheckServerSocketTimes. init (
			_ulCheckServerSocketPeriodInMilliSecs, _pesEventsSet,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CHECKSERVERSOCKETTIMES_INIT_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

		if (_cssCheckUnusedSocketsTimes. init (
			_ulCheckUnusedSocketsPeriodInSeconds * 1000, _pesEventsSet,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CHECKUNUSEDSOCKETSTIMES_INIT_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_cssCheckServerSocketTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSERVERSOCKETTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier <
						(long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) !=
				errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

	// _lbHTTPProxyLoadBalancer
	{
		if ((errGetItemValue = _cfConfiguration. getItemValue ("HTTPProxy",
			"Enabled", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_bIsHTTPProxyEnabled                    = false;

			/*
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "Scheduler", "SchedulerSleepTimeInMilliSecs");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cssCheckServerSocketTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
			*/
		}
		else
		{
			#ifdef WIN32
				if (!stricmp (pConfigurationBuffer, "true"))
			#else
				if (!strcasecmp (pConfigurationBuffer, "true"))
			#endif
				_bIsHTTPProxyEnabled                    = true;
			else
				_bIsHTTPProxyEnabled                    = false;
		}

		if (_bIsHTTPProxyEnabled)
		{
			Error_t					errLoadBalancerInit;


			if ((errLoadBalancerInit = _lbHTTPProxyLoadBalancer. init (
				&_cfConfiguration, "HTTPProxy", &_tSystemTracer)) != errNoError)
			{
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) errLoadBalancerInit, __FILE__, __LINE__);

				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_INIT_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_cssCheckServerSocketTimes. finish () != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_cspCheckSocketsPoolTimes. finish () != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				// bDestroyTimes
				if (_scScheduler. finish (false) != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err,
						__FILE__, __LINE__);
				}

				if (_spSocketsPool. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SOCKETSPOOL_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_ssServerSocket. finish () != errNoError)
				{
					Error err = SocketErrors (__FILE__, __LINE__,
						SCK_SERVERSOCKET_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (_mtFreeSessions. finish () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				#ifdef VECTORFREESESSIONS
					_vFreeSessions. clear ();
				#else
					_phmFreeSessions -> clear ();
				#endif


				{
					long				lConnectionIdentifier;

					for (lConnectionIdentifier = 0;
						lConnectionIdentifier < (long) _ulMaxConnections;
						lConnectionIdentifier++)
					{
						if ((_psSessions [lConnectionIdentifier]).
							finish () != errNoError)
						{
							Error err = ConnectionsManagerErrors (
								__FILE__, __LINE__,
								CM_SESSION_FINISH_FAILED);
							_tSystemTracer. trace (Tracer:: TRACER_LERRR,
								(const char *) err, __FILE__, __LINE__);
						}
					}
				}

				delete [] _psSessions;

				if (onFinishEventsSet (&_pesEventsSet,
					&_tSystemTracer) != errNoError)
				{
					Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
						CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
				{
					// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
	}

	// Processors
	{
		long				lProcessorsNumber;
		unsigned long		ulMaxSimultaneousConnectionsToAccept;
		unsigned long		ulMaxDelayAcceptableInLoopInMilliSecs;
		unsigned long		ulMaxConnectionTTLInSeconds;
		unsigned long		ulMaxMilliSecondsToProcessAnEvent;


		if ((errGetItemValue = _cfConfiguration. getItemValue (
			(const char *) _bServiceName, "ProcessorsNumber",
			pConfigurationBuffer, CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, (const char *) _bServiceName, "ProcessorsNumber");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_bIsHTTPProxyEnabled)
			{
				if (_lbHTTPProxyLoadBalancer. finish () != errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cssCheckServerSocketTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		lProcessorsNumber			=
			atol (pConfigurationBuffer);
		if (lProcessorsNumber <= 0)
			// da calcolare secondo il n.ro dei processori della macchina
			_ulProcessorsNumber		= 1;
		else
			_ulProcessorsNumber		=
				lProcessorsNumber;

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"MaxSimultaneousConnectionsToAccept", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "MaxSimultaneousConnectionsToAccept");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_bIsHTTPProxyEnabled)
			{
				if (_lbHTTPProxyLoadBalancer. finish () != errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cssCheckServerSocketTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		ulMaxSimultaneousConnectionsToAccept		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"MaxDelayAcceptableInLoopInMilliSecs", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "MaxDelayAcceptableInLoopInMilliSecs");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_bIsHTTPProxyEnabled)
			{
				if (_lbHTTPProxyLoadBalancer. finish () != errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cssCheckServerSocketTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		ulMaxDelayAcceptableInLoopInMilliSecs		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"MaxConnectionTTLInSeconds", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "MaxConnectionTTLInSeconds");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_bIsHTTPProxyEnabled)
			{
				if (_lbHTTPProxyLoadBalancer. finish () != errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cssCheckServerSocketTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		ulMaxConnectionTTLInSeconds		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);
		if (ulMaxConnectionTTLInSeconds < 5)
			ulMaxConnectionTTLInSeconds		= 5;

		if ((errGetItemValue = _cfConfiguration. getItemValue ("CMListener",
			"MaxMilliSecondsToProcessAnEvent", pConfigurationBuffer,
			CM_MAXLONGLENGTH)) != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errGetItemValue,
				__FILE__, __LINE__);

			Error err = ConfigurationErrors (__FILE__, __LINE__,
				CFG_CONFIG_GETITEMVALUE_FAILED,
				2, "CMListener", "ulMaxMilliSecondsToProcessAnEvent");
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_bIsHTTPProxyEnabled)
			{
				if (_lbHTTPProxyLoadBalancer. finish () != errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cssCheckServerSocketTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (
							__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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
		ulMaxMilliSecondsToProcessAnEvent		=
			strtoul (pConfigurationBuffer, (char **) NULL, 10);

		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGPROCESSORS,
				1, _ulProcessorsNumber);
			_tSystemTracer. trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}

		// initialization processors
		#ifdef VECTORFREESESSIONS
			if (onInitProcessors (&_pspProcessor, _ulProcessorsNumber,
				&_cfConfiguration,
				&_scScheduler,
				_pesEventsSet,
				_bIsHTTPProxyEnabled,
				&_lbHTTPProxyLoadBalancer,
				ulMaxSimultaneousConnectionsToAccept,
				ulMaxDelayAcceptableInLoopInMilliSecs,
				&_vFreeSessions,
				&_mtFreeSessions,
				_psSessions,
				_ulMaxConnections,
				ulMaxConnectionTTLInSeconds,
				&_ssServerSocket,
				&_spSocketsPool,
				ulMaxMilliSecondsToProcessAnEvent,
				&_tSystemTracer) != errNoError)
		#else
			if (onInitProcessors (&_pspProcessor, _ulProcessorsNumber,
				&_cfConfiguration,
				&_scScheduler,
				_pesEventsSet,
				_bIsHTTPProxyEnabled,
				&_lbHTTPProxyLoadBalancer,
				ulMaxSimultaneousConnectionsToAccept,
				ulMaxDelayAcceptableInLoopInMilliSecs,
				_phmFreeSessions,
				&_mtFreeSessions,
				_psSessions,
				_ulMaxConnections,
				ulMaxConnectionTTLInSeconds,
				&_ssServerSocket,
				&_spSocketsPool,
				ulMaxMilliSecondsToProcessAnEvent,
				&_tSystemTracer) != errNoError)
		#endif
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONINITPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
				__FILE__, __LINE__);

			if (_bIsHTTPProxyEnabled)
			{
				if (_lbHTTPProxyLoadBalancer. finish () != errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_FINISH_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cssCheckServerSocketTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_cspCheckSocketsPoolTimes. finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			// bDestroyTimes
			if (_scScheduler. finish (false) != errNoError)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_SCHEDULER_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err,
					__FILE__, __LINE__);
			}

			if (_spSocketsPool. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_ssServerSocket. finish () != errNoError)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SERVERSOCKET_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (_mtFreeSessions. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			#ifdef VECTORFREESESSIONS
				_vFreeSessions. clear ();
			#else
				_phmFreeSessions -> clear ();
			#endif


			{
				long				lConnectionIdentifier;

				for (lConnectionIdentifier = 0;
					lConnectionIdentifier < (long) _ulMaxConnections;
					lConnectionIdentifier++)
				{
					if ((_psSessions [lConnectionIdentifier]).
						finish () != errNoError)
					{
						Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
							CM_SESSION_FINISH_FAILED);
						_tSystemTracer. trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}
				}
			}

			delete [] _psSessions;

			if (onFinishEventsSet (&_pesEventsSet,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
			{
				// Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
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

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_INITIALIZED);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}


	return errNoError;
}


Error ConnectionsManagerService:: onInitEventsSet (
	CMEventsSet_p *pesEventsSet,
	Tracer_p ptSystemTracer)

{

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGEVENTSSET);
		ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (((*pesEventsSet) = new CMEventsSet_t) ==
		(CMEventsSet_p) NULL)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_NEW_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);

		return err;
	}

	if ((*pesEventsSet) -> init (
		CMEventsSet:: CM_EVENTTYPENUMBER, &_tSystemTracer) !=
		errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_INIT_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		delete (*pesEventsSet);

		return err;
	}


	return errNoError;
}


Error ConnectionsManagerService:: onFinishEventsSet (
	CMEventsSet_p *pesEventsSet,
	Tracer_p ptSystemTracer)

{

	if ((*pesEventsSet) -> finish () != errNoError)
	{
		Error err = EventsSetErrors (__FILE__, __LINE__,
			EVSET_EVENTSSET_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	delete (*pesEventsSet);


	return errNoError;
}


#ifdef VECTORFREESESSIONS
	Error ConnectionsManagerService:: onInitProcessors (
		CMProcessor_p *pspProcessor,
		unsigned long ulProcessorsNumber,
		ConfigurationFile_p pcfConfiguration,
		Scheduler_p pscScheduler,
		CMEventsSet_p pesEventsSet,
		Boolean_t bIsHTTPProxyEnabled,
		LoadBalancer_p plbHTTPProxyLoadBalancer,
		unsigned long ulMaxSimultaneousConnectionsToAccept,
		unsigned long ulMaxDelayAcceptableInLoopInMilliSecs,
		std:: vector<Session_p> *pvFreeSessions,
		PMutex_p pmtFreeSessions,
		Session_p psSessions,
		unsigned long ulMaxConnections,
		unsigned long ulMaxConnectionTTLInSeconds,
		ServerSocket_p pssServerSocket,
		CMSocketsPool_p pspSocketsPool,
		unsigned long ulMaxMilliSecondsToProcessAnEvent,
		Tracer_p ptSystemTracer)
#else
	Error ConnectionsManagerService:: onInitProcessors (
		CMProcessor_p *pspProcessor,
		unsigned long ulProcessorsNumber,
		ConfigurationFile_p pcfConfiguration,
		Scheduler_p pscScheduler,
		CMEventsSet_p pesEventsSet,
		Boolean_t bIsHTTPProxyEnabled,
		LoadBalancer_p plbHTTPProxyLoadBalancer,
		unsigned long ulMaxSimultaneousConnectionsToAccept,
		unsigned long ulMaxDelayAcceptableInLoopInMilliSecs,
		SessionsHashMap_p phmFreeSessions,
		PMutex_p pmtFreeSessions,
		Session_p psSessions,
		unsigned long ulMaxConnections,
		unsigned long ulMaxConnectionTTLInSeconds,
		ServerSocket_p pssServerSocket,
		CMSocketsPool_p pspSocketsPool,
		unsigned long ulMaxMilliSecondsToProcessAnEvent,
		Tracer_p ptSystemTracer)
#endif

{

	long			lProcessorIdentifier;


	if (((*pspProcessor) = new CMProcessor_t [ulProcessorsNumber]) ==
		(CMProcessor_p) NULL)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_NEW_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);

		return err;
	}

	for (lProcessorIdentifier = 0;
		lProcessorIdentifier < (long) ulProcessorsNumber;
		lProcessorIdentifier++)
	{
		{
			Message msg = ConnectionsManagerMessages (
				__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_INITIALIZINGPROCESSOR,
				1, (unsigned long) lProcessorIdentifier);
			ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}

		#ifdef VECTORFREESESSIONS
			if (((*pspProcessor) [lProcessorIdentifier]).
				init (
				lProcessorIdentifier,
				pcfConfiguration,
				pscScheduler,
				pesEventsSet,
				bIsHTTPProxyEnabled,
				plbHTTPProxyLoadBalancer,
				ulMaxSimultaneousConnectionsToAccept,
				ulMaxDelayAcceptableInLoopInMilliSecs,
				pvFreeSessions,
				pmtFreeSessions,
				psSessions,
				ulMaxConnections,
				ulMaxConnectionTTLInSeconds,
				pssServerSocket,
				pspSocketsPool,
				ulMaxMilliSecondsToProcessAnEvent,
				ptSystemTracer) != errNoError)
		#else
			if (((*pspProcessor) [lProcessorIdentifier]).
				init (
				lProcessorIdentifier,
				pcfConfiguration,
				pscScheduler,
				pesEventsSet,
				bIsHTTPProxyEnabled,
				plbHTTPProxyLoadBalancer,
				ulMaxSimultaneousConnectionsToAccept,
				ulMaxDelayAcceptableInLoopInMilliSecs,
				phmFreeSessions,
				pmtFreeSessions,
				psSessions,
				ulMaxConnections,
				ulMaxConnectionTTLInSeconds,
				pssServerSocket,
				pspSocketsPool,
				ulMaxMilliSecondsToProcessAnEvent,
				ptSystemTracer) != errNoError)
		#endif
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_INIT_FAILED);
			ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);

			while (--lProcessorIdentifier >= 0)
			{
				if (((*pspProcessor) [lProcessorIdentifier]).
					finish () != errNoError)
				{
					Error err = ConnectionsManagerErrors (
						__FILE__, __LINE__,
						CM_CMPROCESSOR_FINISH_FAILED);
					ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			delete [] (*pspProcessor);

			return err;
		}
	}


	return errNoError;
}


Error ConnectionsManagerService:: onFinishProcessors (
	CMProcessor_p *pspProcessor,
	unsigned long ulProcessorsNumber,
	Tracer_p ptSystemTracer)

{

	unsigned long			ulProcessorIdentifier;

	for (ulProcessorIdentifier = 0;
		ulProcessorIdentifier < ulProcessorsNumber;
		ulProcessorIdentifier++)
	{
		if (((*pspProcessor) [ulProcessorIdentifier]).
			finish () != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CMPROCESSOR_FINISH_FAILED);
			ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
	}

	delete [] (*pspProcessor);


	return errNoError;
}


Error ConnectionsManagerService:: onWaitProcessors (
	CMProcessor_p pspProcessor,
	unsigned long ulProcessorsNumber,
	Tracer_p ptSystemTracer)

{

	time_t							tUTCNow;
	#ifdef WIN32
		WinThread:: PThreadStatus_t	stRTPThreadState;
	#else
		PosixThread:: PThreadStatus_t	stRTPThreadState;
	#endif


	if ((pspProcessor [
		ulProcessorsNumber - 1]).
		getThreadState (&stRTPThreadState) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
		ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	tUTCNow					= time (NULL);

	#ifdef WIN32
		while (stRTPThreadState == WinThread:: THREADLIB_STARTED ||
			stRTPThreadState == WinThread:: THREADLIB_STARTED_AND_JOINED)
	#else
		while (stRTPThreadState == PosixThread:: THREADLIB_STARTED ||
			stRTPThreadState == PosixThread:: THREADLIB_STARTED_AND_JOINED)
	#endif
	{
		if (time (NULL) - tUTCNow >= 40)
			break;

		#ifdef WIN32
			if (WinThread:: getSleep (1, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (1, 0) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		if ((pspProcessor [
			ulProcessorsNumber - 1]).
			getThreadState (&stRTPThreadState) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
			ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
	}


	return errNoError;
}


Error ConnectionsManagerService:: onCancelProcessors (
	CMProcessor_p pspProcessor,
	unsigned long ulProcessorsNumber,
	Tracer_p ptSystemTracer)

{

	long				lProcessorIdentifier;
	Error_t				errCancel;


	for (lProcessorIdentifier = 0;
		lProcessorIdentifier < (long) ulProcessorsNumber;
		lProcessorIdentifier++)
	{
		if ((errCancel = (pspProcessor [lProcessorIdentifier]).
			cancel ()) != errNoError)
		{
			ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) errCancel, __FILE__, __LINE__);

			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_CANCEL_FAILED);
			ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
	}


	return errNoError;
}


Error ConnectionsManagerService:: onStartProcessors (
	CMProcessor_p pspProcessor,
	unsigned long ulProcessorsNumber,
	ConfigurationFile_p pcfConfiguration,
	Tracer_p ptSystemTracer)

{

	long			lProcessorIdentifier;


	for (lProcessorIdentifier = 0;
		lProcessorIdentifier < (long) ulProcessorsNumber;
		lProcessorIdentifier++)
	{
		if ((pspProcessor [lProcessorIdentifier]).
			start () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_START_FAILED);
			ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			while (--lProcessorIdentifier >= 0)
			{
				if ((pspProcessor [lProcessorIdentifier]).
					cancel () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_CANCEL_FAILED);
					ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			return err;
		}
	}


	return errNoError;
}


Error ConnectionsManagerService:: onStartMyTimes (
	ConfigurationFile_p pcfConfiguration,
	Scheduler_p pscScheduler,
	CMEventsSet_p pesEventsSet,
	Tracer_p ptSystemTracer)

{

	return errNoError;
}


Error ConnectionsManagerService:: onStopMyTimes (
	Tracer_p ptSystemTracer)

{

	return errNoError;
}


Error ConnectionsManagerService:: onStop (void)

{

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_RECEIVEDONSTOP);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	// to finish onStart
	if (_scScheduler. cancel () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_CANCEL_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);
	}

	// to wait that the onStart method finishes
	if (onWaitProcessors (_pspProcessor, _ulProcessorsNumber,
		&_tSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_ONWAITPROCESSORS_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);
	}

	// onStop unblocks the onStart method. The onStart method calls
	// cancel () that calls finishTheService
	/*
	if (finishTheService () != errNoError)
		;		// no trace available, all is closed
	*/

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
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}


	return errNoError;
}


Error ConnectionsManagerService:: finishTheService (void)

{
	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_STOPPING);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (onFinishProcessors (&_pspProcessor, _ulProcessorsNumber,
		&_tSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_ONFINISHPROCESSORS_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);
	}

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_MAINPROCESSORSFINISHED);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (onStopMyTimes (&_tSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_ONSTOPMYTIMES_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);
	}

	if (_bIsHTTPProxyEnabled)
	{
		if (_lbHTTPProxyLoadBalancer. finish () != errNoError)
		{
			Error err = LoadBalancerErrors (__FILE__, __LINE__,
				LB_LOADBALANCER_FINISH_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
	}

	if (_cssCheckUnusedSocketsTimes. finish () != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CHECKUNUSEDSOCKETSTIMES_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_cssCheckServerSocketTimes. finish () != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CHECKSERVERSOCKETTIMES_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_cspCheckSocketsPoolTimes. finish () != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CHECKSOCKETSPOOLTIMES_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	// bDestroyTimes
	if (_scScheduler. finish (false) != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);
	}

	if (_spSocketsPool. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	// The next trace message cannot be moved at the end of this method
	// because the tracer will be already stopped
	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_STOPPED);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (_ssServerSocket. finish () != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SERVERSOCKET_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_mtFreeSessions. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	#ifdef VECTORFREESESSIONS
		_vFreeSessions. clear ();
	#else
		_phmFreeSessions -> clear ();
	#endif


	{
		long				lConnectionIdentifier;

		for (lConnectionIdentifier = 0;
			lConnectionIdentifier < (long) _ulMaxConnections;
			lConnectionIdentifier++)
		{
			if ((_psSessions [lConnectionIdentifier]).
				finish () != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_SESSION_FINISH_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}
	}

	delete [] _psSessions;

	if (onFinishEventsSet (&_pesEventsSet,
		&_tSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_ONFINISHEVENTSSET_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (BaseProcessor:: stopTracer (&_tSystemTracer) != errNoError)
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


Error ConnectionsManagerService:: onStart (void)

{

	Error_t					errJoin;
	Error_t					errScheduler;


	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_STARTING);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	_tServerStartTime						= time (NULL);

	if (onStartProcessors (_pspProcessor, _ulProcessorsNumber,
		&_cfConfiguration, &_tSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_ONSTARTPROCESSORS_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR, (const char *) err,
			__FILE__, __LINE__);

		return err;
	}

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_PROCESSORSTARTED);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (_bIsHTTPProxyEnabled)
	{
		if (_lbHTTPProxyLoadBalancer. start () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_START_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	// start scheduler
	{
		/*
		long l;
		char pppp [1024];
		_scScheduler. getPriority (&l);
		sprintf (pppp, "PRIO: %ld", l);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			pppp, __FILE__, __LINE__);
		_scScheduler. setPriority (1);
		*/
		if (_scScheduler. start () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_START_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);

			if (_bIsHTTPProxyEnabled)
			{
				if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
				{
					Error err = LoadBalancerErrors (__FILE__, __LINE__,
						LB_LOADBALANCER_CANCEL_FAILED);
					_tSystemTracer. trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
				&_tSystemTracer) != errNoError)
			{
				Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
					CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_SCHEDULERSTARTED);
		_tSystemTracer. trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (_cspCheckSocketsPoolTimes. start () != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_START_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);

		if (_scScheduler. cancel () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (_bIsHTTPProxyEnabled)
		{
			if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CANCEL_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (_scScheduler. activeTimes (&_cspCheckSocketsPoolTimes) !=
		errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_ACTIVETIMES_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);

		if (_scScheduler. cancel () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (_bIsHTTPProxyEnabled)
		{
			if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CANCEL_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (_cssCheckServerSocketTimes. start () != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_START_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);

		if (_scScheduler. cancel () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (_bIsHTTPProxyEnabled)
		{
			if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CANCEL_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (_scScheduler. activeTimes (&_cssCheckServerSocketTimes) !=
		errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_ACTIVETIMES_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);

		if (_scScheduler. cancel () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (_bIsHTTPProxyEnabled)
		{
			if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CANCEL_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (_cssCheckUnusedSocketsTimes. start () != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_START_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);

		if (_scScheduler. cancel () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (_bIsHTTPProxyEnabled)
		{
			if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CANCEL_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (_scScheduler. activeTimes (&_cssCheckUnusedSocketsTimes) !=
		errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_ACTIVETIMES_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);

		if (_scScheduler. cancel () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (_bIsHTTPProxyEnabled)
		{
			if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CANCEL_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if (onStartMyTimes (&_cfConfiguration, &_scScheduler,
		_pesEventsSet, &_tSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_ONSTARTMYTIMES_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err,
			__FILE__, __LINE__);

		if (_scScheduler. cancel () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (_bIsHTTPProxyEnabled)
		{
			if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CANCEL_FAILED);
				_tSystemTracer. trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}

		if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

/*
	if (_spSocketsPool. addSocket (
		SocketsPool:: SOCKETSTATUS_READ |
		SocketsPool:: SOCKETSTATUS_EXCEPTION,
		CM_SBSOCKETPOOL_SERVERSOCKET,
		&_ssServerSocket,
		(void *) NULL) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_ADDSOCKET_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_scScheduler. cancel () != errNoError)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (onStopInternalProcessors () != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_SERVICEBROKERPROCESSOR_ONSTOPINTERNALPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}

		if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
			&_tSystemTracer) != errNoError)
		{
			Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}
	*/

	#ifdef WIN32
		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_RUNNING,
				4, _pLocalIPAddress, _ulServerPort,
				// "Not available on Windows", "Windows");
				_pServiceVersion, "Windows");
			_tSystemTracer. trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}
	#else
		{
			Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
				CM_CONNECTIONSMANAGERSERVICE_RUNNING,
				4, _pLocalIPAddress, _ulServerPort,
				_pServiceVersion, "Unix/Linux");
			_tSystemTracer. trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}
	#endif

	// wait of the last thread started.
	if ((errScheduler = _scScheduler. join (&errJoin)) != errNoError)
	{
		if (errScheduler != errNoError)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errScheduler, __FILE__, __LINE__);
		}
	}

	if (errJoin != errNoError)
	{
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) errJoin, __FILE__, __LINE__);

		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_JOIN_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_SHUTDOWN);
		_tSystemTracer. trace (
			Tracer:: TRACER_LINFO, (const char *) msg,
			__FILE__, __LINE__);
	}

	#ifdef WIN32
		if (cancel () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}
	#endif

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_NOTRUNNING);
		_tSystemTracer. trace (
			Tracer:: TRACER_LINFO, (const char *) msg,
			__FILE__, __LINE__);
	}


	return errNoError;
}


Error ConnectionsManagerService:: cancel (void)

{

	Error_t					errCancel;


	if ((errCancel = _scScheduler. cancel ()) != errNoError)
	{
		if ((unsigned long) errCancel != SCH_OPERATION_NOTALLOWED)
		{
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) errCancel,
				__FILE__, __LINE__);

			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SCHEDULER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err,
				__FILE__, __LINE__);
		}
	}

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_SCHEDULERSTOPPED);
		_tSystemTracer. trace (
			Tracer:: TRACER_LINFO, (const char *) msg,
			__FILE__, __LINE__);
	}

/*
	if (_spSocketsPool. deleteSocket (
		&_ssServerSocket, (void **) NULL) !=
		errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_DELETESOCKET_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}
	*/

	if (_bIsHTTPProxyEnabled)
	{
		if (_lbHTTPProxyLoadBalancer. cancel () != errNoError)
		{
			Error err = LoadBalancerErrors (__FILE__, __LINE__,
				LB_LOADBALANCER_CANCEL_FAILED);
			_tSystemTracer. trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
	}

	if (onCancelProcessors (_pspProcessor, _ulProcessorsNumber,
		&_tSystemTracer) != errNoError)
	{
		Error err = ConnectionsManagerErrors (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_ONCANCELPROCESSORS_FAILED);
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	{
		Message msg = ConnectionsManagerMessages (__FILE__, __LINE__,
			CM_CONNECTIONSMANAGERSERVICE_MAINPROCESSORSTOPPED);
		_tSystemTracer. trace (
			Tracer:: TRACER_LINFO, (const char *) msg,
			__FILE__, __LINE__);
	}

	if (finishTheService () != errNoError)
		;		// no trace available, all is closed


	return errNoError;
}


#ifdef WIN32
#else
Error ConnectionsManagerService:: appendStartScriptCommand (
	Buffer_p pbServiceScriptFile)

{

	if (
		pbServiceScriptFile -> append (
			"\tpid=`/sbin/pidof "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_bServiceName. str()
			) != errNoError ||
		pbServiceScriptFile -> append (
			"`" "\n"
			"\tif [ ! \"$pid\" = \"\" ]; then" "\n"
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\t\techo \"The service is already running\"" "\n"
			"\t\treturn 1" "\n"
			"\tfi" "\n"
			) != errNoError ||
		pbServiceScriptFile -> append (
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
			"\t. /usr/local/"
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
		/* _tSystemTracer is not yet initialized
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		return err;
	}


	return errNoError;
}


Error ConnectionsManagerService:: appendStopScriptCommand (
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
			"\t. /usr/local/"
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			".sh" "\n"
			"\n"
			"\n"
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\tpid=`/sbin/pidof "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_bServiceName. str()
			) != errNoError ||
		pbServiceScriptFile -> append (
			"`" "\n"
			"\tif [ \"$pid\" = \"\" ]; then" "\n"
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\t\techo \"The service is not running\"" "\n"
			"\t\treturn 1" "\n"
			"\tfi" "\n"
			) != errNoError ||
		pbServiceScriptFile -> append (
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
		/* _tSystemTracer is not yet initialized
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		return err;
	}


	return errNoError;
}


Error ConnectionsManagerService:: appendStatusScriptCommand (
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
			"\t. /usr/local/"
			) != errNoError ||
		pbServiceScriptFile -> append (
			(const char *) _bServiceName
			) != errNoError ||
		pbServiceScriptFile -> append (
			".sh" "\n"
			"\n"
			) != errNoError ||
		pbServiceScriptFile -> append (
			"\tpid=`/sbin/pidof "
			) != errNoError ||
		pbServiceScriptFile -> append (
			_bServiceName. str()
			) != errNoError ||
		pbServiceScriptFile -> append (
			"`" "\n"
			"\tif [ \"$pid\" = \"\" ]; then" "\n"
			) != errNoError ||
		pbServiceScriptFile -> append (
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
		/* _tSystemTracer is not yet initialized
		_tSystemTracer. trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
		*/

		return err;
	}


	return errNoError;
}
#endif

