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

#include "Scheduler.h"
#include "TimesForMeeting1.h"
#include <iostream>
#include <stdlib.h>


int main (void)

{

	Scheduler_t				schScheduler;
	TimesForMeeting1_t		mMeeting1;
	Boolean_t				bEnd;
	char					pInputBuffer [1024];
	Error					errSchedulerInit;
	Error					errJoin;


	if ((errSchedulerInit = schScheduler. init ()) != errNoError)
	{
		std:: cout << (const char *) errSchedulerInit << std:: endl;
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_INIT_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (schScheduler. start () != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_START_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (mMeeting1. init (5) != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_INIT_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	bEnd = false;
	while (!bEnd)
	{
		std:: cout << std:: endl << std:: endl;
		std:: cout << "1. Meeting 1 start" << std:: endl;
		std:: cout << "2. Meeting 1 stop" << std:: endl;
		std:: cout << "3. Scheduler activeTimes" << std:: endl;
		std:: cout << "4. Scheduler deactiveTimes" << std:: endl;
		std:: cout << "5. Scheduler suspend" << std:: endl;
		std:: cout << "6. Scheduler resume" << std:: endl;
		std:: cout << "7. Scheduler cancel (Exit)" << std:: endl;

		std:: cin >> pInputBuffer;

		switch (atoi (pInputBuffer))
		{
			case 1:	
				if (mMeeting1. start () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_TIMES_START_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 2:
				if (mMeeting1. stop () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_TIMES_STOP_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}


				break;
			case 3:
				if (schScheduler. activeTimes (&mMeeting1) != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_ACTIVETIMES_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 4:
				if (schScheduler. deactiveTimes (&mMeeting1) != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_DEACTIVETIMES_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 5:
				if (schScheduler. suspend () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_SUSPEND_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 6:
				if (schScheduler. resume () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_RESUME_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 7:
				if (schScheduler. cancel () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_CANCEL_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				bEnd = true;


				break;
		}
	}

	if (schScheduler. join (&errJoin) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_JOIN_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (mMeeting1. finish () != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_FINISH_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (schScheduler. finish (true) != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SCHEDULER_FINISH_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}


	return 0;
}
