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

#include "PrinterSpooler.h"
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>


#ifdef WIN32
	PrinterSpooler:: PrinterSpooler (void): WinThread ()
#else
	PrinterSpooler:: PrinterSpooler (void): PosixThread ()
#endif

{

}


PrinterSpooler:: ~PrinterSpooler (void)

{

}


Error PrinterSpooler:: init (EventsSet_p pesEventsSet,
	unsigned long ulThreadIndex)

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
	_ulThreadIndex			= ulThreadIndex;


	return errNoError;
}


Error PrinterSpooler:: run (void)

{

	Event_p			peEvent;
	Boolean_t		bAreRequestsFinished;
	long			lEventTypeIdentifier;
	Error			errGetAndRemove;
	Buffer_t		bDestination;


	if (bDestination. init ("PrinterSpooler") != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	bAreRequestsFinished		= false;

	while (!bAreRequestsFinished)
	{
		while ((errGetAndRemove = _pesEventsSet ->
			getAndRemoveFirstEvent (&bDestination,
			&peEvent, true, 10, 0)) == errNoError)
		{
			if (peEvent -> getTypeIdentifier (&lEventTypeIdentifier) !=
				errNoError)
			{
				_erThreadReturn = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENT_GETTYPEIDENTIFIER_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				bAreRequestsFinished		= true;

				break;
			}

			if (_pesEventsSet -> releaseEvent (0, peEvent) !=
				errNoError)
			{
				_erThreadReturn = EventsSetErrors (__FILE__, __LINE__,
					EVSET_EVENTSSET_RELEASEEVENT_FAILED);
				std:: cout << (const char *) _erThreadReturn << std:: endl;

				break;
			}

			if (lEventTypeIdentifier == PS_REQUEST_ARRIVED)
				std:: cout << "ulThreadIndex: " << _ulThreadIndex
					<< ", PrinterSpooler: Request arrived" << std:: endl;
			else
			{
				bAreRequestsFinished		= true;
			}
		}

		if ((unsigned long) errGetAndRemove == EVSET_EVENTSSET_DESTINATIONNOTFOUND)
		{
			#ifdef WIN32
				WinThread:: getSleep (1, 0);
			#else
				PosixThread:: getSleep (1, 0);
			#endif
		}
		std:: cout << "ulThreadIndex: " << _ulThreadIndex << ", "<< (const char *) errGetAndRemove << std:: endl;

		/*
		if (errGetAndRemove != errNoError &&
			(long) errGetAndRemove != EVSET_EVENTSSET_NOEVENTSFOUND &&
			(long) errGetAndRemove != EVSET_EVENTSSET_DESTINATIONNOTFOUND &&
			(long) errGetAndRemove != EVSET_EVENTSSET_EVENTNOTEXPIREDYET)
		{
			std:: cout << (const char *) errGetAndRemove << std:: endl;

			_erThreadReturn = EventsSetErrors (__FILE__, __LINE__,
				EVSET_EVENTSSET_GETANDREMOVEFIRSTEVENT_FAILED);
			std:: cout << (const char *) _erThreadReturn << std:: endl;

			break;
		}
		*/
	}

	if (bDestination. finish () != errNoError)
	{
		_erThreadReturn = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);
		std:: cout << (const char *) _erThreadReturn << std:: endl;

		return _erThreadReturn;
	}

	std:: cout << "PrinterSpooler shutdown" << std:: endl;


	return _erThreadReturn;
}


Error PrinterSpooler:: cancel (void)

{

	#ifdef WIN32
		// no cancel available for Windows threads

		return errNoError;
	#else
		return PosixThread:: cancel ();
	#endif
}
