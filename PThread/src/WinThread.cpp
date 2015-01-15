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


#include "WinThread.h"
#include <process.h>


unsigned __stdcall runFunction (void *pvWinThread)

{

	WinThread_p					pWinThread			=
		(WinThread_p) pvWinThread;


	if (pWinThread == (WinThread_p) NULL)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pWinThread");

		return 1;	// ThreadStatus
	}

	pWinThread -> run ();

	if (pWinThread -> _stPThreadStatus ==
		WinThread:: THREADLIB_DETACHED)
	{
		if (CloseHandle (pWinThread -> _hThread) != S_OK)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_CLOSEHANDLE_FAILED);
		}

		pWinThread -> finish ();

		delete (pWinThread);

		_endthreadex (0); // ThreadStatus
	}
	else
	{
		// the state is THREADLIB_STARTED or THREADLIB_STARTED_AND_JOINED
		if (pWinThread -> _stPThreadStatus == WinThread:: THREADLIB_STARTED)
		{
			pWinThread -> _stPThreadStatus		=
				WinThread:: THREADLIB_INITIALIZEDAGAINAFTERRUNNING;

			if (CloseHandle (pWinThread -> _hEventForJoin) != S_OK)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CLOSEHANDLE_FAILED);
			}

			if (CloseHandle (pWinThread -> _hThread) != S_OK)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CLOSEHANDLE_FAILED);
			}

			_endthreadex (0); // ThreadStatus
		}
		else if (pWinThread -> _stPThreadStatus ==
			WinThread:: THREADLIB_STARTED_AND_JOINED)
		{
			if (SetEvent (pWinThread -> _hEventForJoin) == 0)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_SETEVENT_FAILED);
			}

			if (CloseHandle (pWinThread -> _hEventForJoin) != S_OK)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CLOSEHANDLE_FAILED);
			}

			if (CloseHandle (pWinThread -> _hThread) != S_OK)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CLOSEHANDLE_FAILED);
			}

			pWinThread -> _stPThreadStatus		=
				WinThread:: THREADLIB_INITIALIZEDAGAINAFTERRUNNING;

			// _endthreadex does not close the thread handle but close the Event handler
			_endthreadex (0); // ThreadStatus
		}
	}


	return 0;	// ThreadStatus
}


WinThread:: WinThread (void)

{

	strcpy (_pPThreadName, "");
	_erThreadReturn			= errNoError;
	_stPThreadStatus		= THREADLIB_BUILDED;
}


WinThread:: ~WinThread (void)

{

}


WinThread:: WinThread (const WinThread &t)

{

	strcpy (_pPThreadName, "");
	_erThreadReturn			= errNoError;
	_stPThreadStatus		= THREADLIB_BUILDED;
	*this = t;
}


/*
WinThread &WinThread:: operator = (const WinThread &)

{

	return *this;
}
*/


Error WinThread:: init (const char *pPThreadName)

{

	if (_stPThreadStatus != THREADLIB_BUILDED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	if (pPThreadName == (const char *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pPThreadName");

		return _erThreadReturn;
	}

	if (setPThreadName (pPThreadName) != errNoError)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_SETPTHREADNAME_FAILED);
		_erThreadReturn. setUserData (&errno, sizeof (int));
		
		return _erThreadReturn;
	}

	_stPThreadStatus		= THREADLIB_INITIALIZED;


	return errNoError;
}


Error WinThread:: finish (void)

{

	if (_stPThreadStatus != THREADLIB_INITIALIZED &&
		_stPThreadStatus != THREADLIB_INITIALIZEDAGAINAFTERRUNNING &&
		_stPThreadStatus != THREADLIB_DETACHED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	_stPThreadStatus		= THREADLIB_BUILDED;


	return errNoError;
}


Error WinThread:: getThreadState (PThreadStatus_p pstThreadState)

{

	if (pstThreadState == (PThreadStatus_p) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pstThreadState");

		return _erThreadReturn;
	}

	*pstThreadState		= _stPThreadStatus;


	return errNoError;
}


WinThread::PThreadStatus_t WinThread:: getThreadState (void)

{

	return _stPThreadStatus;
}


Error WinThread:: setPThreadName (const char *pPThreadName)

{

	if (pPThreadName == (const char *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pPThreadName");

		return _erThreadReturn;
	}

	if (strlen (pPThreadName) > MAX_PTHREADNAMELENGTH - 1)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_SETPTHREADNAME_FAILED);

		return _erThreadReturn;
	}

	strcpy (_pPThreadName, pPThreadName);

	
	return errNoError;
}


Error WinThread:: getPThreadName (char *pPThreadName)

{

	if (pPThreadName == (char *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "pPThreadName");

		return _erThreadReturn;
	}

	strcpy (pPThreadName, _pPThreadName);


	return errNoError;
}


Error WinThread:: start (Boolean_t bToBeDetached)

{

	unsigned				uThreadIdentifier;


	if (_stPThreadStatus != THREADLIB_INITIALIZED &&
		_stPThreadStatus != THREADLIB_INITIALIZEDAGAINAFTERRUNNING)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	_erThreadReturn		= errNoError;

	// E' necessario inizializzare lo stato a STARTED prima perche'
	// nel caso in cui il thread e' molto veloce e finisce prima
	// che la pthread_create ritorna, lo stato viene messo a STARTED
	// con il thread concluso (cioe' viene sovrascritta l'inizializzazione
	// a INITIALIZEDAGAINAFTERRUNNING che il thread finito
	// nella runFunction setta)
	if (bToBeDetached)
	{
		_stPThreadStatus		= THREADLIB_DETACHED;
	}
	else
	{
		_stPThreadStatus		= THREADLIB_STARTED;

		if ((_hEventForJoin = CreateEvent (
			0,
			(int) TRUE,	// manualReset
			(int) FALSE,	// setSignaled
			NULL)) == NULL)
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_CREATEEVENT_FAILED,
				1, (int) GetLastError ());
		
			_stPThreadStatus		= THREADLIB_INITIALIZED;

			return _erThreadReturn;
		}
	}

	if ((_hThread = (HANDLE) _beginthreadex (
		(void *) NULL,	// No security info
		(unsigned) 0,	// default stack size
		runFunction,
		this,
		(unsigned) CREATE_SUSPENDED,
		(unsigned *) &uThreadIdentifier)) <= 0)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB__BEGINTHREADEX_FAILED,
			1, (int) GetLastError ());

		_stPThreadStatus		= THREADLIB_INITIALIZED;

		if (bToBeDetached)
		{
			if (CloseHandle (_hEventForJoin) != S_OK)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CLOSEHANDLE_FAILED);
			}
		}

		return _erThreadReturn;
	}

	if (ResumeThread (_hThread) != 1)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_RESUMETHREAD_FAILED);

		_stPThreadStatus		= THREADLIB_INITIALIZED;

		if (CloseHandle (_hThread) != S_OK)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_CLOSEHANDLE_FAILED);
		}

		if (bToBeDetached)
		{
			if (CloseHandle (_hEventForJoin) != S_OK)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_CLOSEHANDLE_FAILED);
			}
		}

		return _erThreadReturn;
	}


	return errNoError;
}


Error WinThread:: getThreadIdentifier (unsigned long *pulThreadIdentifier) const

{

	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return err;
	}

	if (pulThreadIdentifier == (unsigned long *) NULL)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "plThreadIdentifier");

		return err;
	}

	*pulThreadIdentifier			= GetCurrentThreadId ();


	return errNoError;
}


WinThread:: operator long (void) const

{

	unsigned long			ulThreadIdentifier;


	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return -1;
	}

	ulThreadIdentifier			= GetCurrentThreadId ();


	return ulThreadIdentifier;
}


unsigned long WinThread:: getCurrentThreadIdentifier (void)

{

	unsigned long			ulThreadIdentifier;


	ulThreadIdentifier			= GetCurrentThreadId ();


	return ulThreadIdentifier;
}


Error WinThread:: getSleep (unsigned long ulSeconds,
	unsigned long ulAdditionalMicroSeconds)

{

	if (ulSeconds > 0)
	{
		Sleep (ulSeconds * 1000);
	}

	if (ulAdditionalMicroSeconds > 0)
	{
		Sleep (ulAdditionalMicroSeconds / 1000);
	}


	return errNoError;
}


Error WinThread:: getThreadError (Error *perrThreadError)

{

	if (perrThreadError == (Error *) NULL)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PARAMETER_WRONG, 1, "perrThreadError");

		return _erThreadReturn;
	}

	*perrThreadError	= _erThreadReturn;


	return errNoError;
}


Error WinThread:: join (Error_p perrJoinError)

{


	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		*perrJoinError = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	_stPThreadStatus		= THREADLIB_STARTED_AND_JOINED;

	*perrJoinError			= errNoError;

	if (WaitForSingleObject (_hEventForJoin, INFINITE) != WAIT_OBJECT_0)
	{
		*perrJoinError = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_WAITFORSINGLEOBJCT_FAILED);

		_stPThreadStatus		= THREADLIB_STARTED;

		return _erThreadReturn;
	}

	/*
		_hEventForJoin is already closed calling _endthreadex in runFunction
	if (CloseHandle (_hEventForJoin) != S_OK)
	{
		*perrJoinError = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_CLOSEHANDLE_FAILED);

		if (CloseHandle (_hThread) != S_OK)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_CLOSEHANDLE_FAILED);
		}

		return _erThreadReturn;
	}
	*/

	/* _hThread should not be closed by _endthreadex according the API documentation
		but if I close the handle I get always an error
	if (CloseHandle (_hThread) != S_OK)
	{
		*perrJoinError = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_CLOSEHANDLE_FAILED);

		return _erThreadReturn;
	}
	*/


	return _erThreadReturn;
}


Error WinThread:: detach (void)

{

	if (_stPThreadStatus != THREADLIB_STARTED)
	{
		_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_OPERATIONNOTALLOWED, 1, _stPThreadStatus);

		return _erThreadReturn;
	}

	_stPThreadStatus		= THREADLIB_DETACHED;


	return errNoError;
}
