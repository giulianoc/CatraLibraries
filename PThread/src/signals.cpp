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


extern "C"
{
#include <dce/pthread.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
}

#include <rw/tpslist.h>

#include <statics.H>
#include <transignals.H>
#include <defs.H>
#include <thread.H>
#include <dcecheckstatus.H>
#include "report.H"
#include "sddceTraceMessages.h"
#include "sddceError.h"

/*
 * static class members:
 */

pthread_mutex_t Signaler::handlerLatch;
RWTPtrSlist<Signaled>* Signaler::handlerList = (RWTPtrSlist<Signaled>*)0;
RWTPtrSlist<Terminated>* Signaler::terminatedList = (RWTPtrSlist<Terminated>*)0;
Boolean Signaler::caughtExit = false;
RWTValHashDictionary<int, RWInteger>* Signaler::threadMap = (RWTValHashDictionary<int, RWInteger>*)0;

pthread_once_t Signaler::didInit = pthread_once_init;

/*
 * SigHandler
 *    This is the class for the thread that actually listens for the
 * signals. It's Run() routine calls sigwait() to pick up signals from
 * the OS.
 */

class SigHandler: public ThreadObject

{

	public:
		SigHandler(int whatsignal);

		SigHandler (const SigHandler &s)
		{
			*this = s;
		};

		SigHandler& operator= (const SigHandler &)
		{
			SDGENASSERT(0);
			return *this;
		};

		virtual ~SigHandler ();

		virtual void* Run ();

	private:
		sigset_t sigMask;	// All the signals we'll wait for

};

/*
 * NAME
 *	SigHandler::SigHandler
 *
 * DESCRIPTION
 *	This is the SigHandler constructor. It prepares the internal
 *	signal mask for the signal waiting.
 *	(see sigemptyset, sigaddset)
 *
 * PARAMETERS
 *     	Input
 *		whatsignal: signal to wait for.
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */

SigHandler:: SigHandler (int whatsignal)

{

	int ret;

	ret = sigemptyset(&sigMask);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigemptyset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&sigMask, whatsignal);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}
}

/*
 * NAME
 *	SigHandler::~SigHandler
 *
 * DESCRIPTION
 *	SigHandler destructor
 *
 * PARAMETERS
 *     	Input
 *	        none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
SigHandler:: ~SigHandler()
{

}


/*
 * NAME
 *	SigHandler::Run
 *
 * DESCRIPTION
 *	This is where the signals get handled. As opposed to the "old style",
 * 	we usually spawn one thread for every signal we want to handle. This
 * 	way, users can add new handlers as life goes on, and we won't catch
 * 	signals users aren't interested in.
 *    	So here, we wait for the signals defined by the user.
 * 	When one goes off, we check to see if each handler wants it.
 * 	If so, we call that handler's HandleSignal routine. Each HandleSignal
 * 	routine can return how it handled the signal. If a HandleSignal function
 * 	returns TERMINATE, the program is terminated, but only after all the
 * 	HandleSignal functions have been called. A signal that is not
 * 	handled by any HandleSignal function is ignored.
 *    	After a signal is handled, sigwait() is called again to catch the
 * 	next one. This function loops forever.
 *	(see sigwait, sigismember)
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	(void*)0
 */
void *SigHandler:: Run ()

{

	int sig_number;
	Signaled* curHandler;
	RWTPtrSlistIterator<Signaled> iter(*Signaler::handlerList);
	Boolean hellFreezesOver = false;
	Boolean wasHandled;
	Boolean killMe = false;
	Signaled::SigStatus sigStatus;
	char* sigName;

	TRACE_ENTER(LDBG1, "SigHandler::Run");

	while (!hellFreezesOver)
	{
		sig_number = 0;
		sig_number = sigwait(&sigMask);
		if (sig_number != -1)
		{
			if ((sig_number < 0) || (sig_number >= NSIG))
			{
				SDDCEReport.ReportMsg(__FILE__, __LINE__, ErrorLevels::ErrorMsg,
					SDDCE_ERR_INVALID_SIGNAL, sig_number);
			}
			else
		    {
				sigName = "";
				TRACE_LOG(LDBG1, (char*)SDDCE_SIGNAL_RECEIVED, sig_number);

				wasHandled = false;
				killMe = false;

				if (pthread_mutex_lock(&Signaler::handlerLatch))
					CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_lock,
						__LINE__, __FILE__);

				iter.reset();
				while (++iter == TRUE)
				{
					curHandler = iter.key();
					if (pthread_mutex_unlock(&Signaler::handlerLatch))
						CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_unlock,
							__LINE__, __FILE__);	 

					if (sigismember(&sigMask, sig_number) == 1)
					{
						SDGENASSERT(curHandler != 0);
						if (curHandler->InMask(sig_number))
						{
							sigStatus = curHandler->HandleSignal(sig_number);

							if (sigStatus != Signaled::NOT_HANDLED)
								wasHandled = true;

							if (sigStatus == Signaled::TERMINATE)
								killMe = true;
						}
					}
					if (pthread_mutex_lock(&Signaler::handlerLatch))
						CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_lock,
							__LINE__, __FILE__); 
				}

				if (pthread_mutex_unlock(&Signaler::handlerLatch))
					CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_unlock,
						__LINE__, __FILE__);	 

				if (killMe)
					hellFreezesOver = 1;
			}
		}
		else
			CheckErrno(errno, SDDCE_SYSFUN_sigwait, __LINE__, __FILE__);
	}

	TRACE_EXIT(LDBG1, "SigHandler::Run");


	return (void*)0;
}


/*
 * NAME
 *	Signaled::Signaled
 *
 * DESCRIPTION
 *	This is the class that receives signals. It's an abstract base class,
 * 	so it's pretty simple. It also keeps track of the signals for which
 * 	it was registered to handle, and only delivers the "right ones" to 
 * 	HandleSignal.
 *	(see sigemptyset)
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
Signaled::Signaled()

{
	SDDCEFirstInit();
	int ret = sigemptyset(&myMask);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigemptyset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}
}

/*
 * NAME
 *	Signaled::AddToMask
 *
 * DESCRIPTION
 *	Adds the given signal to the current internal signal mask
 *	(see sigaddset)
 *
 * PARAMETERS
 *     	Input
 *		sig: signal number to add to the internal mask
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaled::AddToMask(const int sig)
{
	int ret = sigaddset(&myMask, sig);

	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}
}

/*
 * NAME
 *	SigHandler::SigHandler
 *
 * DESCRIPTION
 *	Check if the given signal number is present in the internal
 *	signal mask.
 *	(see sigismember)
 *
 * PARAMETERS
 *     	Input
 *		sig: signal to check.
 *	Output
 *		none
 *
 * RETURN VALUES
 *	Returns TRUE is signal is present, FALSE otherwise
 */
Boolean Signaled::InMask(const int sig)

{

	return (sigismember(&myMask, sig) == 1);

}

/*
 * NAME
 *	Signaled::~Signaled
 *
 * DESCRIPTION
 *	This is the SigHandler destructor.
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
Signaled:: ~Signaled()

{

	Signaler::RemoveHandler(this);

}

/*
 * NAME
 *	Terminated::Terminated
 *
 * DESCRIPTION
 *	This is similar to Signaled, but it's for classes that want to catch
 *	the process exit via an exit handler. Each Terminated object has a priority,
 * 	and upon exiting, higher-priority objects are destroyed first.
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
Terminated::Terminated()

{

	SDDCEFirstInit();
	priority = 0;

}

/*
 * NAME
 *	Terminated::~Terminated
 *
 * DESCRIPTION
 *	This is the Terminated destructor. It prepares the internal
 *	signal mask for the signal waiting.
 *	(see sigemptyset, sigaddset)
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
Terminated::~Terminated()

{

	Signaler::RemoveTerminated(this);

}

/*
 * NAME
 *	Terminated::SetPriority
 *
 * DESCRIPTION
 *	Sets the termination priority of this object in order to provide
 *	a specific termination policy.
 *
 * PARAMETERS
 *     	Input
 *		p: termination priority to set
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Terminated::SetPriority(int p)

{

	priority = p;

}

/*
 * NAME
 *	Terminated::GetPriority
 *
 * DESCRIPTION
 *	Gets the termination priority of this object
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	Returns the termination priority associated with this
 *	termination object
 */
int Terminated::GetPriority() const

{ 

	return priority; 

}

/*
 * NAME
 *	Signaler::Signaler
 *
 * DESCRIPTION
 *	This class is used to define signal handlers. Subclasses use it
 * 	to specify which objects are notified of which signals.
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
Signaler::Signaler()

{

	Init();

}

/*
 * NAME
 *	Signaler::RealInit
 *
 * DESCRIPTION
 *	Initialize all the static data. This should be called via pthread_once,
 * 	and in no other way.
 *	(see pthread_mutex_init)
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaler::RealInit()

{

	handlerList = new RWTPtrSlist<Signaled>();
	terminatedList = new RWTPtrSlist<Terminated>();
	if (pthread_mutex_init(&handlerLatch, pthread_mutexattr_default))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_init, __LINE__, __FILE__); 
	}

	threadMap = new RWTValHashDictionary<int, RWInteger>(Signaler::hashInt);
}

/*
 * Init
 *    Do one-time initialization. Just use pthread_once to ensure that
 * "RealInit" is called once and only once.
 */

/*
 * NAME
 *	Signaler::Init
 *
 * DESCRIPTION
 *	Do one-time initialization. Just use pthread_once to ensure that
 * 	"RealInit" is called once and only once.
 *	(see pthread_once)
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaler::Init()
{
	SDDCEFirstInit();
	if (pthread_once(&didInit, RealInit))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_once, __LINE__, __FILE__); 
	}
}

/*
 * NAME
 *	Signaler::~Signaler
 *
 * DESCRIPTION
 *	The handler thread keeps running, so don't destroy anything here.
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
Signaler::~Signaler()
{

}

/*
 * NAME
 *	Signaler::CatchSignal
 *
 * DESCRIPTION
 *	Put the right objects in the right places so signal handling
 * 	works. First, check if this Signaled object has been registered before.
 * 	If not, put it on the list of handlers. Also, always make sure its 
 * 	mask is updated so we can see which signals it cares about.
 *    	Also, check the map of handler threads. If there isn't already
 * 	one running for this signal, spawn one. This means many threads could
 * 	be running. The alternative is to catch all "likely" signals, which
 * 	is a bad idea because someday some other piece of software won't be able
 * 	to deal with that.
 *
 * PARAMETERS
 *     	Input
 *		sigNum:		signal number to handle
 *		handler:	handling object
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaler::CatchSignal (int sigNum, Signaled &handler)

{

	Signaled* oldHandler = &handler;		// For searching the list
	Signaled* newHandler = oldHandler;		// New element to add

	InitSignals();
	if (pthread_mutex_lock(&handlerLatch))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_lock, __LINE__, __FILE__); 
	}
	handler.AddToMask(sigNum);

	RWTPtrSlistIterator<Signaled> iter(*handlerList);

	Signaled* tmpHandler = iter.findNext(oldHandler);
	if ( !tmpHandler )
	{
		TRACE_LOG(LDBG1, "%s", SDDCE_PUT_SIGNALED_ON_LIST);
		SDGENASSERT(handlerList != 0);
		handlerList->insert(newHandler); // Put it on the list if it isn't there
	}
	if ((*threadMap)[sigNum] == 0)
	{ // Spawn a thread for this sig if there isn't one
		TRACE_LOG(LDBG1, (char*)SDDCE_ADD_SIG_HAND_THR, sigNum);
		ThreadObject* handler = new SigHandler(sigNum);     // Start handler
		AsyncThread* handleThread = new AsyncThread(*handler);
		delete handleThread;
	}
	else
	{
		TRACE_LOG(LDBG1, (char*)SDDCE_USE_SIG_HAND_THR, sigNum);
	}
	(*threadMap)[sigNum] = ((*threadMap)[sigNum]) + RWInteger(1);
	if (pthread_mutex_unlock(&handlerLatch))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_unlock,
			__LINE__, __FILE__);	 
	}
}

/*
 * NAME
 *	Signaler::RemoveHandler
 *
 * DESCRIPTION
 *	Unregistered a Signaled object by removing it from the list of
 * 	handlers. We might want to also consider cancelling the handler
 * 	thread for all its signals, but I'm not going to worry about that
 * 	right now.
 *
 * PARAMETERS
 *     	Input
 *		handler: handling object
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaler::RemoveHandler(Signaled* handler)
{
	SDGENASSERT(handler != 0);

	if (pthread_mutex_lock(&handlerLatch))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_lock, __LINE__, __FILE__); 
	}
	RWTPtrSlistIterator<Signaled> iter(*handlerList);
	if (iter.findNext(handler))
	{
		TRACE_LOG(LDBG1, "%s", SDDCE_REMOVE_SIGNALED);
		void* ret = iter.remove();
		SDGENASSERT(ret != 0);     
	}
	if (pthread_mutex_unlock(&handlerLatch))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_unlock,
			__LINE__, __FILE__);	 
	}
}

/*
 * NAME
 *	SignalerTerminateHandler
 *
 * DESCRIPTION
 *	This function isn't a member of any class, but is called by either
 * 	on_exit or atexit to handle catching exit().
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void SignalerTerminateHandler()

{

	Signaler::GotAnExit();

}

/*
 * NAME
 *	Signaler::GotAnExit
 *
 * DESCRIPTION
 *	We have been notified that the process is exiting, so we call
 * 	HandleTermination on every object in the list of Terminated objects.
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaler::GotAnExit()
{
	RWTPtrSlistIterator<Terminated> iter(*terminatedList);
	Terminated* curHandler;

	TRACE_LOG(LDBG1, "%s", SDDCE_EXIT_HAND_ACTIVATED);
	while (++iter)
	{
		curHandler = iter.key();
		SDGENASSERT(curHandler != 0);
		TRACE_LOG(LDBG1, (char*)SDDCE_TERMINATE_OBJECT,
			curHandler->GetPriority());
		curHandler->HandleTermination();
	}
}

/*
 * NAME
 *	Signaler::CatchTermination
 *
 * DESCRIPTION
 *	Similar to CatchSignal in that we put a pointer to the "Terminated"
 * 	object on a list. However, this list is handled in priority order,
 * 	and there are no threads to worry about. Instead, the exit handler
 * 	is fired off if necessary.
 *
 * PARAMETERS
 *     	Input
 *		n:		termination object
 *		priority:	termination priority
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaler::CatchTermination(Terminated& 	n, 
				int 		priority)
{
	Terminated* newHandler = &n;
	Terminated* tmpHandler;

	Init();
	if (pthread_mutex_lock(&handlerLatch))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_lock, __LINE__, __FILE__); 
	}
	SDGENASSERT(newHandler != 0);
	newHandler->SetPriority(priority);
	tmpHandler = newHandler;
  
	SDGENASSERT(terminatedList != 0);
	if (terminatedList->find(tmpHandler) == 0)
	{
		TRACE_LOG(LDBG1, (char*)SDDCE_ADD_TERMINATED, priority);
      
		int foundPos = 0;
		for (int i = 0; i < terminatedList->entries(); i++)
		{
			SDGENASSERT(terminatedList[i] != 0);
			if ( (((*terminatedList)[i])->GetPriority()) <= priority )
			{
				foundPos = 1;
				break;
			}
		}
		if (foundPos)
			terminatedList->insertAt(i, tmpHandler);
		else
			terminatedList->insert(tmpHandler);
 
	}
	if (!caughtExit)
	{
		TRACE_LOG(LDBG1, "%s", SDDCE_REGISTER_EXIT_HAND);
		int ret = atexit(SignalerTerminateHandler);
		SDGENASSERT(ret == 0);
		caughtExit = 1;
	}

	if (pthread_mutex_unlock(&handlerLatch))
		CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_unlock,
			__LINE__, __FILE__);	 
}

/*
 * NAME
 *	Signaler::RemoveTerminated
 *
 * DESCRIPTION
 *	Remove a Terminated object from the list.
 *
 * PARAMETERS
 *     	Input
 *		dead: terminated object to remove
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaler::RemoveTerminated(Terminated* dead)
{
	SDGENASSERT(dead != 0);

	if (pthread_mutex_lock(&handlerLatch))
    {
      CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_lock, __LINE__, __FILE__); 
	}
	RWTPtrSlistIterator<Terminated> iter(*terminatedList);
	if (iter.findNext(dead))
	{
		TRACE_LOG(LDBG1, "%s", SDDCE_REMOVE_TERMINATED);
		void* ret = iter.remove();
		SDGENASSERT(ret != 0);
	}
	if (pthread_mutex_unlock(&handlerLatch))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_mutex_unlock,
			__LINE__, __FILE__);	 
	}
}

static pthread_once_t didInitSignals = pthread_once_init;

/*
 * NAME
 *	SignalerCallInitSignals
 *
 * DESCRIPTION
 *	Set this process's signal mask so it knows we don't want to catch
 * 	certain signals. These are "normal" signals like SIGINT and SIGTERM that
 * 	processes don't usually catch. On some OS's, this won't matter because
 * 	we'll re-set the signal mask if we want to catch any of these signals. On
 * 	others, we must un mask these signals because the signal mask is a 
 * 	thread-specific resource. Only one thread in the program should catch
 * 	any particular signal. Otherwise, the "wrong" thread will get it, and
 * 	the signal will never be delivered.
 *    	This code doesn't do anything if POSIX_THREAD_PER_PROCESS_SIGNALS_1
 * 	is defined. If it's defined, the signal mask affects the whole program
 * 	and blocking these signals will cause non-default behavior (i.e. coredumping
 * 	for some, pausing on SIGTSTP, and so forth).
 *	(see sigemptyset, sigaddset, sigprocmask)
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
static void SignalerCallInitSignals()
{

#ifndef POSIX_THREAD_PER_PROCESS_SIGNALS_1
	sigset_t goAwayMask;

	TRACE_LOG(LDBG1, "%s", SDDCE_INIT_SIGMASK);

	int ret;

	ret = sigemptyset(&goAwayMask);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigemptyset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGALRM);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGCHLD);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGCONT);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGHUP);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGINT);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGQUIT);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGTERM);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGTSTP);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGTTIN);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGTTOU);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGUSR1);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigaddset(&goAwayMask, SIGWINCH);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigaddset, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

	ret = sigprocmask(SIG_BLOCK, &goAwayMask, (sigset_t*)0);
	if (ret == -1)
	{
		CheckErrno(errno, SDDCE_SYSFUN_sigprocmask, __LINE__, __FILE__);
		SDGENASSERT(0);
	}

#else
	TRACE_LOG(LDBG1, "%s", SDDCE_INIT_NO_NEED_SIGMASK);
#endif
}

/*
 * NAME
 *	Signaler::InitSignals
 *
 * DESCRIPTION
 *	Calls the SignalerCallInitSignals function
 *	(see pthread_once)
 *
 * PARAMETERS
 *     	Input
 *		none
 *	Output
 *		none
 *
 * RETURN VALUES
 *	none
 */
void Signaler::InitSignals()
{
	if (pthread_once(&didInitSignals, SignalerCallInitSignals))
	{
		CheckErrno(errno, SDDCE_DCEFUN_pthread_once, __LINE__, __FILE__); 
	}
}

