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


#ifndef PMutex_h
	#define PMutex_h

	#ifdef __linux
		// see features.h
		#ifndef _XOPEN_SOURCE
			#define _XOPEN_SOURCE		500
		#endif
	#endif

	#include <pthread.h>
	#include <errno.h>

	#include "PThreadErrors.h"

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
		Rapresents the POSIX mutex.
	*/
    typedef class PTHREADSHARED_EXPORT PMutex

	{

		private:
			typedef enum PMutexStatus {
				THREADLIB_BUILDED,
				THREADLIB_INITIALIZED
			} PMutexStatus_t, *PMutexStatus_p;

		private:
			pthread_mutex_t			_pMutex;
			pthread_mutexattr_t 	_pMutexAttribute;
			PMutexStatus_t			_stPMutexStatus;


		protected:
			PMutex (const PMutex &m);

//			PMutex &operator = (const PMutex &);

			friend class PCondition;

			/**
 				This method convert this mutex object into its 
 				pthread_mutex_t structure
				Return the pthread_mutex_t pointer.
			*/
			operator pthread_mutex_t * ();


		public:
			#if defined(__hpux) && defined(_CMA__HP)
				typedef enum PMutexType {
					MUTEX_FAST			= MUTEX_FAST_NP,
					MUTEX_RECURSIVE		= MUTEX_RECURSIVE_NP,
					MUTEX_NONRECURSIVE	= MUTEX_NONRECURSIVE_NP
				} PMutexType_t, *PMutexType_p;
			#else	// POSIX
				/*
				#if defined(__sparc)		// SunOs (one of the first version)
					typedef enum PMutexType {
						MUTEX_PROCESS_SHARED		= PTHREAD_PROCESS_SHARED,
						MUTEX_PROCESS_PRIVATE		= PTHREAD_PROCESS_PRIVATE
					} PMutexType_t, *PMutexType_p;
				#else	// POSIX.1-1996 standard (HPUX 11)
				*/
					// POSIX.1-1996 standard (HPUX 11)
					typedef enum PMutexType {
						MUTEX_FAST			= PTHREAD_MUTEX_NORMAL,
						MUTEX_ERRORCHECK	= PTHREAD_MUTEX_ERRORCHECK,
						MUTEX_RECURSIVE		= PTHREAD_MUTEX_RECURSIVE,
						MUTEX_NONRECURSIVE	= PTHREAD_MUTEX_DEFAULT
					} PMutexType_t, *PMutexType_p;
				// #endif
			#endif

			/**
				Constructor.
			*/
			PMutex (void);

			/**
				Destructor.
			*/
			~PMutex (void);

			/**
				Initialize a mutex.
				Parameters:
					pMutexType - specifies the mutex type attribute.
			
				In the HPUX machine with CMA thread:
					This method creates a mutex and initializes it to
					the unlocked state.  If the thread that called this routine
					terminates, the created mutex is not automatically
					deallocated, because it is considered shared among
					multiple threads.
			
					Valid values are MUTEX_FAST (default),
					MUTEX_RECURSIVE, and MUTEX_NONRECURSIVE
			
					A fast mutex is locked and unlocked in the fastest
					manner possible.  A fast mutex can only be locked
					(obtained) once.  All subsequent calls
					to lock () cause the calling thread to block until the
					mutex is freed by the thread that owns it.
					If the thread that owns the mutex attempts to lock it
					again, the thread waits for itself to release the
					mutex (causing a deadlock).
			
					A recursive mutex can be locked more than once by the
					same thread without causing that thread to deadlock.
					In other words, a single thread can make consecutive
					calls to pthread_mutex_lock() without
					blocking.  The thread must then call
					unlock () the same number of times as it called
					lock () before another thread can lock the mutex.
			
					A nonrecursive mutex is locked only once by a thread,
					like a fast mutex.  If the thread tries to lock the
					mutex again without first unlocking it, the thread
					receives an error.  Thus, nonrecursive mutexes are more
					informative than fast mutexes because fast mutexes
					block in such a case, leaving it up to you to determine
					why the thread no longer executes.  Also, if someone
					other than the owner tries to unlock a nonrecursive
					mutex, an error is returned.
			
				In the HP-UX 11 machine (POSIX.1-1996 standard)
					Mutexes can be created with four different types.
					The type of a mutex is contained in the type attribute
					of the mutex attributes object.
      				Valid values for the type attribute are:
						PTHREAD_MUTEX_NORMAL
							This type of mutex does not provide deadlock
							detection. A thread attempting to relock
							this mutex without first unlocking it shall
							deadlock. An error is not returned
							to the caller. Attempting to unlock a mutex locked
							by a different thread results in undefined behavior.
							Attempting to unlock an unlocked mutex results
							in undefined behavior.
						PTHREAD_MUTEX_ERRORCHECK
							This type of mutex provides error checking.
							An owner field is maintained. Only the mutex lock
							owner shall successfully unlock this mutex.
							A thread attempting to relock this mutex shall
							return with an error.  A thread attempting to
							unlock a mutex locked by a different thread shall
							return with an error. A thread attempting to unlock
							an unlocked mutex shall return with an error.
							This type of mutex is useful for debugging.
						PTHREAD_MUTEX_RECURSIVE
							Deadlock cannot occur with this type of mutex.
							An owner field is maintained. A thread attempting
							to relock this mutex shall successfully lock
							the mutex.  Multiple locks of this mutex shall
							require the same number of unlocks to release
							the mutex before another thread can lock the mutex.
							A thread attempting to unlock a mutex locked by a
							different thread shall return with an error.
							A thread attempting to unlock an unlocked mutex
							shall return with an error.
						PTHREAD_MUTEX_DEFAULT
							Attempting to recursively lock a mutex of this type
							results in undefined behavior. Attempting to unlock
							a mutex locked by a different thread results in
							undefined behavior. Attempting to unlock an
							unlocked mutex results in undefined behavior.
							An implementation shall be allowed to map this
							mutex to one of the other mutex types.
			
				In the SUN (SPARC) machine:
					Mutual exclusion locks (mutexes)  prevent  multiple  threads
					from  simultaneously  executing  critical  sections  of code
					which access shared data (that is, mutexes are used to seri-
					alize  the  execution of threads).  All mutexes must be glo-
					bal.    A   successful   call   for   a   mutex   lock
					will  cause  another thread that is also trying to lock
					the same mutex  to  block until the owner thread unlocks it.
					Threads  within  the  same  process  or within other
					processes can share mutexes.
					Mutexes are either intra-process or inter-process, depending
					upon  the  argument  passed  implicitly or explicitly to the
					initialization of that mutex.
					For inter-process synchronization, a mutex needs to be allo-
					cated  in  memory shared between these processes.
			
					Valid values are MUTEX_PROCESS_PRIVATE (default),
					and MUTEX_PROCESS_SHARED
					If  the  process-shared attribute  is
					MUTEX_PROCESS_PRIVATE, only threads created
					within the same process as the thread that
					initialized  the mutex  can  access  the  mutex.
					If  threads  of  differing processes attempt to
					access  the  mutex,  the  behavior  is unpredictable.
			*/
			Error init (PMutexType_t pMutexType);

			/**
				Deletes a mutex.
			*/
			Error finish (void);

			/**
				Locks an unlocked mutex. If the mutex is locked
				when a thread calls this routine, the thread waits for the
				mutex to become available.
				The thread that has locked a mutex becomes its current owner and
				remains the owner until the same thread has unlocked it.
				In the SUN machine
					Mutual exclusion locks (mutexes)  prevent  multiple  threads
					from  simultaneously  executing  critical  sections  of code
					which access shared data (that is, mutexes are used to seri-
					alize  the  execution of threads).  All mutexes must be glo-
					bal.    A   successful   call   for   a   mutex   lock
					will  cause  another thread that is also trying to lock the
					same mutex  to  block until the owner thread unlocks it via
					unLock ().   Threads  within  the  same  process  or
					within other processes can share mutexes.
			
					Mutexes can synchronize threads within the same  process 
					or in  other  processes.  Mutexes  can  be  used to
					synchronize threads between processes if the mutexes  are
					allocated  in writable  memory  and shared among the
					cooperating processes (see mmap(2)), and have been
					initialized for this task.
			
				In the HP-UX 11 machine (POSIX.1-1996 standard)
					If the mutex type is PTHREAD_MUTEX_NORMAL, deadlock
					detection is not provided. Attempting to relock the mutex
					causes deadlock. If a thread attempts to unlock a mutex
					that it has not locked or a mutex which is unlocked,
					undefined behavior results.
					If the mutex type is PTHREAD_MUTEX_ERRORCHECK, the mutex
					maintains the concept of an owner. If a thread attempts
					to relock a mutex that it has already locked, an error
					shall be returned. If a thread attempts to unlock a mutex
					that it has not locked or a mutex that is unlocked,
					an error shall be returned.
					If the mutex type is PTHREAD_MUTEX_RECURSIVE,
					then the mutex maintains the concept of an owner and a lock
					count. When a thread successfully acquires a mutex for the
					first time, the count field shall be set to one.
					Every time a thread relocks this mutex, the count field
					shall be incremented by one. Each time the thread unlocks
					the mutex, the count field shall be decremented by one.
					When the count field reaches zero, the mutex shall become
					available for other threads to acquire. If a thread
					attempts to unlock a mutex that it has not locked, an error
					shall be returned.
					If the mutex type is PTHREAD_MUTEX_DEFAULT, attempting
					to recursively lock the mutex results in undefined behavior.
					Attempting to unlock the mutex if it was not locked by the
					calling thread results in undefined behavior.
					Attempting to unlocked the mutex if it is not locked results
			*/
			Error lock (void);

			/**
				Unlocks a mutex. See lock ().
			*/
			Error unLock (void);

			/**
				Locks a mutex. If the specified mutex is locked when a thread
				calls this routine, the calling thread does not wait for
				the mutex to become available.
			
				In the HP-UX 11 machine (POSIX.1-1996 standard)
					If the mutex type is PTHREAD_MUTEX_NORMAL, deadlock
					detection is not provided. Attempting to relock the mutex
					causes deadlock. If a thread attempts to unlock a mutex
					that it has not locked or a mutex which is unlocked,
					undefined behavior results.
					If the mutex type is PTHREAD_MUTEX_ERRORCHECK, the mutex
					maintains the concept of an owner. If a thread attempts
					to relock a mutex that it has already locked, an error
					shall be returned. If a thread attempts to unlock a mutex
					that it has not locked or a mutex that is unlocked,
					an error shall be returned.
					If the mutex type is PTHREAD_MUTEX_RECURSIVE, then the
					mutex maintains the concept of an owner and a lock count.
					When a thread successfully acquires a mutex for the first
					time, the count field shall be set to one. Every time a
					thread relocks this mutex, the count field shall be
					incremented by one. Each time the thread unlocks the mutex,
					the count field shall be decremented by one. When the count
					field reaches zero, the mutex shall become available
					for other threads to acquire. If a thread attempts
					to unlock a mutex that it has not locked, an error
					shall be returned.
			*/
			Error tryLock (Boolean_p pbIsLockable);

	} PMutex_t, *PMutex_p;

#endif
