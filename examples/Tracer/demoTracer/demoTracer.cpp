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

#include "TracingThread.h"
#include "LoopThread.h"
#include "Tracer.h"
#include <iostream>
#ifdef WIN32
	#include <windows.h>
#endif

#define THREAD_NUMBER					10


Tracer_t			gtTracer;


int main (int argc, char **argv)

{

	#ifdef WIN32
		WinThread_p						ptPThread [THREAD_NUMBER];
	#else
		PosixThread_p					ptPThread [THREAD_NUMBER];
	#endif
	long								lTracingThreadIndex;
	long								lSecondsBetweenTwoCheckTraceToManage;
	Error_t								errJoin;


	lSecondsBetweenTwoCheckTraceToManage		= 7;

	if (gtTracer. init (
		"demoTracer",					// pName
		-1,								// lCacheSizeOfTraceFile K-byte
		"./tmp1",							// pTraceDirectory
		"@YYYY@_@DD@_@MM@_@HI24@_@MI@_@SS@_@MSS@_traceFile",					// pTraceFileName
		5000,							// lMaxTraceFileSize K-byte
		150000,							// lTraceFilePeriodInSecs
		true,							// bCompressedTraceFile
		true,							// _bClosedFileToBeCopied
		"./tmp2",						// _pClosedFilesRepository
		8,								// lTraceFilesNumberToMaintain
		true,							// bTraceOnFile
		false,							// bTraceOnTTY
		0,								// lTraceLevel
		lSecondsBetweenTwoCheckTraceToManage,
		3000,							// lMaxTracesNumber
		-1,								// lListenPort
		10000,							// lTracesNumberAllocatedOnOverflow
		1000) != errNoError)			// lSizeOfEachBlockToGzip K-byte
	{

		std:: cerr << "Tracer. init failed" << std:: endl;

		return 1;
	}

	if (gtTracer. start () != errNoError)
	{
		std:: cerr << "Tracer. start failed" << std:: endl;

		gtTracer. finish (true);

		return 1;
	}

	/*
	PosixThread:: getSleep (5, 0);
	gtTracer. cancel ();
	PosixThread:: getSleep (500, 0);
	*/

	for (lTracingThreadIndex = 0; lTracingThreadIndex < THREAD_NUMBER;
		lTracingThreadIndex++)
	{
		if ((ptPThread [lTracingThreadIndex] = new LoopThread_t ()) ==
			(LoopThread_p) NULL)
		{
			std:: cerr << "ptPThread new failed" << std:: endl;

			return 1;
		}

		if ((ptPThread [lTracingThreadIndex]) -> init () != errNoError)
		{
			std:: cerr << "ptPThread. init failed" << std:: endl;

			return 1;
		}

		if ((ptPThread [lTracingThreadIndex]) -> start () != errNoError)
		{
			std:: cerr << "pttTracingThread. start failed" << std:: endl;

			return 1;
		}
	}

	Error			errJoinReturn;
	long			lErrorCounter;

	// while (1)
	{
		lErrorCounter		= 0;

		#ifdef WIN32
			WinThread:: getSleep (5, 0);
		#else
			PosixThread:: getSleep (5, 0);
		#endif

		for (lTracingThreadIndex = 0; lTracingThreadIndex < THREAD_NUMBER;
			lTracingThreadIndex++)
		{
			if ((errJoinReturn =
				(ptPThread [lTracingThreadIndex]) -> join (&errJoin)) !=
				errNoError)
			{
				if ((long) errJoinReturn == THREADLIB_OPERATIONNOTALLOWED)
					lErrorCounter++;
				else
				{
					std:: cerr << "join failed" << std:: endl;

					return 1;
				}
			}
		}

		// if (lErrorCounter == THREAD_NUMBER)
		// 	break;
	}

/*
	long		lMinutes		= 60;

	PThread:: getSleep (lMinutes * 60, 0);
	std:: cout << "Sleep finish" << std:: endl;
*/

/*
	if (gtTracer. join () != errNoError)
	{
		std:: cerr << "Tracer. join failed" << std:: endl;

		gtTracer. finish ();

		return 1;
	}
*/

	if (gtTracer. cancel () != errNoError)
	{
		std:: cout << "Tracer. cancel failed" << std:: endl;

		return 1;
	}

	#ifdef WIN32
		WinThread:: getSleep (lSecondsBetweenTwoCheckTraceToManage, 0);
	#else
		PosixThread:: getSleep (lSecondsBetweenTwoCheckTraceToManage, 0);
	#endif

	if (gtTracer. finish (true) != errNoError)
	{
		std:: cout << "Tracer. finish failed" << std:: endl;

		return 1;
	}


	return 0;
}

