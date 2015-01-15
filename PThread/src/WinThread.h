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


#ifndef WinThread_h
	#define WinThread_h


	#include "PThreadErrors.h"

	// to avoid that the WinSock is included with the consequence
	// of many types duplication compilation errors
	#define _WINSOCKAPI_

	#include <windows.h>


	#ifndef Boolean_t
		typedef long    Boolean_t;
	#endif

	#ifndef Boolean_p
		typedef long    *Boolean_p;
	#endif

	#ifndef false
		#define false   0L
	#endif

	#ifndef true
		#define true    1L
	#endif


	#define MAX_PTHREADNAMELENGTH		(128 + 1)


	/**
		This class will have the same interface as PosixThread.
		Therefore, to have more information, see the PosixThread documentation.
	*/
	typedef class WinThread

	{

		public:
			typedef enum PThreadStatus {
				THREADLIB_BUILDED,
				THREADLIB_INITIALIZED,
				THREADLIB_STARTED,	// and not joined
				THREADLIB_DETACHED,

				// When a POSIX thread is created, the operative system
				// frees the relative resources when the thread is joined
				// or is detached.
				// In case the thread will not be joined and will not be
				// detached, the resources are not freed and
				// it will cause a memory leak. The next state is added
				// to be sure that there will not be memory leak. In fact, when
				// the thread is finishing and is not joined or detached (state
				// THREADLIB_STARTED or THREADLIB_DETACHED),
				// it will be detached automatically.
				THREADLIB_STARTED_AND_JOINED,

				// When the thread is finished, the state returns
				// to INITIALIZED. The next state is added to discriminate
				// the INITIALIZED state after the 'init' method and
				// the INITIALIZED state when the thread is finished.
				THREADLIB_INITIALIZEDAGAINAFTERRUNNING
			} PThreadStatus_t, *PThreadStatus_p;

		private:
			char				_pPThreadName [MAX_PTHREADNAMELENGTH];
			PThreadStatus_t		_stPThreadStatus;

			friend unsigned __stdcall runFunction (void *pvWinThread);

		protected:
			HANDLE				_hEventForJoin;
			HANDLE				_hThread;

			// thread return
			Error			_erThreadReturn;

			WinThread (const WinThread &t);

//			WinThread &operator = (const WinThread &);

			virtual Error run (void) = 0;

			/**
				Sets this thread's class name.
				Parameters:
					pPThreadName - thread name.
			*/
			Error setPThreadName (const char *pPThreadName);

		public:
			/**
				Constructor.
			*/
			WinThread ();

			/**
				Destructor.
			*/
			virtual ~WinThread ();        

			Error init (const char *pPThreadName = "");

			virtual Error finish (void);

			Error getThreadState (PThreadStatus_p pstThreadState);
			PThreadStatus_t getThreadState (void);

			/**
				Causes this thread to begin execution.
				The result is that two threads are running concurrently:
				the current thread (which returns from the call
				to the start method) and
				the other thread (which executes its run method).

				bToBeDetached:
				If you have to detach a thread normally you will do:
					start ()
					detach ()
				If the thread is too fast potentially when you will call
				detach, the thread is already finished.
				In this case we could have memory leak.
				The parameter bToBeDetached make atomic the two calls avoiding
				any memory leak.
			*/
			virtual Error start (Boolean_t bToBeDetached = false);

			/**
				Return the thread identifier
			*/
			Error getThreadIdentifier (
				unsigned long *pulThreadIdentifier) const;

			/**
				Return the thread identifier
			*/
			operator long (void) const;

			static unsigned long getCurrentThreadIdentifier (void);

			/**
				Sets this thread's class name.
				Parameters:
					pPThreadName - thread name.
			*/
			Error getPThreadName (char *pPThreadName);

			/**
				Causes the currently executing thread to sleep
				(temporarily cease execution) for the specified number of
				lSeconds plus the specified number of
				microseconds (1/1000000 of secs).
					Parameters: 
						lSeconds - the length of time to sleep in seconds
						lAdditionalMicroSeconds - additional microseconds
							to sleep.
				On SUN machine only the lSeconds parameter is considered.
			*/
			static Error getSleep (unsigned long ulSeconds,
				unsigned long ulAdditionalMicroSeconds);

			/**
				On Windows it is not possible to cancel a thread (?).
				Therefore each thread have to implement this method in order to
				finish the thread.
			*/
			virtual Error cancel (void) = 0;

			/**
				Returns this thread's error.
				Parameters:
					perrThreadError - return value of the thread error.
			*/
			Error getThreadError (Error *perrThreadError);

			/**
				Causes the calling thread to wait for the termination
				of this thread.
				A call to this method returns after this thread has terminated.
				The returned error is the error of the customized class.
				The perrJoinError argument contains the eventual error of the
				join method (eventual failure of the pthread_join API)
			*/
			Error join (Error_p perrJoinError);

			/**
				Marks a thread object for deletion. If thread has not terminated
				when this method is called, this method does not cause it to
				terminate.
				Call this routine when a thread object is no longer referenced.
				Additionally, call this routine for every thread that is
				created to ensure that storage for thread objects does not
				accumulate.
				You cannot join with a thread after the thread has been detached
			
				After the calling of this method, is not necessary that the user
				calls the finish method because when the thread terminates,
				will be called the method finish and the thread object will be
				destroyed automatically.
			*/
			Error detach (void);

	} WinThread_t, *WinThread_p;

#endif

