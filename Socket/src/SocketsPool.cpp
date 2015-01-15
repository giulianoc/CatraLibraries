/*
 Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

 This program is free software; you can redistribute it and/or 
 modify it under the terms of the GNU General Public License 
 as published by the Free Software Foundation; either 
 version 2 of the License, or (at your option) any later 
 version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 Commercial use other than under the terms of the GNU General Public
 License is allowed only after express negotiation of conditions
 with the authors.
*/


//	If it is used the select function, if you want to increase the number
//	of file descriptors, for Windows, is sufficient
//	define again FD_SETSIZE inside this file.
//	For linux system it is necessary to use the poll function
#define FD_SETSIZE			1024 * 4

#include "SocketsPool.h"
#include "DateTime.h"
#if defined(WIN32)
	#include <io.h>
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2spi.h>
#else
	#ifdef __QTCOMPILER__
		#define HAVE_POLL
	#else
		#include <CatraLibrariesConfig.h>
	#endif

	#ifdef HAVE_POLL
		#include <sys/poll.h>
	#endif

	#include <sys/socket.h>
	#include <sys/time.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
#endif
#ifdef WIN32
	#include "WinThread.h"
#else
	#include "PosixThread.h"
#endif
#include "FileIO.h"
#include <assert.h>



#ifdef WIN32
	SocketsPool:: SocketsPool (void): WinThread ()
#else
	SocketsPool:: SocketsPool (void): PosixThread ()
#endif

{

	_ssSocketsPoolStatus			= SOCKETSPOOL_BUILDED;

}


SocketsPool:: ~SocketsPool (void)

{

}


SocketsPool:: SocketsPool (const SocketsPool &t)

{
	assert (1 == 0);

}


Error SocketsPool:: init (
	unsigned long ulMaxSocketsNumber,
	unsigned long ulCheckSocketsPoolPeriodInSeconds,
	unsigned long ulAdditionalCheckSocketsPoolPeriodInMilliSeconds,
	Boolean_t bAllowDeletionSocketFromUpdateMethod)

{

	unsigned long				ulSocketInfoIndex;


	if (_ssSocketsPoolStatus != SOCKETSPOOL_BUILDED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	_ulMaxSocketsNumber			= ulMaxSocketsNumber;
	_ulCheckSocketsPoolPeriodInSeconds	=
		ulCheckSocketsPoolPeriodInSeconds;
	_ulAdditionalCheckSocketsPoolPeriodInMilliSeconds	=
		ulAdditionalCheckSocketsPoolPeriodInMilliSeconds;
	_bAllowDeletionSocketFromUpdateMethod	=
		bAllowDeletionSocketFromUpdateMethod;

	if ((_psiSocketsInfo = new SocketInfo_t [_ulMaxSocketsNumber]) ==
		(SocketInfo_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		return err;
	}

	#ifdef HAVE_POLL
		if ((_ppfDescriptors = new struct pollfd [_ulMaxSocketsNumber]) ==
			(struct pollfd *) NULL)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_NEW_FAILED);

			delete [] _psiSocketsInfo;

			return err;
		}
	#endif

	if ((_phHasher = new IntHasher_t) ==
		(IntHasher_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		#ifdef HAVE_POLL
			delete [] ((struct pollfd *) _ppfDescriptors);
		#endif
		delete [] _psiSocketsInfo;

		return err;
	}

	if ((_pcComparer = new IntCmp_t) ==
		(IntCmp_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		delete _phHasher;
		#ifdef HAVE_POLL
			delete [] ((struct pollfd *) _ppfDescriptors);
		#endif
		delete [] _psiSocketsInfo;

		return err;
	}

	if ((_psiSocketsInfoSetByFileDescriptor = new SocketsInfoHashMap_t (
		100, *_phHasher, *_pcComparer)) ==
		(SocketsPool:: SocketsInfoHashMap_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		delete _pcComparer;
		delete _phHasher;
		#ifdef HAVE_POLL
			delete [] ((struct pollfd *) _ppfDescriptors);
		#endif
		delete [] _psiSocketsInfo;

		return err;
	}

	if ((_psiSocketsInfoSetBySocketPort = new SocketsInfoHashMap_t (
		100, *_phHasher, *_pcComparer)) ==
		(SocketsPool:: SocketsInfoHashMap_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		delete _psiSocketsInfoSetByFileDescriptor;
		delete _pcComparer;
		delete _phHasher;
		#ifdef HAVE_POLL
			delete [] ((struct pollfd *) _ppfDescriptors);
		#endif
		delete [] _psiSocketsInfo;

		return err;
	}

	#ifdef _REENTRANT
		#if defined(__hpux) && defined(_CMA__HP)
			if (_mtSocketsPool. init (PMutex:: MUTEX_FAST) !=
		#else	// POSIX
			#if defined(__CYGWIN__)
				if (_mtSocketsPool. init (PMutex:: MUTEX_RECURSIVE) !=
			#else							// POSIX.1-1996 standard (HPUX 11)
				if (_mtSocketsPool. init (PMutex:: MUTEX_RECURSIVE) !=
			#endif
		#endif
			errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);

			delete _psiSocketsInfoSetBySocketPort;
			delete _psiSocketsInfoSetByFileDescriptor;
			delete _pcComparer;
			delete _phHasher;
			#ifdef HAVE_POLL
				delete [] ((struct pollfd *) _ppfDescriptors);
			#endif
			delete [] _psiSocketsInfo;

			return err;
		}
	#endif

	#ifdef _REENTRANT
		#if defined(__hpux) && defined(_CMA__HP)
			if (_mtShutdown. init (PMutex:: MUTEX_FAST) !=
		#else	// POSIX
			#if defined(__CYGWIN__)
				if (_mtShutdown. init (PMutex:: MUTEX_RECURSIVE) !=
			#else							// POSIX.1-1996 standard (HPUX 11)
				if (_mtShutdown. init (PMutex:: MUTEX_FAST) !=
			#endif
		#endif
			errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_INIT_FAILED);

			#ifdef _REENTRANT
				if (_mtSocketsPool. finish () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_FINISH_FAILED);
				}
			#endif

			delete _psiSocketsInfoSetBySocketPort;
			delete _psiSocketsInfoSetByFileDescriptor;
			delete _pcComparer;
			delete _phHasher;
			#ifdef HAVE_POLL
				delete [] ((struct pollfd *) _ppfDescriptors);
			#endif
			delete [] _psiSocketsInfo;

			return err;
		}
	#endif

	for (ulSocketInfoIndex = 0; ulSocketInfoIndex < _ulMaxSocketsNumber;
		ulSocketInfoIndex++)
	{
		_vFreeSocketsInfo. insert (_vFreeSocketsInfo. end (),
			&(_psiSocketsInfo [ulSocketInfoIndex]));
	}

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);

		_vFreeSocketsInfo. clear ();
		#ifdef _REENTRANT
			if (_mtShutdown. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}

			if (_mtSocketsPool. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_FINISH_FAILED);
			}
		#endif

		delete _psiSocketsInfoSetBySocketPort;
		delete _psiSocketsInfoSetByFileDescriptor;
		delete _pcComparer;
		delete _phHasher;
		#ifdef HAVE_POLL
			delete [] ((struct pollfd *) _ppfDescriptors);
		#endif
		delete [] _psiSocketsInfo;

		return err;
	}

	_ssSocketsPoolStatus			= SOCKETSPOOL_INITIALIZED;


	return errNoError;
}


Error SocketsPool:: finish (void)

{

	if (_ssSocketsPoolStatus != SOCKETSPOOL_INITIALIZED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	#ifdef WIN32
		if (WinThread:: finish () != errNoError)
	#else
		if (PosixThread:: finish () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_FINISH_FAILED);

		return err;
	}

	#ifdef _REENTRANT
		if (_mtShutdown. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}

		if (_mtSocketsPool. finish () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_FINISH_FAILED);
		}
	#endif

	_psiSocketsInfoSetBySocketPort -> clear ();
	_psiSocketsInfoSetByFileDescriptor -> clear ();
	_vFreeSocketsInfo. clear ();

	delete _psiSocketsInfoSetBySocketPort;
	delete _psiSocketsInfoSetByFileDescriptor;
	delete _pcComparer;
	delete _phHasher;
	#ifdef HAVE_POLL
		delete [] ((struct pollfd *) _ppfDescriptors);
	#endif
	delete [] _psiSocketsInfo;

	_ssSocketsPoolStatus			= SOCKETSPOOL_BUILDED;


	return errNoError;
}


Error SocketsPool:: run (void)

{

	Boolean_t			bIsShutdown;
	Error_t				errcheckSocketsStatus;


	bIsShutdown					= false;

	if (setIsShutdown (bIsShutdown) != errNoError)
	{
		_erThreadReturn = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_SETISSHUTDOWN_FAILED);

		return _erThreadReturn;
	}

	while (!bIsShutdown)
	{
		if ((errcheckSocketsStatus = checkSocketsStatus (
			_ulCheckSocketsPoolPeriodInSeconds,
			_ulAdditionalCheckSocketsPoolPeriodInMilliSeconds)) != errNoError)
		{
			if ((long) errcheckSocketsStatus !=
				SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED &&
				(long) errcheckSocketsStatus !=
				SCK_SOCKETSPOOL_POOLEMPTY)
			{
				_erThreadReturn			= errcheckSocketsStatus;
				// _erThreadReturn = SocketErrors (__FILE__, __LINE__,
				// 	SCK_SOCKETSPOOL_CHECKSOCKETSSTATUS_FAILED);

				return _erThreadReturn;
			}
		}

		#ifdef WIN32
			if (WinThread:: getSleep (0, 100) != errNoError)
		#else
			if (PosixThread:: getSleep (0, 100) != errNoError)
		#endif
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);

			return _erThreadReturn;
		}

		if (getIsShutdown (&bIsShutdown) != errNoError)
		{
			_erThreadReturn = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_GETISSHUTDOWN_FAILED);

			return _erThreadReturn;
		}
	}


	return _erThreadReturn;
}


Error SocketsPool:: cancel (void)

{

	time_t							tUTCNow;
	#ifdef WIN32
		WinThread:: PThreadStatus_t	stRTPThreadState;
	#else
		PosixThread:: PThreadStatus_t	stRTPThreadState;
	#endif


	if (setIsShutdown (true) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_SETISSHUTDOWN_FAILED);

		return err;
	}

	if (getThreadState (&stRTPThreadState) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);

		return err;
	}

	tUTCNow					= time (NULL);

	while (stRTPThreadState == THREADLIB_STARTED ||
		stRTPThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		if (time (NULL) - tUTCNow >= 5)
			break;

		#ifdef WIN32
			if (WinThread:: getSleep (1, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (1, 0) != errNoError)
		#endif
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);

			return err;
		}

		if (getThreadState (&stRTPThreadState) != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETTHREADSTATE_FAILED);

			return err;
		}
	}

	if (stRTPThreadState == THREADLIB_STARTED ||
		stRTPThreadState == THREADLIB_STARTED_AND_JOINED)
	{
		#ifdef WIN32
			// no cancel for windows thread
		#else
			if (PosixThread:: cancel () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_CANCEL_FAILED);

				return err;
			}
		#endif
	}


	return errNoError;
}


Error SocketsPool:: getIsShutdown (Boolean_p pbIsShutdown)

{

	if (_mtShutdown. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	*pbIsShutdown				= _bIsShutdown;

	if (_mtShutdown. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}

	return errNoError;
}


Error SocketsPool:: setIsShutdown (Boolean_t bIsShutdown)

{

	if (_mtShutdown. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	_bIsShutdown			= bIsShutdown;

	if (_mtShutdown. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}

	return errNoError;
}


#ifdef HAVE_POLL
	Error SocketsPool:: checkSocketsStatus (
		unsigned long ulSecondsToWait,
		unsigned long ulAdditionalMilliSecsToWait)

	{

		int						iMilliSeconds;
		SocketInfo_p			psiSocketInfo;
		SocketsInfoHashMap_t:: iterator			it;
		unsigned short			usLocalSocketCheckType;
		// int						iFileDescriptor;
		int						iPollReturn;
		unsigned long			ulSocketsPoolSize;
		unsigned long			ulSocketIndex;
		#ifdef WIN32
			__int64				ullStartUTCInMilliSecs;
			__int64				ullNowUTCInMilliSecs;
		#else
			unsigned long long	ullStartUTCInMilliSecs;
			unsigned long long	ullNowUTCInMilliSecs;
		#endif
		Boolean_t				bSystemCallFinished;


		#ifdef _REENTRANT
			if (_mtSocketsPool. lock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_LOCK_FAILED);

				return err;
			}
		#endif

		if (_ssSocketsPoolStatus != SOCKETSPOOL_INITIALIZED)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACTIVATION_WRONG);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			return err;
		}

		ulSocketsPoolSize			=
			_psiSocketsInfoSetByFileDescriptor -> size ();

		if (ulSocketsPoolSize == 0)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_POOLEMPTY);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			/* why it is necessary to wait if no sockets are present
				in the queue?
				Maybe the calling thread can do other processing or
				sleep himself if it doesn't have nothing to do
			if (PosixThread:: getSleep (ulSecondsToWait,
				ulAdditionalMicrosecondsToWait) != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_GETSLEEP_FAILED);
			}
			*/

			return err;
		}

		iMilliSeconds			= (ulSecondsToWait * 1000) +
			ulAdditionalMilliSecsToWait;

		if (DateTime:: nowUTCInMilliSecs (
			&ullStartUTCInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			return err;
		}

		bSystemCallFinished				= false;

		while (!bSystemCallFinished)
		{
// char aaa [128];
// sprintf (aaa, "poll: size: %lu, iMilliSeconds: %ld", ulSocketsPoolSize, (long) iMilliSeconds);
// FileIO::appendBuffer("/tmp/debug.txt", aaa, true);
			iPollReturn			= ::poll (((struct pollfd *) _ppfDescriptors),
				ulSocketsPoolSize, iMilliSeconds);

			if (iPollReturn == -1 && errno == EINTR)
			{
				if (DateTime:: nowUTCInMilliSecs (
					&ullNowUTCInMilliSecs,
					(long *) NULL) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

					#ifdef _REENTRANT
						if (_mtSocketsPool. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
						}
					#endif

					return err;
				}

				if (ullNowUTCInMilliSecs - ullStartUTCInMilliSecs
					>= iMilliSeconds)
					bSystemCallFinished			= true;
				else
					iMilliSeconds				= iMilliSeconds -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs);
			}
			else
				bSystemCallFinished				= true;
		}

		if (iPollReturn == 0)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			return err;
		}
		else if (iPollReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (iErrno != EINTR)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_POLL_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				#ifdef _REENTRANT
					if (_mtSocketsPool. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif

				return err;
			}
			else
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED);

				#ifdef _REENTRANT
					if (_mtSocketsPool. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif

				return err;
			}
		}
		else
		{
			// here I have to call the update method. In order to allow the
			// derived class to deleted the socket from the pool
			// (deleteSocket (pSocket)) it is necessary to copy all the sockets
			// to be update in a vector. In this way, also if the socket
			// is deleted from the pool, the deletion does not disturb the
			// iterator on the hashmap.
			SocketToBeUpdated_t							suSocketToBeUpdated;

			std:: vector<SocketToBeUpdated_t>	vSocketsToBeUpdated;


			vSocketsToBeUpdated. clear ();

			// fill is the vector with the sockets to be updated
			for (it = _psiSocketsInfoSetByFileDescriptor -> begin (),
				ulSocketIndex = 0;
				it != _psiSocketsInfoSetByFileDescriptor -> end ();
				++it, ulSocketIndex++)
			{
				// iFileDescriptor			= it -> first;

				psiSocketInfo			= it -> second;

				usLocalSocketCheckType	= 0x00;

				if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_READ)
				{
					if (((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) ->
						revents & POLLIN)
						usLocalSocketCheckType		|= SOCKETSTATUS_READ;
				}

				if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_WRITE)
				{
					if (((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) ->
						revents & POLLOUT)
						usLocalSocketCheckType		|= SOCKETSTATUS_WRITE;
				}

				if (psiSocketInfo -> _usSocketCheckType &
					SOCKETSTATUS_EXCEPTION)
				{
					if (((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) ->						revents & POLLPRI)
						usLocalSocketCheckType		|= SOCKETSTATUS_EXCEPTION;
				}

				((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) ->
					revents			= 0;

				if (usLocalSocketCheckType)
				{
					if (_bAllowDeletionSocketFromUpdateMethod)
					{
						suSocketToBeUpdated. _usLocalSocketCheckType		=
							usLocalSocketCheckType;
						suSocketToBeUpdated. _psiSocketInfo					=
							psiSocketInfo;

						vSocketsToBeUpdated. insert (
								vSocketsToBeUpdated. end (),
								suSocketToBeUpdated);
					}
					else
					{
						if (updateSocketStatus (
							psiSocketInfo -> _psSocket,
							psiSocketInfo -> _lSocketType,
							psiSocketInfo -> _pvData,
							usLocalSocketCheckType) != errNoError)
						{
							// Error err = SocketErrors (__FILE__, __LINE__,
							// 	SCK_SOCKETSPOOL_UPDATESOCKETSTATUS_FAILED);

							// #ifdef _REENTRANT
							// 	if (_mtSocketsPool. unLock () != errNoError)
							// 	{
							// 		Error err = PThreadErrors (
							// 			__FILE__, __LINE__,
							// 			THREADLIB_PMUTEX_UNLOCK_FAILED);
							// 	}
							// #endif

							// return err;
						}
					}
				}
			}

			if (_bAllowDeletionSocketFromUpdateMethod)
			{
				std:: vector<SocketToBeUpdated_t>:: const_iterator	it;


				for (it = vSocketsToBeUpdated. begin ();
					it != vSocketsToBeUpdated. end ();
					++it)
				{
					suSocketToBeUpdated			= *it;

					if (updateSocketStatus (
						suSocketToBeUpdated. _psiSocketInfo -> _psSocket,
						suSocketToBeUpdated. _psiSocketInfo -> _lSocketType,
						suSocketToBeUpdated. _psiSocketInfo -> _pvData,
						suSocketToBeUpdated. _usLocalSocketCheckType) !=
						errNoError)
					{
						/*
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETSPOOL_UPDATESOCKETSTATUS_FAILED);

						#ifdef _REENTRANT
							if (_mtSocketsPool. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}
						#endif

						return err;
						*/
					}
				}

				vSocketsToBeUpdated. clear ();
			}
		}
		/* OLD IMPLEMENTATION
		{
			std:: vector<Socket_p>		vSocketsToBeDeleted;
			Boolean_t					bSocketToBeRemoved;


			for (it = _psiSocketsInfoSetByFileDescriptor -> begin (),
				ulSocketIndex = 0;
				it != _psiSocketsInfoSetByFileDescriptor -> end ();
				++it, ulSocketIndex++)
			{
				// iFileDescriptor			= it -> first;

				psiSocketInfo			= it -> second;

				usLocalSocketCheckType	= 0x00;

				if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_READ)
				{
					if (((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) ->
						revents & POLLIN)
						usLocalSocketCheckType		|= SOCKETSTATUS_READ;
				}

				if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_WRITE)
				{
					if (((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) ->
						revents & POLLOUT)
						usLocalSocketCheckType		|= SOCKETSTATUS_WRITE;
				}

				if (psiSocketInfo -> _usSocketCheckType &
					SOCKETSTATUS_EXCEPTION)
				{
					if (((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) ->						revents & POLLPRI)
						usLocalSocketCheckType		|= SOCKETSTATUS_EXCEPTION;
				}

				((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) ->
					revents			= 0;

				if (usLocalSocketCheckType)
				{
					bSocketToBeRemoved			= false;

					if (updateSocketStatus (
						psiSocketInfo -> _psSocket,
						psiSocketInfo -> _lSocketType,
						psiSocketInfo -> _pvData, usLocalSocketCheckType,
						&bSocketToBeRemoved) != errNoError)
					{
						// Error err = SocketErrors (__FILE__, __LINE__,
						// 	SCK_SOCKETSPOOL_UPDATESOCKETSTATUS_FAILED);

						// #ifdef _REENTRANT
						// 	if (_mtSocketsPool. unLock () != errNoError)
						// 	{
						// 		Error err = PThreadErrors (__FILE__, __LINE__,
						// 			THREADLIB_PMUTEX_UNLOCK_FAILED);
						// 	}
						// #endif

						// return err;
					}

					if (bSocketToBeRemoved)
					{
						vSocketsToBeDeleted. insert (
							vSocketsToBeDeleted. end (),
							psiSocketInfo -> _psSocket);
					}
				}
			}

			{
				std:: vector<Socket_p>:: const_iterator		it;
				Socket_p									psSocket;
				Error_t										errDel;


				for (it = vSocketsToBeDeleted. begin ();
					it != vSocketsToBeDeleted. end ();
					++it)
				{
					psSocket			= *it;

					if ((errDel = deleteSocket (psSocket, (void **) NULL)) !=
						errNoError)
					{
						// Error err = SocketErrors (__FILE__, __LINE__,
						// 	SCK_SOCKETSPOOL_DELETESOCKET_FAILED);

						vSocketsToBeDeleted. clear ();

						#ifdef _REENTRANT
							if (_mtSocketsPool. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}
						#endif

						return errDel;
					}
				}
				vSocketsToBeDeleted. clear ();
			}
		}
		*/

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);

				return err;
			}
		#endif


		return errNoError;
	}
#else
	Error SocketsPool:: checkSocketsStatus (
		unsigned long ulSecondsToWait,
		unsigned long ulAdditionalMilliSecsToWait)

	{

		struct timeval			tvTimeval;
		fd_set					fdReadFd;
		fd_set					fdWriteFd;
		fd_set					fdExceptionFd;
		Boolean_t				bReadCheck;
		Boolean_t				bWriteCheck;
		Boolean_t				bExceptionCheck;
		int						iMaxFileDescriptor;
		SocketInfo_p			psiSocketInfo;
		SocketsInfoHashMap_t:: iterator			it;
		unsigned short			usLocalSocketCheckType;
		int						iFileDescriptor;
		int						iSelectReturn;
		unsigned long			ulSocketsPoolSize;
		/*
		#ifdef WIN32
			__int64				ullMilliSecondsToWait;
			__int64				ullStartUTCInMilliSecs;
			__int64				ullNowUTCInMilliSecs;
		#else
		*/
			unsigned long long	ullMilliSecondsToWait;
			unsigned long long	ullStartUTCInMilliSecs;
			unsigned long long	ullNowUTCInMilliSecs;
		// #endif
		Boolean_t				bSystemCallFinished;


		#ifdef _REENTRANT
			if (_mtSocketsPool. lock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_LOCK_FAILED);

				return err;
			}
		#endif

		if (_ssSocketsPoolStatus != SOCKETSPOOL_INITIALIZED)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_ACTIVATION_WRONG);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			return err;
		}

		ulSocketsPoolSize			=
			_psiSocketsInfoSetByFileDescriptor -> size ();

		if (ulSocketsPoolSize == 0)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_POOLEMPTY);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			/* why it is necessary to wait if no sockets are present
				in the queue?
				Maybe the calling thread can do other processing or
				sleep himself if it doesn't have nothing to do
			if (PosixThread:: getSleep (ulSecondsToWait,
				ulAdditionalMicrosecondsToWait) != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PTHREAD_GETSLEEP_FAILED);
			}
			*/

			return err;
		}

		tvTimeval. tv_sec  = ulSecondsToWait;
		tvTimeval. tv_usec = ulAdditionalMilliSecsToWait * 1000;
		ullMilliSecondsToWait		=
			(ulSecondsToWait * 1000) + ulAdditionalMilliSecsToWait;

		iMaxFileDescriptor			= -1;

		FD_ZERO (&fdReadFd);

		FD_ZERO (&fdWriteFd);

		FD_ZERO (&fdExceptionFd);

		bReadCheck				= false;
		bWriteCheck				= false;
		bExceptionCheck			= false;

		for (it = _psiSocketsInfoSetByFileDescriptor -> begin ();
			it != _psiSocketsInfoSetByFileDescriptor -> end (); ++it)
		{
			iFileDescriptor			= it -> first;
			psiSocketInfo			= it -> second;

			if (iFileDescriptor > iMaxFileDescriptor)
				iMaxFileDescriptor		= iFileDescriptor;

			if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_READ)
			{
				bReadCheck				= true;
				FD_SET (iFileDescriptor, &fdReadFd);
			}

			if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_WRITE)
			{
				bWriteCheck				= true;
				FD_SET (iFileDescriptor, &fdWriteFd);
			}

			if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_EXCEPTION)
			{
				bExceptionCheck				= true;
				FD_SET (iFileDescriptor, &fdExceptionFd);
			}
		}

		#ifdef WIN32
		#else
			if (iMaxFileDescriptor >= FD_SETSIZE)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETIMPL_FD_SETSIZETOBEINCREMENTED,
					2, (long) iMaxFileDescriptor, (long) FD_SETSIZE);

				#ifdef _REENTRANT
					if (_mtSocketsPool. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif

				return err;
			}
		#endif

		if (DateTime:: nowUTCInMilliSecs (
			&ullStartUTCInMilliSecs,
			(long *) NULL) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			return err;
		}

		bSystemCallFinished				= false;

		while (!bSystemCallFinished)
		{
			iSelectReturn			= ::select (iMaxFileDescriptor + 1,
				bReadCheck ? &fdReadFd : (fd_set *) NULL,
				bWriteCheck ? &fdWriteFd : (fd_set *) NULL,
				bExceptionCheck ? &fdExceptionFd : (fd_set *) NULL,
				&tvTimeval);

			if (iSelectReturn == -1 && errno == EINTR)
			{
				if (DateTime:: nowUTCInMilliSecs (
					&ullNowUTCInMilliSecs,
					(long *) NULL) != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

					#ifdef _REENTRANT
						if (_mtSocketsPool. unLock () != errNoError)
						{
							Error err = PThreadErrors (__FILE__, __LINE__,
								THREADLIB_PMUTEX_UNLOCK_FAILED);
						}
					#endif

					return err;
				}

				if (ullNowUTCInMilliSecs - ullStartUTCInMilliSecs
					>= ullMilliSecondsToWait)
					bSystemCallFinished			= true;
				else
				{
					tvTimeval. tv_sec  = (ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs)) / 1000;
					tvTimeval. tv_usec = ((ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs)) -
						tvTimeval. tv_sec * 1000) * 1000;
					ullMilliSecondsToWait		= (ullMilliSecondsToWait -
						(ullNowUTCInMilliSecs - ullStartUTCInMilliSecs));
				}
			}
			else
				bSystemCallFinished				= true;
		}

		if (iSelectReturn == 0)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			return err;
		}
		else if (iSelectReturn == -1)
		{
			int				iErrno;


			#ifdef WIN32
				iErrno			= WSAGetLastError ();
			#else
				iErrno			= errno;
			#endif

			if (iErrno != EINTR)
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SELECT_FAILED, 1, iErrno);
				err. setUserData (&iErrno, sizeof (int));

				#ifdef _REENTRANT
					if (_mtSocketsPool. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif

				return err;
			}
			else
			{
				Error err = SocketErrors (__FILE__, __LINE__,
					SCK_SOCKETSPOOL_SOCKETSTATUSNOTCHANGED);

				#ifdef _REENTRANT
					if (_mtSocketsPool. unLock () != errNoError)
					{
						Error err = PThreadErrors (__FILE__, __LINE__,
							THREADLIB_PMUTEX_UNLOCK_FAILED);
					}
				#endif

				return err;
			}
		}
		else
		{
			// here I have to call the update method. In order to allow the
			// derived class to deleted the socket from the pool
			// (deleteSocket (pSocket)) it is necessary to copy all the sockets
			// to be update in a vector. In this way, also if the socket
			// is deleted from the pool, the deletion does not disturb the
			// iterator on the hashmap.
			SocketToBeUpdated_t							suSocketToBeUpdated;

			std:: vector<SocketToBeUpdated_t>		vSocketsToBeUpdated;


			vSocketsToBeUpdated. clear ();

			// fill is the vector with the sockets to be updated
			for (it = _psiSocketsInfoSetByFileDescriptor -> begin ();
				it != _psiSocketsInfoSetByFileDescriptor -> end (); ++it)
			{
				iFileDescriptor			= it -> first;

				psiSocketInfo			= it -> second;

				usLocalSocketCheckType	= 0x00;

				if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_READ)
				{
					if (FD_ISSET (iFileDescriptor, &fdReadFd))
						usLocalSocketCheckType		|= SOCKETSTATUS_READ;
				}

				if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_WRITE)
				{
					if (FD_ISSET (iFileDescriptor, &fdWriteFd))
						usLocalSocketCheckType		|= SOCKETSTATUS_WRITE;
				}

				if (psiSocketInfo -> _usSocketCheckType &
					SOCKETSTATUS_EXCEPTION)
				{
					if (FD_ISSET (iFileDescriptor, &fdExceptionFd))
						usLocalSocketCheckType		|= SOCKETSTATUS_EXCEPTION;
				}

				if (usLocalSocketCheckType)
				{
					if (_bAllowDeletionSocketFromUpdateMethod)
					{
						suSocketToBeUpdated. _usLocalSocketCheckType		=
							usLocalSocketCheckType;
						suSocketToBeUpdated. _psiSocketInfo					=
							psiSocketInfo;

						vSocketsToBeUpdated. insert (
							vSocketsToBeUpdated. end (),
							suSocketToBeUpdated);
					}
					else
					{
						if (updateSocketStatus (
							psiSocketInfo -> _psSocket,
							psiSocketInfo -> _lSocketType,
							psiSocketInfo -> _pvData,
							usLocalSocketCheckType) != errNoError)
						{
							// Error err = SocketErrors (__FILE__, __LINE__,
							// 	SCK_SOCKETSPOOL_UPDATESOCKETSTATUS_FAILED);

							// #ifdef _REENTRANT
							// 	if (_mtSocketsPool. unLock () != errNoError)
							// 	{
							// 		Error err = PThreadErrors (
							//			__FILE__, __LINE__,
							// 			THREADLIB_PMUTEX_UNLOCK_FAILED);
							// 	}
							// #endif

							// return err;
						}
					}
				}
			}

			if (_bAllowDeletionSocketFromUpdateMethod)
			{
				std:: vector<SocketToBeUpdated_t>:: const_iterator	it;
				Error_t										errDel;


				for (it = vSocketsToBeUpdated. begin ();
					it != vSocketsToBeUpdated. end ();
					++it)
				{
					suSocketToBeUpdated			= *it;

					if (updateSocketStatus (
						suSocketToBeUpdated. _psiSocketInfo -> _psSocket,
						suSocketToBeUpdated. _psiSocketInfo -> _lSocketType,
						suSocketToBeUpdated. _psiSocketInfo -> _pvData,
						suSocketToBeUpdated. _usLocalSocketCheckType) !=
						errNoError)
					{
						/*
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETSPOOL_UPDATESOCKETSTATUS_FAILED);

						#ifdef _REENTRANT
							if (_mtSocketsPool. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}
						#endif

						return err;
						*/
					}
				}

				vSocketsToBeUpdated. clear ();
			}
		}
		/* OLD IMPLEMENTATION
		{
			std:: vector<Socket_p>		vSocketsToBeDeleted;
			Boolean_t					bSocketToBeRemoved;


			for (it = _psiSocketsInfoSetByFileDescriptor -> begin ();
				it != _psiSocketsInfoSetByFileDescriptor -> end (); ++it)
			{
				iFileDescriptor			= it -> first;

				psiSocketInfo			= it -> second;

				usLocalSocketCheckType	= 0x00;

				if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_READ)
				{
					if (FD_ISSET (iFileDescriptor, &fdReadFd))
						usLocalSocketCheckType		|= SOCKETSTATUS_READ;
				}

				if (psiSocketInfo -> _usSocketCheckType & SOCKETSTATUS_WRITE)
				{
					if (FD_ISSET (iFileDescriptor, &fdWriteFd))
						usLocalSocketCheckType		|= SOCKETSTATUS_WRITE;
				}

				if (psiSocketInfo -> _usSocketCheckType &
					SOCKETSTATUS_EXCEPTION)
				{
					if (FD_ISSET (iFileDescriptor, &fdExceptionFd))
						usLocalSocketCheckType		|= SOCKETSTATUS_EXCEPTION;
				}

				if (usLocalSocketCheckType)
				{
					bSocketToBeRemoved			= false;

					if (updateSocketStatus (psiSocketInfo -> _psSocket,
						psiSocketInfo -> _lSocketType,
						psiSocketInfo -> _pvData, usLocalSocketCheckType,
						&bSocketToBeRemoved) != errNoError)
					{
						Error err = SocketErrors (__FILE__, __LINE__,
							SCK_SOCKETSPOOL_UPDATESOCKETSTATUS_FAILED);

						// #ifdef _REENTRANT
						// 	if (_mtSocketsPool. unLock () != errNoError)
						// 	{
						// 		Error err = PThreadErrors (__FILE__, __LINE__,
						// 			THREADLIB_PMUTEX_UNLOCK_FAILED);
						// 	}
						// #endif

						// return err;
					}

					if (bSocketToBeRemoved)
					{
						vSocketsToBeDeleted. insert (
							vSocketsToBeDeleted. end (),
							psiSocketInfo -> _psSocket);
					}
				}
			}

			{
				std:: vector<Socket_p>:: const_iterator		it;
				Socket_p									psSocket;
				Error_t										errDel;


				for (it = vSocketsToBeDeleted. begin ();
					it != vSocketsToBeDeleted. end ();
					++it)
				{
					psSocket			= *it;

					if ((errDel = deleteSocket (psSocket, (void **) NULL)) !=
						errNoError)
					{
						// Error err = SocketErrors (__FILE__, __LINE__,
						// 	SCK_SOCKETSPOOL_DELETESOCKET_FAILED);

						vSocketsToBeDeleted. clear ();

						#ifdef _REENTRANT
							if (_mtSocketsPool. unLock () != errNoError)
							{
								Error err = PThreadErrors (__FILE__, __LINE__,
									THREADLIB_PMUTEX_UNLOCK_FAILED);
							}
						#endif

						return errDel;
					}
				}
				vSocketsToBeDeleted. clear ();
			}
		}
		*/

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);

				return err;
			}
		#endif


		return errNoError;
	}
#endif


Error SocketsPool:: addSocket (
	unsigned short usSocketCheckType,
	long lSocketType,
	Socket_p pSocket, void *pvSocketData)

{

	SocketImpl_p						pSocketImpl;
	SocketsInfoHashMap_t:: iterator		it;
	SocketInfo_p						psiSocketInfo;
	int									iDidInsert;
	unsigned long						ulSocketsPoolSize;


	if ((!(usSocketCheckType & SOCKETSTATUS_READ ||
		usSocketCheckType & SOCKETSTATUS_WRITE ||
		usSocketCheckType & SOCKETSTATUS_EXCEPTION)) ||
		pSocket == (Socket_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_mtSocketsPool. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_ssSocketsPoolStatus != SOCKETSPOOL_INITIALIZED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	if (pSocket -> getSocketImpl (&pSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	it		= _psiSocketsInfoSetByFileDescriptor -> find (pSocketImpl -> _iFd);

	if (it != _psiSocketsInfoSetByFileDescriptor -> end ()) 
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_SOCKETALREADYADDED,
			1, (long) (pSocketImpl -> _iFd));

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	ulSocketsPoolSize		= _psiSocketsInfoSetByFileDescriptor -> size ();

	if (ulSocketsPoolSize >= _ulMaxSocketsNumber)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_REACHEDMAXSOCKETSNUMBER,
			1, _ulMaxSocketsNumber);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	if (_vFreeSocketsInfo. begin () == _vFreeSocketsInfo. end ())
	{
		// we cannot have ulSocketsPoolSize < _ulMaxSocketsNumber
		//	and the vector empty
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_FREESOCKETSVECTORNOTCONSISTENT);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	psiSocketInfo			= *(_vFreeSocketsInfo. begin ());

	#ifdef HAVE_POLL
		(((struct pollfd *) _ppfDescriptors) [ulSocketsPoolSize]).
			fd				= pSocketImpl -> _iFd;

		{
			(((struct pollfd *) _ppfDescriptors) [ulSocketsPoolSize]).
				events			= 0;

			if (usSocketCheckType & SOCKETSTATUS_READ)
				(((struct pollfd *) _ppfDescriptors) [ulSocketsPoolSize]).
					events		|= POLLIN;

			if (usSocketCheckType & SOCKETSTATUS_WRITE)
				(((struct pollfd *) _ppfDescriptors) [ulSocketsPoolSize]).
					events		|= POLLOUT;

			if (usSocketCheckType & SOCKETSTATUS_EXCEPTION)
				(((struct pollfd *) _ppfDescriptors) [ulSocketsPoolSize]).
					events		|= POLLPRI;
		}

		(((struct pollfd *) _ppfDescriptors) [ulSocketsPoolSize]).
			revents			= 0;

		psiSocketInfo -> _ppfDescriptor				=
			&(((struct pollfd *) _ppfDescriptors) [ulSocketsPoolSize]);
	#endif

	psiSocketInfo -> _psSocket				= pSocket;
	psiSocketInfo -> _pvData				= pvSocketData;
	psiSocketInfo -> _usSocketCheckType		= usSocketCheckType;
	psiSocketInfo -> _lSocketType			= lSocketType;

	_psiSocketsInfoSetByFileDescriptor -> InsertWithoutDuplication (
		pSocketImpl -> _iFd, psiSocketInfo, &iDidInsert);
	if (!iDidInsert)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_KEYDUPLICATED,
			1, pSocketImpl -> _iFd);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}
	/*
	char aaa [1024];
	sprintf (aaa, "Added: %ld\n", (long) (pSocketImpl -> _iFd));
	FileIO:: appendBuffer ("/tmp/sock.txt", aaa, false);
	*/

	if (pSocketImpl -> _lLocalPort != -1)
	{
		// pSocketImpl -> _lLocalPort is -1 when we have a client socket opened
		// with the accept method (see SocketImpl:: acceptConnection)
		_psiSocketsInfoSetBySocketPort -> InsertWithoutDuplication (
			pSocketImpl -> _lLocalPort, psiSocketInfo, &iDidInsert);
		if (!iDidInsert)
		{
			Error err = SocketErrors (__FILE__, __LINE__,
				SCK_SOCKETSPOOL_KEYDUPLICATED,
				1, pSocketImpl -> _lLocalPort);

			_psiSocketsInfoSetByFileDescriptor -> Delete (pSocketImpl -> _iFd);

			#ifdef _REENTRANT
				if (_mtSocketsPool. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}
			#endif

			return err;
		}
	}

	_vFreeSocketsInfo. erase (_vFreeSocketsInfo. begin ());

	#ifdef _REENTRANT
		if (_mtSocketsPool. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error SocketsPool:: deleteSocket (
	Socket_p pSocket, void **pvSocketData)

{

	SocketImpl_p						pSocketImpl;
	SocketsInfoHashMap_t:: iterator		it;
	SocketInfo_p						psiSocketInfo;


	if (pSocket == (Socket_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_mtSocketsPool. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_ssSocketsPoolStatus != SOCKETSPOOL_INITIALIZED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	if (pSocket -> getSocketImpl (&pSocketImpl) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKET_GETSOCKETIMPL_FAILED);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	it		= _psiSocketsInfoSetByFileDescriptor -> find (pSocketImpl -> _iFd);

	/*
	char aaa [1024];
	sprintf (aaa, "ToDel: %ld\n", (long) (pSocketImpl -> _iFd));
	FileIO:: appendBuffer ("/tmp/sock.txt", aaa, false);
	*/

	if (it == _psiSocketsInfoSetByFileDescriptor -> end ()) 
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_SOCKETNOTFOUND,
			1, (long) (pSocketImpl -> _iFd));

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	psiSocketInfo			= it -> second;

	if (pvSocketData != (void **) NULL)
		*pvSocketData			= psiSocketInfo -> _pvData;

	#ifdef HAVE_POLL
		unsigned long						ulLastDescriptorIndex;

		ulLastDescriptorIndex		=
			_psiSocketsInfoSetByFileDescriptor -> size () - 1;

		if (((struct pollfd *) (psiSocketInfo -> _ppfDescriptor)) !=
			&(((struct pollfd *) _ppfDescriptors) [ulLastDescriptorIndex]))
		{
			*((struct pollfd *) (psiSocketInfo -> _ppfDescriptor))		=
				(((struct pollfd *) _ppfDescriptors) [ulLastDescriptorIndex]);
		}
	#endif

	_psiSocketsInfoSetByFileDescriptor -> Delete (pSocketImpl -> _iFd);
	if (pSocketImpl -> _lLocalPort != -1)
	{
		_psiSocketsInfoSetBySocketPort -> Delete (pSocketImpl -> _lLocalPort);
	}

	_vFreeSocketsInfo. insert (_vFreeSocketsInfo. end (),
		psiSocketInfo);

	#ifdef _REENTRANT
		if (_mtSocketsPool. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error SocketsPool:: findServerSocket (
	unsigned long ulServerPort,
	Socket_p *psSocket, void **pvSocketData)

{

	SocketInfo_p			psiSocketInfo;
	SocketsInfoHashMap_t:: iterator			itSocket;
	// int					iFileDescriptor;


	if (psSocket == (Socket_p *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	#ifdef _REENTRANT
		if (_mtSocketsPool. lock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_LOCK_FAILED);

			return err;
		}
	#endif

	if (_ssSocketsPoolStatus != SOCKETSPOOL_INITIALIZED)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	itSocket		= _psiSocketsInfoSetBySocketPort -> find (
		ulServerPort);

	if (itSocket == _psiSocketsInfoSetBySocketPort -> end ()) 
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETSPOOL_SOCKETNOTFOUND,
			1, (long) ulServerPort);

		#ifdef _REENTRANT
			if (_mtSocketsPool. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}
		#endif

		return err;
	}

	psiSocketInfo			= itSocket -> second;

	if (pvSocketData != (void **) NULL)
		*pvSocketData			= psiSocketInfo -> _pvData;

	*psSocket		= psiSocketInfo -> _psSocket;

	#ifdef _REENTRANT
		if (_mtSocketsPool. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}
	#endif


	return errNoError;
}

