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


#ifndef PThreadErrors_h
	#define PThreadErrors_h

	#include "Error.h"
	#include <iostream>

	/**

		Click <a href="PThreadErrors.C#PThreadErrors" target=classContent>here</a> for the errors strings.

	*/
	enum PThreadErrorsCodes {

		// PThread
		THREADLIB_CREATE_FAILED,
		THREADLIB_ATTR_CREATE_FAILED,
		THREADLIB_ATTR_DELETE_FAILED,
		THREADLIB_DELAY_NP_FAILED,
		THREADLIB_CANCEL_FAILED,
		THREADLIB_SETCANCEL_FAILED,
		THREADLIB_SETCANCELTYPE_FAILED,
		THREADLIB_JOIN_FAILED,
		THREADLIB_YIELD_FAILED,
		THREADLIB_DETACH_FAILED,
		THREADLIB_ATTR_SETPRIO_FAILED,
		THREADLIB_ATTR_SETDETACHSTATE_FAILED,
		THREADLIB_ATTR_GETPRIO_FAILED,
		THREADLIB_SETPRIO_FAILED,
		THREADLIB_GETPRIO_FAILED,
		THREADLIB_ATTR_SETINHERITSCHED_FAILED,
		THREADLIB_ATTR_GETINHERITSCHED_FAILED,
		THREADLIB_ATTR_SETSCHED_FAILED,
		THREADLIB_ATTR_GETSCHED_FAILED,
		THREADLIB_ATTR_SETSTACKSIZE_FAILED,
		THREADLIB_ATTR_GETSTACKSIZE_FAILED,
		THREADLIB_PTHREAD_SETPTHREADNAME_FAILED,
		THREADLIB_PTHREAD_GETPTHREADNAME_FAILED,
		THREADLIB_PTHREAD_GETSLEEP_FAILED,
		THREADLIB_PTHREAD_CANCEL_FAILED,
		THREADLIB_PTHREAD_EXIT_FAILED,
		THREADLIB_PTHREAD_START_FAILED,
		THREADLIB_PTHREAD_RUN_FAILED,
		THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED,
		THREADLIB_PTHREAD_GETCURRENTTHREADIDENTIFIER_FAILED,
		THREADLIB_PTHREAD_GETCURRENTTHREAD_FAILED,
		THREADLIB_PTHREAD_JOIN_FAILED,
		THREADLIB_PTHREAD_DETACH_FAILED,
		THREADLIB_PTHREAD_GETTHREADERROR_FAILED,
		THREADLIB_PTHREAD_INIT_FAILED,
		THREADLIB_PTHREAD_FINISH_FAILED,
		THREADLIB_PTHREAD_GETTHREADSTATE_FAILED,
		THREADLIB_PTHREAD_SETSTACKSIZE_FAILED,
		THREADLIB_PTHREAD_SETPRIORITY_FAILED,
		THREADLIB_PTHREAD_SETCURRENTPRIORITY_FAILED,

		// PMutex
		THREADLIB_MUTEXATTR_CREATE_FAILED,
		THREADLIB_MUTEXATTR_SETTYPE_FAILED,
		THREADLIB_MUTEX_INIT_FAILED,
		THREADLIB_MUTEX_DESTROY_FAILED,
		THREADLIB_MUTEXATTR_DELETE_FAILED,
		THREADLIB_MUTEX_LOCK_FAILED,
		THREADLIB_MUTEX_UNLOCK_FAILED,
		THREADLIB_MUTEX_TRYLOCK_FAILED,
		THREADLIB_PMUTEX_INIT_FAILED,
		THREADLIB_PMUTEX_FINISH_FAILED,
		THREADLIB_PMUTEX_LOCK_FAILED,
		THREADLIB_PMUTEX_UNLOCK_FAILED,

		// PCondition
		THREADLIB_CONDATTR_CREATE_FAILED,
		THREADLIB_CONDATTR_DELETE_FAILED,
		THREADLIB_CONDATTR_SETATTRIBUTE_FAILED,
		THREADLIB_COND_INIT_FAILED,
		THREADLIB_COND_DESTROY_FAILED,
		THREADLIB_COND_WAIT_FAILED,
		THREADLIB_COND_SIGNAL_FAILED,
		THREADLIB_COND_BROADCAST_FAILED,
		THREADLIB_COND_TIMEDWAIT_FAILED,
		THREADLIB_PCONDITION_INIT_FAILED,
		THREADLIB_PCONDITION_FINISH_FAILED,
		THREADLIB_PCONDITION_WAIT_FAILED,
		THREADLIB_PCONDITION_TIMEDWAIT_FAILED,
		THREADLIB_PCONDITION_SIGNAL_FAILED,
		THREADLIB_PCONDITION_BROADCAST_FAILED,

		// common
		THREADLIB_PARAMETER_WRONG,
		THREADLIB_OPERATIONNOTALLOWED,
		THREADLIB_CLOSEHANDLE_FAILED,
		THREADLIB_SETEVENT_FAILED,
		THREADLIB_CREATEEVENT_FAILED,
		THREADLIB__BEGINTHREADEX_FAILED,
		THREADLIB_RESUMETHREAD_FAILED,
		THREADLIB_WAITFORSINGLEOBJCT_FAILED,
		THREADLIB_NEW_FAILED,

		// Insert here other errors...

		THREADLIB_MAXERRORS

	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long', possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomErrorClass (PThreadErrors, THREADLIB_MAXERRORS)

#endif

