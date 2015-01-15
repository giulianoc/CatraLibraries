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


#ifndef PosixThread_h
	#define PosixThread_h


	#ifdef __linux
		// see features.h
		#ifndef _XOPEN_SOURCE
			#define _XOPEN_SOURCE		500
		#endif
	#endif

	#if defined(__hpux) && defined(_CMA__HP)
		#include <pthread.h>	// may be #include <cma.h>
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		#include <pthread.h>
	#endif

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

		A PosixThread is a thread of execution in a program.
		To create a new thread of execution you must declare a class
		to be a subclass of PosixThread.
		This subclass should override the run method of class PosixThread.
		An instance of the subclass can then be allocated and started.
		For example, a thread that computes primes larger
		than a stated value could be written as follows: 

			class PrimeThread extends Thread {
				long minPrime;
				PrimeThread (long minPrime)
				{
					this. minPrime = minPrime;
				}
				public void run ()
				{
					compute primes larger than minPrime
					. . .
				}
			}
	
		The following code would then create a thread and start it running: 
	
			PrimeThread p = new PrimeThread (143);
			p. init ();
			p. start ();
			p. finish ();

		La libreria PosixThread semplifica la gestione di threads secondo
		lo standard POSIX fornendo una interfaccia object oriented semplice
		da utilizzare.

		Un oggetto di tipo PosixThread rappresenta un thread non necessariamente
		in esecuzione.

		Per creare ed eseguire un thread e' necessario:

		1. dichiarare una classe come sottoclasse di PosixThread
			(es. ServerCorbaThread)
		2. ridefinire nella sottoclasse il metodo run della classe PosixThread
		3. istanziare la sottoclasse
			(sctServerCorbaThread = new ServerCorbaThread)
		4. inizializzare la sottoclasse (sctServerCorbaThread. init ())
		5. far partire l'esecuzione del thread (sctServerCorbaThread. start ())

		Quando il thread termina e' necessario

		1. eseguire la finish del thread (sctServerCorbaThread. finish ())
		2. de-allocare il thread (delete sctServerCorbaThread)

		Le precedenti due operazione vengono eseguite automaticamente dalla
		libreria nel caso in cui il thread sia detached (vedi la descrizione
		del metodo PosixThread::detach all'interno del Reference Manual).

		La classe PosixThread mette a disposizione tutto cio' che e' possibile
		fare su di un thread (vedi Reference Manual).

		La libreria permette anche di gestire la mutua esclusione.
		Come il nome stesso indica tale meccanismo serve per fare in modo
		che solo un thread acceda in un certo istante ad una certa zona
		di dati o, equivalentemente, esegue un certo spezzone di codice.
		La classe che ci viene in aiuto per risolvere questa situazione e'
		PMutex. Essa possiede i metodi lock ed unLock esattamente per la
		gestione di 'regioni critiche'. Questi metodi devono essere chiamati
		risp.te all'inizio ed alla fine della regione critica e si comportano
		come un semaforo, garantendo che al piu' un solo thread si puo trovare
		in quella porzione di codice.

		Nel caso in cui si desideri che un thread voglia sospendere la propria
		esecuzione aspettando che una certa condizione sia vera, e' necessario
		utilizzare le condition variable realizzate nella libreria dalla classe
		PConditionVariable. Ad esempio, un thread che legge da un buffer puo'
		volere aspettare che ci siano dei dati nel buffer senza essere
		costretto a dei cicli di attesa attiva.

		Gli esempi che si trovano nella directory examples chiariranno
		l'uso di questa libreria.

	*/
    typedef class PTHREADSHARED_EXPORT PosixThread

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

			#if defined(__hpux) && defined(_CMA__HP)
				typedef enum PThreadInheritScheduling {
					THREADLIB_INHERITSCHED	= PTHREAD_INHERIT_SCHED,
					THREADLIB_DEFAULTSCHED	= PTHREAD_DEFAULT_SCHED
				} PThreadInheritScheduling_t, *PThreadInheritScheduling_p;
			#elif __QTCOMPILER__
			#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
				typedef enum PThreadInheritScheduling {
					THREADLIB_INHERITSCHED	= PTHREAD_INHERIT_SCHED,
					THREADLIB_DEFAULTSCHED	= PTHREAD_EXPLICIT_SCHED
				} PThreadInheritScheduling_t, *PThreadInheritScheduling_p;
			#endif

			#if defined(__hpux) && defined(_CMA__HP)
				typedef enum PThreadSchedulingPolicy {
					THREADLIB_SCHEDFIFO		= SCHED_FIFO,
					THREADLIB_SCHEDRR		= SCHED_RR,
					THREADLIB_SCHEDOTHER	= SCHED_OTHER,
					THREADLIB_SCHEDFGNP		= SCHED_FG_NP,
					THREADLIB_SCHEDBGNP		= SCHED_BG_NP
				} PThreadSchedulingPolicy_t, *PThreadSchedulingPolicy_p;
			#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
				typedef enum PThreadSchedulingPolicy {
					THREADLIB_SCHEDFIFO		= SCHED_FIFO,
					THREADLIB_SCHEDRR		= SCHED_RR,
					THREADLIB_SCHEDOTHER	= SCHED_OTHER
				} PThreadSchedulingPolicy_t, *PThreadSchedulingPolicy_p;
			#endif

			#if defined(__hpux) && defined(_CMA__HP)
				typedef enum PThreadPriority {
					THREADLIB_PRIOTHERMIN	= PRI_OTHER_MIN,
					THREADLIB_PRIOTHERMAX	= PRI_OTHER_MAX,
					THREADLIB_PRIFIFOMIN	= PRI_FIFO_MIN,
					THREADLIB_PRIFIFOMAX	= PRI_FIFO_MAX,
					THREADLIB_PRIRRMIN		= PRI_RR_MIN,
					THREADLIB_PRIRRMAX		= PRI_RR_MAX,
					THREADLIB_PRIFGMINNP	= PRI_FG_MIN_NP,
					THREADLIB_PRIFGMAXNP	= PRI_FG_MAX_NP,
					THREADLIB_PRIBGMINNP	= PRI_BG_MIN_NP,
					THREADLIB_PRIBGMAXNP	= PRI_BG_MAX_NP
				} PThreadPriority_t, *PThreadPriority_p;
			#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
				typedef enum PThreadPriority {
					THREADLIB_PRIORITY_MIN		= -1000,
					THREADLIB_PRIORITY_MEDIUM	= -2000,
					THREADLIB_PRIORITY_MAX		= -3000
				} PThreadPriority_t, *PThreadPriority_p;
			#endif

			#if defined(__hpux) && defined(_CMA__HP)
				typedef enum PThreadCancellationState {
					THREADLIB_CANCEL_OFF	= CANCEL_OFF,
					THREADLIB_CANCEL_ON		= CANCEL_ON
				} PThreadCancellationState_t, *PThreadCancellationState_p;
			#elif __QTCOMPILER__
				typedef enum PThreadCancellationState {
				} PThreadCancellationState_t, *PThreadCancellationState_p;
			#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
				typedef enum PThreadCancellationState {
					THREADLIB_CANCEL_OFF	= PTHREAD_CANCEL_DISABLE,
					THREADLIB_CANCEL_ON		= PTHREAD_CANCEL_ENABLE
				} PThreadCancellationState_t, *PThreadCancellationState_p;
			#endif

			#if defined(__hpux) && defined(_CMA__HP)
				typedef enum PThreadCancellationType {
					THREADLIB_CANCEL_ASYNCHRONOUS	= CANCEL_ON,
					THREADLIB_CANCEL_DEFERRED		= CANCEL_OFF
				} PThreadCancellationType_t, *PThreadCancellationType_p;
			#elif __QTCOMPILER__
			#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
				typedef enum PThreadCancellationType {
					THREADLIB_CANCEL_ASYNCHRONOUS	=
						PTHREAD_CANCEL_ASYNCHRONOUS,
					THREADLIB_CANCEL_DEFERRED		= PTHREAD_CANCEL_DEFERRED
				} PThreadCancellationType_t, *PThreadCancellationType_p;
			#endif

		private:
			char				_pPThreadName [MAX_PTHREADNAMELENGTH];
			PThreadStatus_t		_stPThreadStatus;

			friend void *runFunction (void *pPosixThread);

		protected:
			pthread_t			_pThread;
			pthread_attr_t		_pThreadAttribute;
			// thread return
			Error			_erThreadReturn;

			PosixThread (const PosixThread &t);

//			PosixThread &operator = (const PosixThread &);

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
			PosixThread ();

			/**
				Destructor. Call the finish() method in case
				it was not already called.
			*/
			virtual ~PosixThread ();        

			Error init (const char *pPThreadName = "");

			/*
			 * This method is called automatically when the object is deleted
			 */
			virtual Error finish (void);

			Error getThreadState (PThreadStatus_p pstThreadState);
			PThreadStatus_t getThreadState (void);

			/**
				Changes the scheduling priority attribute of thread creation.
				Parameters:
					lNewPriority - New value for the priority attribute.
						The priority attribute depends on scheduling policy.
			
				With HPUX machine with CMA
						valid values fall within one of the following ranges:
			
				THREADLIB_PRIOTHERMIN <= lNewPriority <= THREADLIB_PRIOTHERMAX
					(use with the THREADLIB_SCHEDOTHER policy)
				THREADLIB_PRIFIFOMIN <= lNewPriority <= THREADLIB_PRIFIFOMAX
					(use with the THREADLIB_SCHEDFIFO policy)
				THREADLIB_PRIRRMIN <= lNewPriority <= THREADLIB_PRIRRMAX
					(use with the THREADLIB_SCHEDRR policy)
				THREADLIB_PRIFGMINNP <= lNewPriority <= THREADLIB_PRIFGMAXNP
					(use with the THREAD_SCHEDFGNP policy)
				THREADLIB_PRIBGMINNP <= lNewPriority <= THREADLIB_PRIBGMAXNP
					(use with the THREADLIB_SCHEDBGNP policy)
			
				The  default  priority  is  the  midpoint  between
				THREADLIB_PRIOTHERMIN  and THREADLIB_PRIOTHERMAX.
			
				To  specify  a  minimum  or  maximum priority, use the
				appropriate symbol; for  example, THREADLIB_PRIFIFOMIN or
				THREADLIB_PRIFIFOMAX.
				To specify a value between the minimum and maximum, use an
				appropriate arithmetic expression.  For example,  to  specify
				a  priority  midway between the minimum and maximum for the
				Round Robin scheduling policy, specify  the  following  concept
				using  your  programming  language's syntax:
				pri_rr_mid = (THREADLIB_PRIRRMIN + THREADLIB_PRIRRMAX + 1) / 2
				If your expression results in a value outside the range of
				minimum  to maximum, an error results when you attempt to use
				it. By default, a created thread  inherits  the  priority  of
				the  thread calling
			
				With SUN or HP-UX 11 machine, there are functions as:
					int sched_get_priority_max(int policy);
					int sched_get_priority_min(int policy);
			*/
			Error setPriority (long lNewPriority);

			/**
				Obtains the scheduling priority attribute.
				Parameters:
					plPriority - returns the scheduling priority attribute value
			*/
			Error getPriority (long *plPriority);

			/**
				Changes the inherit scheduling attribute
				Parameters:
					lNewInheritScheduling - New value for the inherit scheduling
						attribute. Valid values are as follows:
			
						THREADLIB_INHERITSCHED: this is the default value.
							The created thread inherits the current priority
							and scheduling policy of the thread calling.
						THREADLIB_DEFAULTSCHED: the created thread starts
							execution with the priority and scheduling policy
							stored in the thread attributes object.
			*/
			#if __QTCOMPILER__
			#else
				Error setInheritScheduling (
					PThreadInheritScheduling_t isNewInheritScheduling);
			#endif

			/**
				Obtains the inherit scheduling attribute
				Parameters:
					pisInheritScheduling - returns the inherit scheduling
						attribute value
			*/
			#if __QTCOMPILER__
			#else
				Error getInheritScheduling (
					PThreadInheritScheduling_p pisInheritScheduling);
			#endif

			/**
				Changes the scheduling policy attribute of thread creation
				Parameters:
					lNewSchedulingPolicy - New value for the scheduling policy
						attribute. Valid values are as follows:
			
						THREADLIB_SCHEDFIFO: (First In, First Out)
							The highest-priority thread runs until it blocks.
							If there is more than one thread with the same
							priority, and that priority is the highest among
							other threads, the first thread to begin running
							continues until it blocks.
						THREADLIB_SCHEDRR: (Round Robin) The highest-priority
							thread runs until it blocks; however, threads of
							equal priority, if that priority is the highest
							among other threads, are timesliced.
							Timeslicing is a process in which threads alternate
							using available processors.
						THREADLIB_SCHEDOTHER: (Default) All threads are
							timesliced. SCHED_OTHER ensures that all threads,
							regardless of priority, receive some scheduling so
							that no thread is completely denied execution time.
							(However, THREADLIB_SCHEDOTHER threads can be denied
							execution time by THREADLIB_SCHEDFIFO or
							THREADLIB_SCHED_RR threads.)
						THREADLIB_SCHEDFGNP: (Foreground)
							Same as THREADLIB_SCHEDOTHER. Threads are timesliced
							and priorities can be modified dynamically by the
							scheduler to ensure fairness.
						THREADLIB_SCHEDBGNP: (Background)
							Ensures that all threads, regardless of priority,
							receive some scheduling.
							However, THREADLIB_SCHEDBGNP can be denied
							execution by THREADLIB_SCHEDFIFO or
							THREADLIB_SCHEDRR threads.
			*/
			Error setSchedulingPolicy (
				PThreadSchedulingPolicy_t spNewSchedulingPolicy);

			/**
				Obtains the value of the scheduling policy attribute
				Parameters:
					pspSchedulingPolicy - returns the value of the
						scheduling policy attribute
			*/
			Error getSchedulingPolicy (
				PThreadSchedulingPolicy_p pspSchedulingPolicy);

			/**
				Changes the stacksize attribute of thread creation
				Parameters:
					lNewStackSize - New value for the stacksize attribute.
						The stacksize parameter specifies the minimum size
						(in bytes) of the stack needed for a thread.
						The default value of the stacksize attribute is
						machine specific. Most compilers do not check for
						stack overflow.  Ensure that your thread stack is
				large enough for anything that you call from the thread.
			*/
			Error setStackSize (long lNewStackSize);

			/**
				Obtains the value of the stacksize attribute.
				The pthread_attr_getstacksize() routine obtains the minimum
				size (in bytes) of the stack for a thread created using the
				thread attributes object specified by the attr parameter.
				Parameters:
					plStackSize - returns the stacksize attribute value
			*/
			Error getStackSize (long *plStackSize);

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

			static Error getThreadIdentifier (const pthread_t *pThread,
				unsigned long *pulThreadIdentifier);

			static unsigned long getCurrentThreadIdentifier (void);

			/**
				This method allows a thread to obtain its own
				identifier.
				This value becomes meaningless when
				the thread object is deleted.
			*/
			static Error getCurrentThread (pthread_t &pThread);

			/**
				Changes the current priority of a thread.
				Parameters:
					lNewCurrentPriority - new thread priority
					(see setPriority).
			*/
			Error setCurrentPriority (long lNewCurrentPriority);

			/**
				Obtains the current priority of a thread.
				Parameters:
					plCurrentPriority - return value of the thread priority.
			*/
			Error getCurrentPriority (long *plCurrentPriority);

			/**
				Sets this thread's class name.
				Parameters:
					pPThreadName - thread name.
			*/
			Error getPThreadName (char *pPThreadName);

			/**
 				This method notifies the scheduler that the current
 				thread is willing to release its processor to other threads
				of the same priority. (A thread releases its processor
				to a thread of a higher priority without calling this routine.)
				Causes the currently executing thread object to temporarily
				pause and allow other threads to execute.
				sched_yield() forces the running process to  relinquish  the
				processor  until  the  process again becomes the head of its
				process list.
				Requeue current process in process list. The sched_yield()
				function forces the running process to relinquish
				the processor until it again becomes the head
				of its process list.  It takes no arguments.
			*/
			static Error yield (void);

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
				Terminates the calling thread.
			*/
			static Error exit (void);

			/**
				Allows a thread to request that it terminate execution.
				This method sends a cancel to this thread. A
				cancel is a mechanism by which a calling thread informs 
				this thread to terminate as quickly as possible. Issuing a
				cancel does not guarantee that the canceled thread receives
				or handles
				the cancel. The canceled thread can delay processing the
				cancel after
				receiving it. For instance, if a cancel arrives during an
				important operation, the canceled thread can continue
				if what it is doing cannot be interrupted at the point
				where the cancel is requested.
				Because of communications delays, the calling thread can only
				rely on the fact that a cancel eventually becomes
				pending in the designated thread (provided that the thread
				does not terminate beforehand).
				Furthermore, the calling thread has no guarantee that
				a pending cancel
				is to be delivered because delivery is controlled by the
				designated thread.
			*/
			virtual Error cancel (void);

			/**
				Requests delivery of a pending cancel to the current thread.
				The cancel is delivered only if a cancel is
				pending for the current thread and general cancel delivery is
				not currently disabled. (A thread disables delivery of
				cancels to itself by calling the setCancel () method.)
			*/
			#ifdef __QTCOMPILER__
				// not supported by Android
			#else
				static Error testCancel (void);
			#endif

			/**
				Enables or disables the current thread's general cancelability.
				When general cancelability is set to THREADLIB_CANCEL_OFF,
				a cancel cannot be delivered to the thread.
				When a thread is created, the default general cancelability
				state is THREADLIB_CANCEL_ON.
			*/
			#ifdef __QTCOMPILER__
				// not supported by Android
			#else
				static Error setCancel (
					PThreadCancellationState_t csCancelability);
			#endif

			/**
				The thread's cancellation type determines  when  a
				thread could get cancelled.
				When the cancellation state is disabled, a thread's  cancel-
				lation  type  is  meaningless.   The  following cancellation
				types behave as follows when enabled:
			
				THREADLIB_CANCEL_ASYNCHRONOUS
					Receipt  of  a  pthread_cancel()  call   will
					result in an immediate cancellation.
			
				THREADLIB_CANCEL_DEFERRED
					Cancellation will not occur until the  target
					thread  reaches  a  cancellation  point  (see
					below).  Receipt of a  pthread_cancel()  call
					will  result  in an immediate cancellation at
					this cancellation point.
				The cancellation type is set to THREADLIB_CANCEL_DEFERRED,  by
				default.
			*/
			#ifdef __QTCOMPILER__
				// not supported by Android
			#else
				Error setCancelType (PThreadCancellationType_t ctCancelType);
			#endif

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

	} PosixThread_t, *PosixThread_p;

#endif

