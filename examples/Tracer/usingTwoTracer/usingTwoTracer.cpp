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

#include "LoopThread.h"
#include "Tracer.h"
#include <iostream>
#ifdef WIN32
	#include <windows.h>
#endif

#define THREAD_NUMBER					30

Tracer_p			pgtTracer1;
Tracer_p			pgtTracer2;



int main (int argc, char **argv)

{

	Tracer_t							tTracer1;
	Tracer_t							tTracer2;
	LoopThread_p						ptPThread [THREAD_NUMBER];
	long								lTracingThreadIndex;
	long								lSecondsBetweenTwoCheckTraceToManage;


	lSecondsBetweenTwoCheckTraceToManage		= 7;

	if (tTracer1. init (
		"Tracer1",						// pName
		100,							// lCacheSizeOfTraceFile K-byte
		"./",							// pTraceDirectory
		"traceFile1",					// pTraceFileName
		1000,							// lMaxTraceFileSize K-byte
		30,								// lTraceFilePeriodInSecs
		false,							// bCompressedTraceFile
		true,							// _bClosedFileToBeCopied
		"./tmp",						// _pClosedFilesRepository
		100,								// lTraceFilesNumberToMaintain
		true,							// bTraceOnFile
		false,							// bTraceOnTTY
		0,								// lTraceLevel
		lSecondsBetweenTwoCheckTraceToManage,
		3000,							// lMaxTracesNumber
		7532,							// lListenPort
		10000,							// lTracesNumberAllocatedOnOverflow
		1000) != errNoError)			// lSizeOfEachBlockToGzip K-byte
	{

		std:: cerr << "Tracer. init failed" << std:: endl;

		return 1;
	}

	if (tTracer2. init (
		"Tracer2",						// pName
		100,							// lCacheSizeOfTraceFile K-byte
		"./",							// pTraceDirectory
		"traceFile2",					// pTraceFileName
		1000,							// lMaxTraceFileSize K-byte
		30,								// lTraceFilePeriodInSecs
		true,							// bCompressedTraceFile
		true,							// _bClosedFileToBeCopied
		"./tmp",						// _pClosedFilesRepository
		100,								// lTraceFilesNumberToMaintain
		true,							// bTraceOnFile
		false,							// bTraceOnTTY
		0,								// lTraceLevel
		lSecondsBetweenTwoCheckTraceToManage,
		3000,							// lMaxTracesNumber
		7533,							// lListenPort
		10000,							// lTracesNumberAllocatedOnOverflow
		1000) != errNoError)			// lSizeOfEachBlockToGzip K-byte
	{

		std:: cerr << "Tracer. init failed" << std:: endl;

		tTracer1. finish (true);

		return 1;
	}

	if (tTracer1. start () != errNoError)
	{
		std:: cerr << "Tracer. start failed" << std:: endl;

		tTracer2. finish (true);
		tTracer1. finish (true);

		return 1;
	}

	if (tTracer2. start () != errNoError)
	{
		std:: cerr << "Tracer. start failed" << std:: endl;

		tTracer2. finish (true);
		tTracer1. finish (true);

		return 1;
	}

	pgtTracer1		= &tTracer1;
	pgtTracer2		= &tTracer2;

	for (lTracingThreadIndex = 0; lTracingThreadIndex < THREAD_NUMBER;
		lTracingThreadIndex++)
	{
		if ((ptPThread [lTracingThreadIndex] = new LoopThread_t ()) ==
			(LoopThread_p) NULL)
		{
			std:: cerr << "ptPThread new failed" << std:: endl;

			return 1;
		}

		if ((ptPThread [lTracingThreadIndex]) -> init (&tTracer1, &tTracer2) !=
			errNoError)
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

	Error			errReturn;
	Error			errJoin;
	long			lErrorCounter;

	while (1)
	{
		lErrorCounter		= 0;

		#ifdef WIN32
				WinThread:: getSleep (60, 0);
		#else
				PosixThread:: getSleep (60, 0);
		#endif

		for (lTracingThreadIndex = 0; lTracingThreadIndex < THREAD_NUMBER;
			lTracingThreadIndex++)
		{
			if ((errReturn =
				(ptPThread [lTracingThreadIndex]) -> join (&errJoin)) !=
				errNoError)
			{
				if ((long) errReturn == THREADLIB_OPERATIONNOTALLOWED)
					lErrorCounter++;
				else
				{
					std:: cerr << "join failed" << std:: endl;

					return 1;
				}
			}
		}

		if (lErrorCounter == THREAD_NUMBER)
			break;
	}

/*
	long		lMinutes		= 60;

	PThread:: getSleep (lMinutes * 60, 0);
	std:: cout << "Sleep finish" << std:: endl;
*/

/*
	if (tTracer. join () != errNoError)
	{
		std:: cerr << "Tracer. join failed" << std:: endl;

		tTracer. finish ();

		return 1;
	}
*/

	if (tTracer1. cancel () != errNoError)
	{
		std:: cout << "Tracer. cancel failed" << std:: endl;

		return 1;
	}

	if (tTracer2. cancel () != errNoError)
	{
		std:: cout << "Tracer. cancel failed" << std:: endl;

		return 1;
	}

	#ifdef WIN32
			WinThread:: getSleep (lSecondsBetweenTwoCheckTraceToManage, 0);
	#else
			PosixThread:: getSleep (lSecondsBetweenTwoCheckTraceToManage, 0);
	#endif

	if (tTracer1. finish (true) != errNoError)
	{
		std:: cout << "Tracer. finish failed" << std:: endl;

		return 1;
	}

	if (tTracer2. finish (true) != errNoError)
	{
		std:: cout << "Tracer. finish failed" << std:: endl;

		return 1;
	}


	return 0;
}

