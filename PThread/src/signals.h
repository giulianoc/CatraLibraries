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


#ifndef signals_h
#define signals_h

extern "C"
{
#include <dce/pthread.h>
}
#include <signal.h>
#include <defs.H>

#include <rw/tpslist.h>
#include <rw/tvhdict.h>
#include <rw/rwint.h>

#include <Trace.h>

class Signaled

{

	protected:
		Signaled(); // This is an abstract base class

		Signaled(const Signaled& t) { *this = t; };
		Signaled& operator= (const Signaled&) { SDGENASSERT(0); return *this; };

	public:
		enum SigStatus {
			HANDLED, 
			NOT_HANDLED, 
			TERMINATE
		};

		virtual ~Signaled();

		virtual SigStatus 	HandleSignal(const int signal) = 0;

		void AddToMask(const int sig);

		Boolean InMask(const int sig);

		int operator ==(const Signaled &m) const
		{
			return myMask. __sigbits == m.myMask.__sigbits;
		};

	private:
		sigset_t myMask;
};


class Terminated

{

	protected:
		Terminated();
		Terminated(const Terminated& s) { *this = s; };
		Terminated& operator= (const Terminated&)
		{
			SDGENASSERT(0);
			return *this;
		};

	public:
		virtual ~Terminated();

		virtual void 	HandleTermination() = 0;

		void SetPriority(int priority);

		int GetPriority() const;

		int operator==(const Terminated& p) const
		{
			return priority == p.priority;
		};

	private:
		int priority;
};


class Signaler

{

	friend class _EXPORT SigHandler;
	friend void SignalerTerminateHandler();

	public:
		Signaler();

		Signaler(const Signaler& s) { *this = s; };

		Signaler& operator= (const Signaler&)
		{
			SDGENASSERT(0);
			return *this;
		};

		virtual ~Signaler();

		void CatchSignal (int, Signaled &); // Spawn a signal handler

		static void CatchTermination (Terminated &, int priority = 0); // Catch process termination

		static void RemoveHandler (Signaled *handler);  // Unregister a handler

		static void RemoveTerminated (Terminated *term);// Unregister exit handler

		static void InitSignals (); // One-time-only initalization

	private:
		static void GotAnExit();			 // Called by exit handler

		static void Init();				 // Initialize me

		static void RealInit();			 // Really initialize me

		static RWTPtrSlist<Signaled> *handlerList; // List of signal handlers

		static RWTPtrSlist<Terminated> *terminatedList; // List of "terminated" objects

		static pthread_mutex_t handlerLatch; // Lock on list of handlers

		static Boolean caughtExit;	   // Is the exit handler "on"?

		static unsigned hashInt (const int &i)
		{
			return i;
		};

		static RWTValHashDictionary<int, RWInteger> *threadMap; // Map of handler threads

		static pthread_once_t didInit; // We were initialized
};

#endif

