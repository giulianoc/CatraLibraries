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


#include "PThreadErrors.h"


ErrMsgBase:: ErrMsgsInfo PThreadErrorsStr = {


	// PThread
	{ THREADLIB_CREATE_FAILED,
		"The thread create function failed (errno: %d)" },
	{ THREADLIB_ATTR_CREATE_FAILED,
		"The attribute create function failed (errno: %d)" },
	{ THREADLIB_ATTR_DELETE_FAILED,
		"The attribute delete function failed (errno: %d)" },
	{ THREADLIB_DELAY_NP_FAILED,
		"The sleep function failed (errno: %d)" },
	{ THREADLIB_CANCEL_FAILED,
		"The thread cancel function failed (errno: %d)" },
	{ THREADLIB_SETCANCEL_FAILED,
		"The thread set cancel function failed (errno: %d)" },
	{ THREADLIB_SETCANCELTYPE_FAILED,
		"The thread set cancel type function failed (errno: %d)" },
	{ THREADLIB_JOIN_FAILED,
		"The thread join function failed (errno: %d)" },
	{ THREADLIB_YIELD_FAILED,
		"The thread yield function failed (errno: %d)" },
	{ THREADLIB_DETACH_FAILED,
		"The thread detach function failed (errno: %d)" },
	{ THREADLIB_ATTR_SETPRIO_FAILED,
		"The thread attribute set priority function failed (errno: %d)" },
	{ THREADLIB_ATTR_SETDETACHSTATE_FAILED,
		"The thread attribute set detach state function failed (errno: %d)" },
	{ THREADLIB_ATTR_GETPRIO_FAILED,
		"The thread attribute get priority function failed (errno: %d)" },
	{ THREADLIB_SETPRIO_FAILED,
		"The thread set priority function failed (errno: %d)" },
	{ THREADLIB_GETPRIO_FAILED,
		"The thread get priority function failed (errno: %d)" },
	{ THREADLIB_ATTR_SETINHERITSCHED_FAILED,
	"The thread attribute set inherit scheduling function failed (errno: %d)" },
	{ THREADLIB_ATTR_GETINHERITSCHED_FAILED,
	"The thread attribute get inherit scheduling function failed (errno: %d)" },
	{ THREADLIB_ATTR_SETSCHED_FAILED,
		"The thread attribute set scheduling function failed (errno: %d)" },
	{ THREADLIB_ATTR_GETSCHED_FAILED,
		"The thread attribute get scheduling function failed (errno: %d)" },
	{ THREADLIB_ATTR_SETSTACKSIZE_FAILED,
		"The thread attribute set stack size function failed (errno: %d)" },
	{ THREADLIB_ATTR_GETSTACKSIZE_FAILED,
		"The thread attribute get stack size function failed (errno: %d)" },
	{ THREADLIB_PTHREAD_SETPTHREADNAME_FAILED,
		"The setPThreadName method of PThread class failed" },
	{ THREADLIB_PTHREAD_GETPTHREADNAME_FAILED,
		"The getPThreadName method of PThread class failed" },
	{ THREADLIB_PTHREAD_GETSLEEP_FAILED,
		"The getSleep method of PThread class failed" },
	{ THREADLIB_PTHREAD_CANCEL_FAILED,
		"The cancel method of PThread class failed" },
	{ THREADLIB_PTHREAD_EXIT_FAILED,
		"The exit method of PThread class failed" },
	{ THREADLIB_PTHREAD_START_FAILED,
		"The start method of PThread class failed" },
	{ THREADLIB_PTHREAD_RUN_FAILED,
		"The run method of PThread class failed" },
	{ THREADLIB_PTHREAD_GETTHREADIDENTIFIER_FAILED,
		"The getThreadIdentifier method of PThread class failed" },
	{ THREADLIB_PTHREAD_GETCURRENTTHREADIDENTIFIER_FAILED,
		"The getCurrentThreadIdentifier method of PThread class failed" },
	{ THREADLIB_PTHREAD_GETCURRENTTHREAD_FAILED,
		"The getCurrentThread method of PThread class failed" },
	{ THREADLIB_PTHREAD_JOIN_FAILED,
		"The join method of PThread class failed" },
	{ THREADLIB_PTHREAD_DETACH_FAILED,
		"The detach method of PThread class failed" },
	{ THREADLIB_PTHREAD_GETTHREADERROR_FAILED,
		"The getThreadError method of PThread class failed" },
	{ THREADLIB_PTHREAD_INIT_FAILED,
		"The init method of PThread class failed" },
	{ THREADLIB_PTHREAD_FINISH_FAILED,
		"The finish method of PThread class failed" },
	{ THREADLIB_PTHREAD_GETTHREADSTATE_FAILED,
		"The getThreadState method of PThread class failed" },
	{ THREADLIB_PTHREAD_SETSTACKSIZE_FAILED,
		"The setStackSize method of PThread class failed" },
	{ THREADLIB_PTHREAD_SETPRIORITY_FAILED,
		"The setPriority method of PThread class failed" },
	{ THREADLIB_PTHREAD_SETCURRENTPRIORITY_FAILED,
		"The setCurrentPriority method of PThread class failed" },

	// PMutex
	{ THREADLIB_MUTEXATTR_CREATE_FAILED,
		"The thread mutex attribute create function failed (errno: %d)" },
	{ THREADLIB_MUTEXATTR_SETTYPE_FAILED,
		"The thread mutex attribute set type function failed (errno: %d)" },
	{ THREADLIB_MUTEX_INIT_FAILED,
		"The thread mutex init function failed (errno: %d)" },
	{ THREADLIB_MUTEX_DESTROY_FAILED,
		"The thread mutex destroy function failed (errno: %d)" },
	{ THREADLIB_MUTEXATTR_DELETE_FAILED,
		"The thread mutex attribute delete function failed (errno: %d)" },
	{ THREADLIB_MUTEX_LOCK_FAILED,
		"The thread mutex lock function failed (errno: %d)" },
	{ THREADLIB_MUTEX_UNLOCK_FAILED,
		"The thread mutex unlock function failed (errno: %d)" },
	{ THREADLIB_MUTEX_TRYLOCK_FAILED,
		"The thread mutex try lock function failed (errno: %d)" },
	{ THREADLIB_PMUTEX_INIT_FAILED,
		"The init method of PMutex class failed" },
	{ THREADLIB_PMUTEX_FINISH_FAILED,
		"The finish method of PMutex class failed" },
	{ THREADLIB_PMUTEX_LOCK_FAILED,
		"The lock method of PMutex class failed" },
	{ THREADLIB_PMUTEX_UNLOCK_FAILED,
		"The unLock method of PMutex class failed" },

	// PCondition
	{ THREADLIB_CONDATTR_CREATE_FAILED,
		"The thread condition attribute create function failed (errno: %d)" },
	{ THREADLIB_CONDATTR_DELETE_FAILED,
		"The thread condition attribute delete function failed (errno: %d)" },
	{ THREADLIB_CONDATTR_SETATTRIBUTE_FAILED,
		"The set attribute method of the thread condition attribute function failed (errno: %d)" },
	{ THREADLIB_COND_INIT_FAILED,
		"The thread condition init function failed (errno: %d)" },
	{ THREADLIB_COND_DESTROY_FAILED,
		"The thread condition destroy function failed (errno: %d)" },
	{ THREADLIB_COND_WAIT_FAILED,
		"The thread condition wait function failed (errno: %d)" },
	{ THREADLIB_COND_SIGNAL_FAILED,
		"The thread condition signal function failed (errno: %d)" },
	{ THREADLIB_COND_BROADCAST_FAILED,
		"The thread condition broadcast function failed (errno: %d)" },
	{ THREADLIB_COND_TIMEDWAIT_FAILED,
		"The thread condition timed wait function failed (errno: %d)" },
	{ THREADLIB_PCONDITION_INIT_FAILED,
		"The init method of PCondition class failed" },
	{ THREADLIB_PCONDITION_FINISH_FAILED,
		"The finish method of PCondition class failed" },
	{ THREADLIB_PCONDITION_WAIT_FAILED,
		"The wait method of PCondition class failed" },
	{ THREADLIB_PCONDITION_TIMEDWAIT_FAILED,
		"The timedWait method of PCondition class failed" },
	{ THREADLIB_PCONDITION_SIGNAL_FAILED,
		"The signal method of PCondition class failed" },
	{ THREADLIB_PCONDITION_BROADCAST_FAILED,
		"The broadcast method of PCondition class failed" },

	// common
	{ THREADLIB_PARAMETER_WRONG,
		"The '%s' parameter is wrong" },
	{ THREADLIB_OPERATIONNOTALLOWED,
		"Operation not allowed (Current object status: %ld)" },
	{ THREADLIB_CLOSEHANDLE_FAILED,
		"CloseHandle failed" },
	{ THREADLIB_SETEVENT_FAILED,
		"SetEvent failed" },
	{ THREADLIB_CREATEEVENT_FAILED,
		"CreateEvent failed (Error: %ld)" },
	{ THREADLIB__BEGINTHREADEX_FAILED,
		"_beginthreadex failed (Error: %ld)" },
	{ THREADLIB_RESUMETHREAD_FAILED,
		"ResumeThread failed" },
	{ THREADLIB_WAITFORSINGLEOBJCT_FAILED,
		"WaitForSingleObject failed" },
	{ THREADLIB_NEW_FAILED,
		"The new function failed" }

	// Insert here other errors...

} ;

