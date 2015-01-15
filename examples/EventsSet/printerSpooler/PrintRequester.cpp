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

#include "PrintRequester.h"
#include "FileReader.h"
#include "FileIO.h"
#include "DateTime.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>


#ifdef WIN32
	PrintRequester:: PrintRequester (void): WinThread ()
#else
	PrintRequester:: PrintRequester (void): PosixThread ()
#endif

{

}


PrintRequester:: ~PrintRequester (void)

{

}


Error PrintRequester:: init (EventsSet_p pesEventsSet)

{

	#ifdef WIN32
		if (WinThread:: init () != errNoError)
	#else
		if (PosixThread:: init () != errNoError)
	#endif
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_INIT_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return err;
	}

	_pesEventsSet			= pesEventsSet;


	return errNoError;
}


Error PrintRequester:: run (void)

{

	Boolean_t		bAreRequestsFinished;
	FileReader		frRequestFile;
	char			pRequest [1024];
	#ifdef WIN32
		__int64		ullCharsRead;
		__int64		ullNowLocalInMilliSecs;
	#else
		unsigned long long		ullCharsRead;
		unsigned long long		ullNowLocalInMilliSecs;
	#endif
	Event_p			peEvent;
	long			lEventTypeIdentifier;
	Boolean_t		bIsRequestArrived;


	bAreRequestsFinished		= false;

	while (!bAreRequestsFinished)
	{
		#ifdef WIN32
			if (WinThread:: getSleep (5, 0) != errNoError)
		#else
			if (PosixThread:: getSleep (5, 0) != errNoError)
		#endif
		{
			_erThreadReturn = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PTHREAD_GETSLEEP_FAILED);
			std:: cout << (const char *) _erThreadReturn << std:: endl;

			break;
		}

		bIsRequestArrived		= false;

		if (frRequestFile. init ("printRequest", 1024) !=
			errNoError)
		{
			_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_INIT_FAILED,
				1, "printRequest");
			std:: cout << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}

		while (frRequestFile. readLine (pRequest, 1024,
			&ullCharsRead) == errNoError)
		{
			if (!strcmp (pRequest, ""))
				break;
			else
			{
				std:: cout << "PrintRequester: Request: " << pRequest << std:: endl;
				bIsRequestArrived		= true;
			}

			if (_pesEventsSet -> getFreeEvent (0, &peEvent) != errNoError)
			{
				_erThreadReturn = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_GETFREEEVENT_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				bAreRequestsFinished		= true;

				break;
			}

			if (DateTime:: nowLocalInMilliSecs (
				&ullNowLocalInMilliSecs) != errNoError)
			{
				_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				return _erThreadReturn;
			}


			if (!strcmp (pRequest, "NoRequest"))
				lEventTypeIdentifier		= PS_REQUEST_FINISHED;
			else
				lEventTypeIdentifier		= PS_REQUEST_ARRIVED;

			/*
			ullNowLocalInMilliSecs			=
				ullNowLocalInMilliSecs + 30000;
			*/

			if (peEvent -> init ("PrintRequester",
				"PrintRequester",
				lEventTypeIdentifier, "", ullNowLocalInMilliSecs) !=
				errNoError)
			{
				_erThreadReturn = EventsSetErrors (__FILE__, __LINE__,
					EVSET_NEW_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				bAreRequestsFinished		= true;

				break;
			}

			if (_pesEventsSet -> addEvent (peEvent) !=
				errNoError)
			{
				_erThreadReturn = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_ADDEVENT_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				bAreRequestsFinished		= true;

				break;
			}

			if (!strcmp (pRequest, "NoRequest"))
			{
				bAreRequestsFinished		= true;

				break;
			}
		}

		if (frRequestFile. finish () != errNoError)
		{
			_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
				TOOLS_FILEREADER_FINISH_FAILED);
			std:: cout << (const char *) _erThreadReturn << std:: endl;

			return _erThreadReturn;
		}

		if (bIsRequestArrived)
		{
			int				iRequestFile;

			if (FileIO:: open ("printRequest", O_WRONLY | O_TRUNC, 
				&iRequestFile) != errNoError)
			{
				_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_OPEN_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				return _erThreadReturn;
			}

			if (FileIO:: close (iRequestFile) != errNoError)
			{
				_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
					TOOLS_FILEIO_CLOSE_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				return _erThreadReturn;
			}
		}
	}

	std:: cout << "PrintRequester shutdown" << std:: endl;


	return _erThreadReturn;
}


Error PrintRequester:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}
