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


#ifndef PCondition_h
	#define PCondition_h


	#ifdef __linux
		// see features.h
		#ifndef _XOPEN_SOURCE
			#define _XOPEN_SOURCE		500
		#endif
	#endif

	#include <pthread.h>
	#include <errno.h>
	#include "PThreadErrors.h"
	#include "PMutex.h"

    #ifdef __QTCOMPILER__
        #include <QtCore/qglobal.h>

        #if defined(PTHREAD_LIBRARY)
            #define PTHREADSHARED_EXPORT Q_DECL_EXPORT
        #else
            #define PTHREADSHARED_EXPORT Q_DECL_IMPORT
        #endif
    #else
        #define PTHREADSHARED_EXPORT
    #endif

	/**
		Condition class represents a condition variable.
		Once the variable has been
		created, threads can wait for the variable to "come true", or signal
		one or all the waiting threads to continue.
	*/
    typedef class PTHREADSHARED_EXPORT PCondition

	{

		private:
			pthread_cond_t				_pCondition;
			pthread_condattr_t			_pConditionAttribute;

		protected:
			PCondition (const PCondition &c);

//			PCondition &operator = (const PCondition &);

			/**
 				This method convert this Condition object into its 
 				pthread_cond_t structure
				Returns the pthread_cond_t pointer.
			*/
			operator pthread_cond_t *();

		public:
			#if defined(__hpux) && defined(_CMA__HP)
				typedef enum PConditionType {
					COND_DEFAULT
				} PConditionType_t, *PConditionType_p;
			#else	// POSIX
				#if defined(__sparc)				// SunOs
					typedef enum PConditionType {
						COND_PROCESS_SHARED			= PTHREAD_PROCESS_SHARED,
						COND_PROCESS_PRIVATE		= PTHREAD_PROCESS_PRIVATE
					} PConditionType_t, *PConditionType_p;
				#else	// POSIX.1-1996 standard (HPUX 11)
					typedef enum PConditionType {
						COND_DEFAULT
					} PConditionType_t, *PConditionType_p;
				#endif
			#endif


			/**
				Constructor.
			*/
			PCondition (void);

			/**
				Destructor.
			*/
			~PCondition (void);

			/**
				Initializes a condition variable.
				Occasionally, a thread running within a mutex needs to  wait
				for  an  event,  in  which case it blocks or sleeps.  When a
				thread is waiting for  another  thread  to  communicate  its
				disposition,  it  uses  a  condition variable in conjunction
				with a mutex.  Although a mutex is exclusive and the code it
				protects  is shareable (at certain moments), condition vari-
				ables enable the synchronization of  differing  events  that
				share  a mutex, but not necessarily data.  Several condition
				variables may be used by threads to signal each other when a
				task  is complete, which then allows the next waiting thread
				to take ownership of the mutex.
			
				In SUN machine
					Condition variables and mutexes should be global.  Condition
					variables  that  are  allocated  in writable memory can syn-
					chronize threads among processes if they are shared  by  the
					cooperating  processes (see mmap(2)) and are initialized for
					this purpose.
			
					The  pConditionType can assume the PTHREAD_PROCESS_PRIVATE
					value,  which  only  allows  the condition
					variable to be operated upon by threads created  within  the
					same  process  as  the thread that initialized the condition
					variable.  If threads from other processes try to operate on
					this condition variable, the behavior is undefined.
			
					The    pConditionType    may    be    set    to
					PTHREAD_PROCESS_SHARED  ,  which allows a condition variable
					to be operated upon by any thread with access to the  memory
					allocated  to  the condition variable, even if the condition
					variable is allocated in memory that is shared  by  multiple
					processes.
			
			*/
			Error init (PConditionType_t pConditionType);

			/**
				Deletes a condition variable.
			*/
			Error finish (void);

			/**
 				This method waits on the condition variable to be signalled
 				by another thread: it simply calls POSIX pthread_cond_wait
 				primitive.
				Causes a thread to wait for a condition variable to be
				signaled or broadcast.
				Call this routine after you have locked the mutex specified in
				pMutex.
				The results of this routine are unpredictable if this routine is
				called without first locking the mutex.
			
				This routine automatically releases the mutex and causes the
				calling thread to wait on the condition.
				If the wait is satisfied as a result
				of some thread calling signal () or
				broadcast (), the mutex is reacquired and the
				routine returns.
			
				This routine might (with low probability) return when the
				condition variable has not been signaled or broadcast.
				When a spurious wakeup occurs, the mutex is reacquired before
				the routine returns. (To handle this type of situation,
				enclose this routine in a loop that checks the predicate.)
			
				Parameters:
					pMutex - Mutex associated with the condition variable.
			*/
			Error wait (PMutex_p pMutex);

			/**
 				This method waits on the condition variable to be signalled
 				by another thread.
				Return:
					- errNoError if a signal is received
					- THREADLIB_COND_TIMEDWAIT_FAILED if an error is catched
						(include also the case of a timeout)

				Important, the man page says: "It is important to note that when
				pthread_cond_wait() and pthread_cond_timedwait() return without
				error, the associated predicate may still be false. Similarly,
				when pthread_cond_timedwait() returns with the timeout error,
				the associated predicate may be true due to an unavoidable
				race between the expiration of the timeout and the predi-
				cate state change.
				In general, whenever a condition wait returns, the thread has
				to re-evaluate the predicate associated with the condition wait
				to determine whether it can safely proceed, should wait again,
				or should declare a timeout. A return from the wait does not
				imply that the associated predicate is either true or false."

				It simply calls POSIX pthread_cond_timedwait primitive.
				Causes a thread to wait for a condition variable to be
				signaled or broadcast.
				See the wait comment.
				In The SUN machine a ETIME error indicates the time expired.
				In The HPUX machine a EAGAIN error indicates the time expired.

				Parameters:
					pMutex - Mutex associated with the condition variable
					ulSeconds - the length of time to wait in seconds
						if the condition has not been signaled or broadcast
					ulAdditionalMilliSeconds - additional milliseconds to wait
			*/
			Error timedWait (PMutex_p pMutex, unsigned long ulSeconds,
				unsigned long ulAdditionalMilliSeconds);

			/**
 				Wakes one thread that is waiting on a condition variable.
 				It simply calls the POSIX pthread_cond_signal primitive.
				Wakes one thread that is waiting on a condition variable.
				Calling this method implies that data guarded by
				the associated mutex has changed so that it is possible
				for a single waiting thread to proceed.
				Call this method when any thread waiting
				on the specified condition variable might find its
				predicate true,
				but only one thread needs to proceed.
				The scheduling policy determines which thread is awakened.
			*/
			Error signal (void);

			/**
 				Wakes all threads that are waiting on this condition variable.
 				It simply calls the POSIX pthread_cond_broadcast primitive.
				Wakes all threads that are waiting on a condition variable.
			*/
			Error broadcast (void);

	} PCondition_t, *PCondition_p;

#endif
