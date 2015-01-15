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
#include "CollectionTimes.h"
#include "DistributionTimes.h"
#include <stdlib.h>
#include <sys/stat.h>
#include <iostream>


int main (void)

{

	Scheduler_t				schScheduler;
	CollectionTimes_t		ctCollectionTimes;
	DistributionTimes_t		dtDistributionTimes;
	Boolean_t				bEnd;
	char					pInputBuffer [1024];
	Error_t					errJoin;


	if (schScheduler. init (10) != errNoError)
	{
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

	if (ctCollectionTimes. init (15) != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_INIT_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (dtDistributionTimes. init (10) != errNoError)
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
		std:: cout << "1. Collection start" << std:: endl;
		std:: cout << "2. Collection stop" << std:: endl;
		std:: cout << "3. Distribution start" << std:: endl;
		std:: cout << "4. Distribution stop" << std:: endl;
		std:: cout << "5. Scheduler active Collection (activeTimes)" << std:: endl;
		std:: cout << "6. Scheduler deactive Collection (deactiveTimes)" << std:: endl;
		std:: cout << "7. Scheduler active Distribution (activeTimes)" << std:: endl;
		std:: cout << "8. Scheduler deactive Distribution (deactiveTimes)" << std:: endl;
		std:: cout << "9. Scheduler suspend" << std:: endl;
		std:: cout << "10. Scheduler resume" << std:: endl;
		std:: cout << "11. Scheduler cancel (Exit)" << std:: endl;

		std:: cin >> pInputBuffer;

		switch (atoi (pInputBuffer))
		{
			case 1:	
				if (ctCollectionTimes. start () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_TIMES_START_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 2:
				if (ctCollectionTimes. stop () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_TIMES_STOP_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}


				break;
			case 3:	
				if (dtDistributionTimes. start () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_TIMES_START_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 4:
				if (dtDistributionTimes. stop () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_TIMES_STOP_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}


				break;
			case 5:
				{
					Error	err;

					if ((err = schScheduler. activeTimes (
						&ctCollectionTimes)) != errNoError)
					{
						std:: cout << (const char *) err << std:: endl;

						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULER_ACTIVETIMES_FAILED);

						std:: cout << (const char *) err << std:: endl;
					}
				}

				break;
			case 6:
				if (schScheduler. deactiveTimes (&ctCollectionTimes) !=
					errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_DEACTIVETIMES_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 7:
				if (schScheduler. activeTimes (&dtDistributionTimes) !=
					errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_ACTIVETIMES_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 8:
				if (schScheduler. deactiveTimes (&dtDistributionTimes) !=
					errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_DEACTIVETIMES_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 9:
				if (schScheduler. suspend () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_SUSPEND_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 10:
				if (schScheduler. resume () != errNoError)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULER_RESUME_FAILED);

					std:: cout << (const char *) err << std:: endl;
				}

				break;
			case 11:
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

	// At this point the join method is useless because
	//	to exit of the previous loop the user must have
	//	provoked the call to the cancel method
	//	of the schScheduler object
	if (schScheduler. join (&errJoin) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PTHREAD_JOIN_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (dtDistributionTimes. finish () != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_FINISH_FAILED);

		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	if (ctCollectionTimes. finish () != errNoError)
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

