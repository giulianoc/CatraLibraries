
#include "HTTPCacheManager.h"
#include "HTTPCacheManagerMessages.h"
#include "FileIO.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#ifdef __QTCOMPILER__
	#include <QDebug>
#endif



#ifdef WIN32
	HTTPCacheManager:: HTTPCacheManager (void): WinThread ()
#else
	HTTPCacheManager:: HTTPCacheManager (void): PosixThread ()
#endif

{

}


HTTPCacheManager:: ~HTTPCacheManager (void)

{

}


Error HTTPCacheManager:: init (
	const char *pCacheDirectory,
	unsigned long ulCacheRetentionInHours,
	unsigned long ulMaxConcurrentHTTPRequest,
	unsigned long ulHTTPRequestsQueueSize,
	const char *pLocalIPAddress,
	unsigned long ulTimeoutToWaitAnswerInSeconds,
	unsigned long ulRetentionInSecondsForHTTPRequestKeys,
	Tracer_p ptSystemTracer)

{

	if (strlen (pLocalIPAddress) >= SCK_MAXHOSTNAMELENGTH)
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_ACTIVATION_WRONG);

		return err;
	}

	_ptSystemTracer			= ptSystemTracer;

	_ulTimeoutToWaitAnswerInSeconds		= ulTimeoutToWaitAnswerInSeconds;
	strcpy (_pLocalIPAddress, pLocalIPAddress);

	_ulRetentionInSecondsForHTTPRequestKeys	=
		ulRetentionInSecondsForHTTPRequestKeys;

	{
		unsigned long				ulHTTPRequestIndex;


		_ulHTTPRequestsQueueSize		= ulHTTPRequestsQueueSize;

		if ((_phrHTTPRequests = new HTTPRequest_t [
			_ulHTTPRequestsQueueSize]) == (HTTPRequest_p) NULL)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_NEW_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		for (ulHTTPRequestIndex = 0;
			ulHTTPRequestIndex < _ulHTTPRequestsQueueSize;
			ulHTTPRequestIndex++)
		{
			(_phrHTTPRequests [ulHTTPRequestIndex]). _ulIdentifier		=
				ulHTTPRequestIndex;

			_vAvailableHTTPRequests. insert (_vAvailableHTTPRequests. end (),
				&(_phrHTTPRequests [ulHTTPRequestIndex]));
		}

		_vWaitingQueueHTTPRequests. clear ();
	}

	{
		_ulMaxConcurrentHTTPRequest		= ulMaxConcurrentHTTPRequest;

		if ((_pshgtSaveHttpGetThread = new CacheSaveHttpGetThread_t [
			_ulMaxConcurrentHTTPRequest]) == (CacheSaveHttpGetThread_p) NULL)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_NEW_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			delete [] _phrHTTPRequests;

			return err;
		}
	}

	if (_bCacheDirectory. init (pCacheDirectory) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		delete [] _pshgtSaveHttpGetThread;

		delete [] _phrHTTPRequests;

		return err;
	}

	#ifdef WIN32
		if (_bCacheDirectory.strip(Buffer::STRIPTYPE_TRAILING, "\\") !=
			errNoError)
	#else
		if (_bCacheDirectory.strip(Buffer::STRIPTYPE_TRAILING, "/") !=
			errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_STRIP_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		delete [] _pshgtSaveHttpGetThread;

		delete [] _phrHTTPRequests;

		return err;
	}

	_ulCacheRetentionInHours				= ulCacheRetentionInHours;

	_tLastTimeRetentionCheckDone			= 0;

	{
		if ((_phHasher = new BufferHasher_t) == (BufferHasher_p) NULL)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_NEW_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			delete [] _pshgtSaveHttpGetThread;

			delete [] _phrHTTPRequests;

			return err;
		}

		if ((_pcComparer = new BufferCmp_t) == (BufferCmp_p) NULL)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_NEW_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			delete _phHasher;

			delete [] _pshgtSaveHttpGetThread;

			delete [] _phrHTTPRequests;

			return err;
		}

		if ((_phmHTTPRequestKeys = new HTTPRequestKeysHashMap_t (
			100, *_phHasher, *_pcComparer)) ==
			(HTTPRequestKeysHashMap_p) NULL)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NEW_FAILED);

			delete _pcComparer;
			delete _phHasher;

			delete [] _pshgtSaveHttpGetThread;

			delete [] _phrHTTPRequests;

			return err;
		}
	}

	if (_mtShutdown. init (PMutex:: MUTEX_RECURSIVE) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		delete _phmHTTPRequestKeys;
		delete _pcComparer;
		delete _phHasher;

		delete [] _pshgtSaveHttpGetThread;

		delete [] _phrHTTPRequests;

		return err;
	}

	if (_mtHTTPRequests. init (PMutex:: MUTEX_RECURSIVE) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtShutdown. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		delete _phmHTTPRequestKeys;
		delete _pcComparer;

		delete _phHasher;

		delete [] _pshgtSaveHttpGetThread;

		delete [] _phrHTTPRequests;

		return err;
	}

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		delete _phmHTTPRequestKeys;
		delete _pcComparer;
		delete _phHasher;

		if (_mtHTTPRequests. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		if (_mtShutdown. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		delete [] _pshgtSaveHttpGetThread;

		delete [] _phrHTTPRequests;

		return err;
	}


	return errNoError;
}


Error HTTPCacheManager:: finish (void)

{

	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_mtHTTPRequests. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	if (_mtShutdown. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	{
		HTTPRequestKeysHashMap_t:: const_iterator		it;
		HTTPRequestKeyInfo_p					phriHTTPRequestKeyInfo;


		for (it = _phmHTTPRequestKeys -> begin ();
			it != _phmHTTPRequestKeys -> end ();
			++it)
		{
			phriHTTPRequestKeyInfo		= it -> second;

			delete phriHTTPRequestKeyInfo;
		}

		_phmHTTPRequestKeys -> clear ();

		delete _phmHTTPRequestKeys;
		delete _pcComparer;
		delete _phHasher;
	}

	{
		unsigned long				ulHTTPRequestIndex;
		Error_t						errCancel;


		for (ulHTTPRequestIndex = 0;
			ulHTTPRequestIndex < _ulMaxConcurrentHTTPRequest;
			ulHTTPRequestIndex++)
		{
			if ((errCancel = (_pshgtSaveHttpGetThread [ulHTTPRequestIndex]).
				cancel ()) != errNoError)
			{
				if ((long) errCancel != THREADLIB_OPERATIONNOTALLOWED)
				{
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) errCancel, __FILE__, __LINE__);

					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_CANCEL_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}
			}

			if ((_pshgtSaveHttpGetThread [ulHTTPRequestIndex]). finish () !=
				errNoError)
			{
				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_SAVEHTTPGETTHREAD_FINISH_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}
		}
	}

	delete [] _pshgtSaveHttpGetThread;

	delete [] _phrHTTPRequests;

	_vAvailableHTTPRequests. clear ();
	_vWaitingQueueHTTPRequests. clear ();


	return errNoError;
}


Error HTTPCacheManager:: run (void)

{

	Boolean_t				bIsShutdown;
	HTTPRequest_p			phrWaitingHTTPRequest;
	#ifdef WIN32
		WinThread:: PThreadStatus_t		stThreadState;
	#else
		PosixThread:: PThreadStatus_t	stThreadState;
	#endif
	unsigned long			ulHTTPRequestIndex;
	Error_t					errHTTPGet;

	std:: vector<HTTPRequest_p>:: iterator			itHTTPRequest;


	bIsShutdown					= false;
	if (setIsShutdown (bIsShutdown) != errNoError)
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_SETISSHUTDOWN_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	while (!bIsShutdown)
	{
		if (_mtHTTPRequests. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (HTTPRequestKeysRetention () != errNoError)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYSRETENTION_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtHTTPRequests. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if (_vWaitingQueueHTTPRequests. size() == 0)
		{
			if (logHTTPRequestsRunning () != errNoError)
			{
				Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
					HCM_HTTPCACHEMANAGER_LOGHTTPREQUESTSRUNNING_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				if (_mtHTTPRequests. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);
				}

				return err;
			}

			if (_mtHTTPRequests. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			#ifdef WIN32
				if (WinThread:: getSleep (5, 0) != errNoError)
			#else
				if (PosixThread:: getSleep (5, 0) != errNoError)
			#endif
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_GETSLEEP_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			if (getIsShutdown (&bIsShutdown) != errNoError)
			{
				Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
					HCM_HTTPCACHEMANAGER_GETISSHUTDOWN_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			continue;
		}

		for (ulHTTPRequestIndex = 0,
			itHTTPRequest = _vWaitingQueueHTTPRequests. begin ();
			itHTTPRequest != _vWaitingQueueHTTPRequests. end ();
			itHTTPRequest = _vWaitingQueueHTTPRequests. begin ())
		{
			phrWaitingHTTPRequest		= (HTTPRequest_p) (*itHTTPRequest);

			for (; ulHTTPRequestIndex < _ulMaxConcurrentHTTPRequest;
				ulHTTPRequestIndex++)
			{
				if ((_pshgtSaveHttpGetThread [ulHTTPRequestIndex]).
					getThreadState (&stThreadState) != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_mtHTTPRequests. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}

				if (stThreadState != THREADLIB_STARTED &&
					stThreadState != THREADLIB_STARTED_AND_JOINED)
					break;
			}

			if (ulHTTPRequestIndex == _ulMaxConcurrentHTTPRequest)
			{
				// max concurrency reached

				{
					Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
						HCM_HTTPCACHEMANAGER_REACHEDMAXCONCURRENCY,
						1, _ulMaxConcurrentHTTPRequest);
					_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
						(const char *) msg, __FILE__, __LINE__);
				}

				break;
			}

			{
				Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
					HCM_HTTPCACHEMANAGER_HTTPGETINITIALIZATION,
					7,
					phrWaitingHTTPRequest -> _ulIdentifier,
					(phrWaitingHTTPRequest -> _bHTTPHost). str(),
					(unsigned long) (phrWaitingHTTPRequest -> _lHTTPPort),
					_pLocalIPAddress,
					(const char *) (phrWaitingHTTPRequest -> _bURI),
					(const char *) (phrWaitingHTTPRequest -> _bURLParameters),
					_ulTimeoutToWaitAnswerInSeconds);
				_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
					(const char *) msg, __FILE__, __LINE__);
			}

			if ((errHTTPGet = (_pshgtSaveHttpGetThread [ulHTTPRequestIndex]).
				init (
				phrWaitingHTTPRequest -> _ulIdentifier,
				_ptSystemTracer,
				&(phrWaitingHTTPRequest -> _bHTTPLocalPathName), false,
				(phrWaitingHTTPRequest -> _bHTTPHost). str(),
				(unsigned long) (phrWaitingHTTPRequest -> _lHTTPPort),
				(const char *) (phrWaitingHTTPRequest -> _bURI),
				(const char *) (phrWaitingHTTPRequest -> _bURLParameters),
				(const char *) NULL,        // pCookie
				"CMS Client",           // User-Agent
				_ulTimeoutToWaitAnswerInSeconds,
				0,
				_ulTimeoutToWaitAnswerInSeconds,
				0,
				_pLocalIPAddress
				)) != errNoError)
			{
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) errHTTPGet, __FILE__, __LINE__);

				Error err = WebToolsErrors (__FILE__, __LINE__,
					WEBTOOLS_SAVEHTTPGETTHREAD_INIT_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				break;
			}

			if ((_pshgtSaveHttpGetThread [ulHTTPRequestIndex]).
				start (false) != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_START_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				break;
			}

			{
				Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
					HCM_HTTPCACHEMANAGER_HTTPREQUESTRUNNING,
					5,
					phrWaitingHTTPRequest -> _ulIdentifier,
					(phrWaitingHTTPRequest -> _bHTTPHost). str(),
					(unsigned long) (phrWaitingHTTPRequest -> _lHTTPPort),
					(const char *) (phrWaitingHTTPRequest -> _bURI),
					(const char *) (phrWaitingHTTPRequest -> _bURLParameters));
				_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
					(const char *) msg, __FILE__, __LINE__);
			}

			_vWaitingQueueHTTPRequests. erase (
				_vWaitingQueueHTTPRequests. begin ());
			_vAvailableHTTPRequests. insert (_vAvailableHTTPRequests. end (),
				phrWaitingHTTPRequest);

			ulHTTPRequestIndex++;
		}

		if (logHTTPRequestsRunning () != errNoError)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_LOGHTTPREQUESTSRUNNING_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtHTTPRequests. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if (logHTTPRequestsWaiting () != errNoError)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_LOGHTTPREQUESTSWAITING_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtHTTPRequests. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (getIsShutdown (&bIsShutdown) != errNoError)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_GETISSHUTDOWN_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}
	}


	return errNoError;
}


Error HTTPCacheManager:: getLocalPathNameAndCacheContent (
	Buffer_p pbHTTPRequest,
	Buffer_p pbHTTPRequestKey,
	Buffer_p pbHTTPLocalPathName,
	Boolean_p pbAlreadyInCache)

{

	HTTPRequest_p			phrLocalHTTPRequest;
	unsigned long			ulHTTPRequestIndex;
	std:: vector<HTTPRequest_p>:: const_iterator		itHTTPRequest;

	char					pReceivedHTTPHost [SCK_MAXHOSTNAMELENGTH];
	long					lReceivedHTTPPort;
	Buffer_t				bReceivedURI;
	Buffer_t				bReceivedURLParameters;

	char					pLocalHTTPHost [SCK_MAXHOSTNAMELENGTH];
	long					lLocalHTTPPort;
	Buffer_t				bLocalURI;
	Buffer_t				bLocalURLParameters;


	if (getLocalPathName (pbHTTPRequest, pbHTTPLocalPathName) != errNoError)
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_GETLOCALPATHNAME_FAILED,
			1, (const char *) (*pbHTTPRequest));
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (isLocalPathNamePresentInCache (pbHTTPLocalPathName,
		pbAlreadyInCache) != errNoError)
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_ISLOCALPATHNAMEPRESENTINCACHE_FAILED,
			1, (const char *) (*pbHTTPLocalPathName));
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (*pbAlreadyInCache)
	{
		{
			Message msg = HTTPCacheManagerMessages (
				__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_ALREADYINCACHE,
				3,
				(const char *) (*pbHTTPRequest),
				(const char *) (*pbHTTPLocalPathName));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) msg, __FILE__, __LINE__);
		}

		// saving the association (pbHTTPRequestKey, pbHTTPRequest)
		// if pbHTTPRequestKey is present
		if (pbHTTPRequestKey != (Buffer_p) NULL &&
			!(pbHTTPRequestKey -> isEmpty ()))
		{
			if (saveHTTPRequestKey (pbHTTPRequestKey, pbHTTPRequest, true) !=
				errNoError)
			{
				Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
					HCM_HTTPCACHEMANAGER_SAVEHTTPREQUESTKEY_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		}

		return errNoError;
	}

	if (WebUtility:: parseURL ((const char *) (*pbHTTPRequest),
		pReceivedHTTPHost, SCK_MAXHOSTNAMELENGTH,
		&lReceivedHTTPPort, &bReceivedURI,
		&bReceivedURLParameters) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_PARSEURL_FAILED,
			1, (const char *) (*pbHTTPRequest));
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (_mtHTTPRequests. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	for (ulHTTPRequestIndex = 0;
		ulHTTPRequestIndex < _ulMaxConcurrentHTTPRequest;
		ulHTTPRequestIndex++)
	{
		if ((_pshgtSaveHttpGetThread [ulHTTPRequestIndex]).
			getInputDetails (pLocalHTTPHost, SCK_MAXHOSTNAMELENGTH,
				&lLocalHTTPPort, &bLocalURI) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtHTTPRequests. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}

		if (!strcmp ((const char *) bReceivedURI, (const char *) bLocalURI))
		{
			{
				Message msg = HTTPCacheManagerMessages (
					__FILE__, __LINE__,
					HCM_HTTPCACHEMANAGER_HTTPGETALREADYRUNNING,
					3,
					pLocalHTTPHost,
					(unsigned long) (lLocalHTTPPort),
					(const char *) (bLocalURI));
				_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
					(const char *) msg, __FILE__, __LINE__);
			}

			// saving the association (pbHTTPRequestKey, pbHTTPRequest)
			// if pbHTTPRequestKey is present
			if (pbHTTPRequestKey != (Buffer_p) NULL &&
				!(pbHTTPRequestKey -> isEmpty ()))
			{
				if (saveHTTPRequestKey (pbHTTPRequestKey, pbHTTPRequest,
					true) != errNoError)
				{
					Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
						HCM_HTTPCACHEMANAGER_SAVEHTTPREQUESTKEY_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_mtHTTPRequests. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}
			}

			if (_mtHTTPRequests. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return errNoError;
		}
	}

	for (itHTTPRequest = _vWaitingQueueHTTPRequests. begin ();
		itHTTPRequest != _vWaitingQueueHTTPRequests. end ();
		itHTTPRequest++)
	{
		phrLocalHTTPRequest		= (HTTPRequest_p) (*itHTTPRequest);

		if (!strcmp ((const char *) bReceivedURI,
			(const char *) (phrLocalHTTPRequest -> _bURI)))
		{
			{
				Message msg = HTTPCacheManagerMessages (
					__FILE__, __LINE__,
					HCM_HTTPCACHEMANAGER_HTTPGETALREADYINWAITINGQUEUE,
					3,
					(phrLocalHTTPRequest -> _bHTTPHost). str(),
					(unsigned long) (phrLocalHTTPRequest -> _lHTTPPort),
					(const char *) (phrLocalHTTPRequest -> _bURI));
				_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
					(const char *) msg, __FILE__, __LINE__);
			}

			// saving the association (pbHTTPRequestKey, pbHTTPRequest)
			// if pbHTTPRequestKey is present
			if (pbHTTPRequestKey != (Buffer_p) NULL &&
				!(pbHTTPRequestKey -> isEmpty ()))
			{
				if (saveHTTPRequestKey (pbHTTPRequestKey, pbHTTPRequest,
					true) != errNoError)
				{
					Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
						HCM_HTTPCACHEMANAGER_SAVEHTTPREQUESTKEY_FAILED);
					_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
						(const char *) err, __FILE__, __LINE__);

					if (_mtHTTPRequests. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
						_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
							(const char *) err, __FILE__, __LINE__);
					}

					return err;
				}
			}

			if (_mtHTTPRequests. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return errNoError;
		}
	}

	if (_vAvailableHTTPRequests. size() == 0)
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_NOMOREHTTPREQUESTAVAILABLE,
			1, _ulHTTPRequestsQueueSize);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	phrLocalHTTPRequest		= *(_vAvailableHTTPRequests. begin ());

	phrLocalHTTPRequest -> _lHTTPPort		= lReceivedHTTPPort;

	if ((phrLocalHTTPRequest -> _bHTTPHost). setBuffer (pReceivedHTTPHost) !=
		errNoError ||
		(phrLocalHTTPRequest -> _bHTTPRequestURL). setBuffer (
		(const char *) (*pbHTTPRequest)) != errNoError ||
		(phrLocalHTTPRequest -> _bHTTPLocalPathName). setBuffer (
			(const char *) (*pbHTTPLocalPathName)) != errNoError ||
		(phrLocalHTTPRequest -> _bURI). setBuffer (
			(const char *) bReceivedURI) != errNoError ||
		(phrLocalHTTPRequest -> _bURLParameters). setBuffer (
			(const char *) bReceivedURLParameters) != errNoError
		)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	{
		Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_HTTPREQUESTWAITINGTOBEUPLOADED,
			5,
			phrLocalHTTPRequest -> _ulIdentifier,
			(phrLocalHTTPRequest -> _bHTTPHost). str (),
			(unsigned long) (phrLocalHTTPRequest -> _lHTTPPort),
			(const char *) (phrLocalHTTPRequest -> _bURI),
			(const char *) (phrLocalHTTPRequest -> _bURLParameters));
		_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
			(const char *) msg, __FILE__, __LINE__);
	}

	_vAvailableHTTPRequests. erase (_vAvailableHTTPRequests. begin ());
	_vWaitingQueueHTTPRequests. insert (_vWaitingQueueHTTPRequests. end (),
		phrLocalHTTPRequest);
	
	// saving the association (pbHTTPRequestKey, pbHTTPRequest)
	// if pbHTTPRequestKey is present
	if (pbHTTPRequestKey != (Buffer_p) NULL &&
		!(pbHTTPRequestKey -> isEmpty ()))
	{
		if (saveHTTPRequestKey (pbHTTPRequestKey, pbHTTPRequest,
			true) != errNoError)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_SAVEHTTPREQUESTKEY_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			if (_mtHTTPRequests. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);
			}

			return err;
		}
	}

	if (_mtHTTPRequests. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error HTTPCacheManager:: getLocalPathNameUsingHTTPRequestKey (
	Buffer_p pbHTTPRequestKey,
	Buffer_p pbHTTPLocalPathName,
	Boolean_p pbAlreadyInCache)

{

	HTTPRequestKeysHashMap_t:: iterator		it;
	HTTPRequestKeyInfo_p					phriHTTPRequestKeyInfo;
	Error_t									errGet;


	if (_mtHTTPRequests. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	it		= _phmHTTPRequestKeys -> find (pbHTTPRequestKey);

	if (it == _phmHTTPRequestKeys -> end ())
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYNOTFOUND,
			1, (const char *) (*pbHTTPRequestKey));
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6,
			(const char *) err, __FILE__, __LINE__);

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	phriHTTPRequestKeyInfo		= it -> second;

	{
		Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_FOUNDHTTPREQUESTKEY,
			3, (const char *) (phriHTTPRequestKeyInfo -> _bHTTPRequestKey),
			(const char *) (phriHTTPRequestKeyInfo -> _bHTTPRequest),
			phriHTTPRequestKeyInfo -> _bIsHTTPRequestSuccess);
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (!(phriHTTPRequestKeyInfo -> _bIsHTTPRequestSuccess))
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYISMARKEDASFAILED,
			1, (const char *) (*pbHTTPRequestKey));
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6,
			(const char *) err, __FILE__, __LINE__);

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if ((errGet = getLocalPathNameAndCacheContent (
		&(phriHTTPRequestKeyInfo -> _bHTTPRequest),
		pbHTTPRequestKey,
		pbHTTPLocalPathName,
		pbAlreadyInCache)) != errNoError)
	{
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) errGet, __FILE__, __LINE__);

		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_GETLOCALPATHNAMEANDCACHECONTENT_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errGet;
	}

	if (_mtHTTPRequests. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error HTTPCacheManager:: markHTTPRequestKeyAsFailed (
	Buffer_p pbHTTPRequestKey)

{

	Buffer_t		bHTTPRequest;


	return saveHTTPRequestKey (pbHTTPRequestKey,
		&bHTTPRequest, false);
}


Error HTTPCacheManager:: saveHTTPRequestKey (
	Buffer_p pbHTTPRequestKey,
	Buffer_p pbHTTPRequest,
	Boolean_t bIsHTTPRequestSuccess)

{

	int							iDidInsert;
	HTTPRequestKeyInfo_p		phriHTTPRequestKeyInfo;


	if (_mtHTTPRequests. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if ((phriHTTPRequestKeyInfo = new HTTPRequestKeyInfo_t) ==
		(HTTPRequestKeyInfo_p) NULL)
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_NEW_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	if ((phriHTTPRequestKeyInfo -> _bHTTPRequestKey). setBuffer (
		(const char *) (*pbHTTPRequestKey)) != errNoError ||
		(phriHTTPRequestKeyInfo -> _bHTTPRequest). setBuffer (
			(const char *) (*pbHTTPRequest)) != errNoError
		)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		delete phriHTTPRequestKeyInfo;

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return err;
	}

	phriHTTPRequestKeyInfo -> _bIsHTTPRequestSuccess	= bIsHTTPRequestSuccess;

	phriHTTPRequestKeyInfo -> _tTimestamp				= time (NULL);

	_phmHTTPRequestKeys -> InsertWithoutDuplication (
		&(phriHTTPRequestKeyInfo -> _bHTTPRequestKey),
		phriHTTPRequestKeyInfo, &iDidInsert);

	if (iDidInsert == 0)
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYALREADYPRESENT,
			1, (const char *) (phriHTTPRequestKeyInfo -> _bHTTPRequestKey));
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
			(const char *) err, __FILE__, __LINE__);

		delete phriHTTPRequestKeyInfo;

		if (_mtHTTPRequests. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}

		return errNoError;
	}
	else
	{
		Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYADDEDTOHASHMAP,
			2, (const char *) (phriHTTPRequestKeyInfo -> _bHTTPRequestKey),
			(const char *) (phriHTTPRequestKeyInfo -> _bHTTPRequest));
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
			(const char *) msg, __FILE__, __LINE__);
	}

	_mmHTTPRequestKeys. insert (
		std:: pair<time_t, HTTPRequestKeyInfo_p>(
		phriHTTPRequestKeyInfo -> _tTimestamp, phriHTTPRequestKeyInfo));

	{
		Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYADDEDTOMULTIMAP,
			2, (const char *) (phriHTTPRequestKeyInfo -> _bHTTPRequestKey),
			(const char *) (phriHTTPRequestKeyInfo -> _bHTTPRequest));
		_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6,
			(const char *) msg, __FILE__, __LINE__);
	}

	if (_mtHTTPRequests. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error HTTPCacheManager:: getLocalPathName (
	Buffer_p pbHTTPRequest,
	Buffer_p pbHTTPLocalPathName)

{
	Buffer_t				bURLParameters;
	char					pHTTPHost [SCK_MAXHOSTNAMELENGTH];
	long					lHTTPPort;
	Buffer_t				bURI;
	const char				*pLastSlash;


	if (WebUtility:: parseURL ((const char *) (*pbHTTPRequest),
		pHTTPHost, SCK_MAXHOSTNAMELENGTH,
		&lHTTPPort, &bURI, &bURLParameters) != errNoError)
	{
		Error err = WebToolsErrors (__FILE__, __LINE__,
			WEBTOOLS_WEBUTILITY_PARSEURL_FAILED,
			1, (const char *) (*pbHTTPRequest));
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	#ifdef WIN32
		if (bURI. substitute("/", "\\") != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_SETBUFFER_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if ((pLastSlash = strrchr (bURI. str(), '\\')) ==
			(const char *) NULL)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_WRONGURI,
				1, (const char *) bURI);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}
	#else
		if ((pLastSlash = strrchr (bURI. str(), '/')) ==
			(const char *) NULL)
		{
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_WRONGURI,
				1, (const char *) bURI);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}
	#endif

	if (pbHTTPLocalPathName -> setBuffer (
		_bCacheDirectory. str()) != errNoError ||
		pbHTTPLocalPathName -> append (bURI. str(),
			pLastSlash - bURI. str()) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	#ifdef WIN32
		if (FileIO:: createDirectory ((const char *) (*pbHTTPLocalPathName),
	 		0, true, true) != errNoError)
	#else
		if (FileIO:: createDirectory ((const char *) (*pbHTTPLocalPathName),
	 		S_IRUSR | S_IWUSR | S_IXUSR |
			S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH, true, true) !=
			errNoError)
	#endif
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_CREATEDIRECTORY_FAILED,
			1, (const char *) (*pbHTTPLocalPathName));
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (pbHTTPLocalPathName -> append (pLastSlash) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_APPEND_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error HTTPCacheManager:: isLocalPathNamePresentInCache (
	Buffer_p pbHTTPLocalPathName,
	Boolean_p pbAlreadyInCache)

{
	Buffer_t			bCompletedHTTPLocalPathName;


	if (bCompletedHTTPLocalPathName. setBuffer (
		(const char *) (*pbHTTPLocalPathName)) != errNoError ||
		bCompletedHTTPLocalPathName. append (
			".complete") != errNoError
		)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_SETBUFFER_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (FileIO:: isFileExisting ((const char *) bCompletedHTTPLocalPathName,
		pbAlreadyInCache) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_FILEIO_ISFILEEXISTING_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}


	return errNoError;
}


Error HTTPCacheManager:: logHTTPRequestsRunning (void)

{

	unsigned long			ulHTTPRequestIndex;
	#ifdef WIN32
		WinThread:: PThreadStatus_t		stThreadState;
	#else
		PosixThread:: PThreadStatus_t	stThreadState;
	#endif
	char					pHTTPHost [SCK_MAXHOSTNAMELENGTH];
	long					lHTTPPort;
	Buffer_t				bURI;


	for (ulHTTPRequestIndex = 0;
		ulHTTPRequestIndex < _ulMaxConcurrentHTTPRequest;
		ulHTTPRequestIndex++)
	{
		if ((_pshgtSaveHttpGetThread [ulHTTPRequestIndex]).
			getThreadState (&stThreadState) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (stThreadState == THREADLIB_STARTED &&
			stThreadState == THREADLIB_STARTED_AND_JOINED)
		{
			if ((_pshgtSaveHttpGetThread [ulHTTPRequestIndex]).
				getInputDetails (pHTTPHost, SCK_MAXHOSTNAMELENGTH,
					&lHTTPPort, &bURI) != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}

			{
				Message msg = HTTPCacheManagerMessages (
					__FILE__, __LINE__,
					HCM_HTTPCACHEMANAGER_HTTPGETRUNNING,
					3,
					pHTTPHost,
					(unsigned long) (lHTTPPort),
					bURI. str());
				_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
					(const char *) msg, __FILE__, __LINE__);
			}
		}
	}

	return errNoError;
}


Error HTTPCacheManager:: logHTTPRequestsWaiting (void)

{

	HTTPRequest_p		phrLocalHTTPRequest;
	std:: vector<HTTPRequest_p>:: const_iterator		itHTTPRequest;


	for (itHTTPRequest = _vWaitingQueueHTTPRequests. begin ();
		itHTTPRequest != _vWaitingQueueHTTPRequests. end ();
		itHTTPRequest++)
	{
		phrLocalHTTPRequest		= (HTTPRequest_p) (*itHTTPRequest);

		{
			Message msg = HTTPCacheManagerMessages (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_HTTPGETWAITING,
				1, (const char *)
					(phrLocalHTTPRequest -> _bHTTPRequestURL));
			_ptSystemTracer -> trace (Tracer:: TRACER_LINFO,
				(const char *) msg, __FILE__, __LINE__);
		}
	}


	return errNoError;
}


// RequestKeys hashmap and multimap retention
Error HTTPCacheManager:: HTTPRequestKeysRetention (void)

{

	HTTPRequestKeysMultiMap_t:: iterator		itHTTPRequestKeys;
	HTTPRequestKeyInfo_p		phriHTTPRequestKeyInfo;
	Buffer_t					pbLocalHTTPRequestKey;
	unsigned long				ulHTTPRequestKeysNumberToBeRemoved;
	unsigned long				ulHTTPRequestKeyIndexToBeRemoved;
	time_t						tNowUTCInSecs;


	tNowUTCInSecs							= time (NULL);
	ulHTTPRequestKeysNumberToBeRemoved		= 0;

	for (itHTTPRequestKeys = _mmHTTPRequestKeys. begin ();
		itHTTPRequestKeys != _mmHTTPRequestKeys. end ();
		++itHTTPRequestKeys)
	{
		phriHTTPRequestKeyInfo		= itHTTPRequestKeys -> second;

		if (tNowUTCInSecs - phriHTTPRequestKeyInfo -> _tTimestamp >
			_ulRetentionInSecondsForHTTPRequestKeys)
		{
			ulHTTPRequestKeysNumberToBeRemoved++;	
		}
		else
		{
			break;
		}
	}

	if (_mmHTTPRequestKeys. size() != _phmHTTPRequestKeys -> size())
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_HASHMAPANDMULTIMAPNOTCONSISTENT,
			2,
			(unsigned long) _phmHTTPRequestKeys -> size(),
			(unsigned long) _mmHTTPRequestKeys. size());
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);
	}

	for (ulHTTPRequestKeyIndexToBeRemoved = 0;
		ulHTTPRequestKeyIndexToBeRemoved <
			ulHTTPRequestKeysNumberToBeRemoved;
		ulHTTPRequestKeyIndexToBeRemoved++)
	{
		phriHTTPRequestKeyInfo		=
			_mmHTTPRequestKeys. begin() -> second;

		_mmHTTPRequestKeys. erase (_mmHTTPRequestKeys. begin());

		{
			Message msg = HTTPCacheManagerMessages (
				__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYREMOVEDFROMMULTIMAP,
				2, (const char *)
					(phriHTTPRequestKeyInfo -> _bHTTPRequestKey),
				(const char *)
					(phriHTTPRequestKeyInfo -> _bHTTPRequest));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG6,
				(const char *) msg, __FILE__, __LINE__);
		}

		if (!_phmHTTPRequestKeys -> Delete (
			&(phriHTTPRequestKeyInfo -> _bHTTPRequestKey)))
		{
			// false means the key was not found and
			// the table was not modified. That should not happen
			Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
				HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYNOTFOUND,
				1, (const char *)
					(phriHTTPRequestKeyInfo -> _bHTTPRequestKey));
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);
		}
		else
		{
			Message msg = HTTPCacheManagerMessages (
				__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_HTTPREQUESTKEYREMOVEDFROMHASHMAP,
				2, (const char *)
					(phriHTTPRequestKeyInfo -> _bHTTPRequestKey),
				(const char *)
					(phriHTTPRequestKeyInfo -> _bHTTPRequest));
			_ptSystemTracer -> trace (Tracer:: TRACER_LDBG5,
				(const char *) msg, __FILE__, __LINE__);
		}

		delete phriHTTPRequestKeyInfo;
	}


	return errNoError;
}


Error HTTPCacheManager:: cancel (void)

{

	time_t							tUTCNow;
	#ifdef WIN32
		WinThread:: PThreadStatus_t		stThreadState;
	#else
		PosixThread:: PThreadStatus_t	stThreadState;
	#endif


	if (setIsShutdown (true) != errNoError)
	{
		Error err = HTTPCacheManagerErrors (__FILE__, __LINE__,
			HCM_HTTPCACHEMANAGER_SETISSHUTDOWN_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	if (getThreadState (&stThreadState) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	tUTCNow					= time (NULL);

	while (stThreadState == THREADLIB_STARTED ||
		stThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		if (time (NULL) - tUTCNow >= 10)
			break;

		#ifdef WIN32
			if (WinThread:: getSleep (1, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (1, 0) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}

		if (getThreadState (&stThreadState) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);
			_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
				(const char *) err, __FILE__, __LINE__);

			return err;
		}
	}

	if (stThreadState == THREADLIB_STARTED ||
		stThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		#ifdef WIN32
			// no cancel for windows thread
		#else
			if (PosixThread:: cancel () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_CANCEL_FAILED);
				_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
					(const char *) err, __FILE__, __LINE__);

				return err;
			}
		#endif
	}


	return errNoError;
}


Error HTTPCacheManager:: getIsShutdown (
	Boolean_p pbIsShutdown)

{

	if (_mtShutdown. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	*pbIsShutdown				= _bIsShutdown;

	if (_mtShutdown. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	return errNoError;
}


Error HTTPCacheManager:: setIsShutdown (
	Boolean_t bIsShutdown)

{

	if (_mtShutdown. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	_bIsShutdown			= bIsShutdown;

	if (_mtShutdown. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);
		_ptSystemTracer -> trace (Tracer:: TRACER_LERRR,
			(const char *) err, __FILE__, __LINE__);

		return err;
	}

	return errNoError;
}


