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


#include "Times.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#ifdef WIN32
	#include <Windows.h>
#endif
#include <iostream>
#include "DateTime.h"



Times:: Times (void)

{

	_schTimesStatus		= SCHTIMES_BUILDED;

}


Times:: ~Times (void)

{

}


Times:: Times (const Times &t)

{

	assert (1==0);

	// to do

	*this = t;
}


Error Times:: init (
	unsigned long ulPeriodInMilliSeconds, const char *pClassName)

{

	if (_schTimesStatus != SCHTIMES_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		return err;
	}

	_ulPeriodInMilliSeconds		= ulPeriodInMilliSeconds;
	_ttTimesType				= SCH_TYPE_PERIODIC;

	// _bCurrentDaylightSavingTime
	strcpy (_pCurrentExpirationDateTime, "");
	// _bNextDaylightSavingTime
	strcpy (_pNextExpirationDateTime, "");

	if (_mtTimesMutex. init (PMutex:: MUTEX_FAST) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);

		return err;
	}

	if (pClassName != (const char *) NULL)
	{
		if (strlen (pClassName) > SCH_MAXCLASSNAMELENGTH - 1)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_CLASSNAME_TOOLONG);

			if (_mtTimesMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_INIT_FAILED);
			}

			return err;
		}

		strcpy (_pClassName, pClassName);
	}
	else
		strcpy (_pClassName, "");


	_schTimesStatus		= SCHTIMES_INITIALIZED;


	return errNoError;
}


Error Times:: init (const char *pSchedule, const char *pClassName)

{

	// schedule format:
	//	<year> <month> <monthday> <weekday> <hour> <minute> <second>

	if (_schTimesStatus != SCHTIMES_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		return err;
	}

	if (pSchedule == (const char *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		return err;
	}

	// checks on pSchedule parameter
	{

		long		lScheduleLength;

		
		lScheduleLength		= strlen (pSchedule);

		// check length
		{
			if (lScheduleLength > SCH_MAXSCHEDULELENGTH - 1)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_ACTIVATION_WRONG);

				return err;
			}
		}

		// check spaces number and characters (digit, '*', ',', '-', ' ')
		{
			long			lSpacesNumber;
			long			lScheduleIndex;


			lSpacesNumber		= 0;

			for (lScheduleIndex = 0; lScheduleIndex < lScheduleLength;
				lScheduleIndex++)
			{
				if (!isdigit (pSchedule [lScheduleIndex]))
				{
					if (pSchedule [lScheduleIndex] != '*')
					{
						if (pSchedule [lScheduleIndex] != ',')
						{
							if (pSchedule [lScheduleIndex] != '-')
							{
								if (pSchedule [lScheduleIndex] != ' ')
								{
									Error err = SchedulerErrors (__FILE__,
										__LINE__, SCH_ACTIVATION_WRONG);

									return err;
								}
								else
								{
									lSpacesNumber++;
								}
							}
						}
					}
				}
			}

			if (lSpacesNumber != 6)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_ACTIVATION_WRONG);

				return err;
			}
		}

		// check range values
		{
			char				pScheduleCopy [SCH_MAXSCHEDULELENGTH];
			char				pYear [SCH_MAXSCHEDULELENGTH];
			long				lMonth;
			char				*pScheduleToken;
			char				*pScheduleTokenLast;


			strcpy (pScheduleCopy, pSchedule);

			// year
			if ((pScheduleToken = strtok_r (pScheduleCopy, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			strcpy (pYear, pScheduleToken);

			// month

			lMonth				= -1;

			if ((pScheduleToken = strtok_r ((char *) NULL, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			if (strchr (pScheduleToken, ',') != (char *) NULL)
			{
				long			lValue;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				char			*pPointerToScheduleToken;
				long			lNumberLength;


				pPointerToScheduleToken			= pScheduleToken;

				do
				{
					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						lNumberLength		=
							strlen (pPointerToScheduleToken);
					else
						lNumberLength		=
							strchr (pPointerToScheduleToken, ',') -
							pPointerToScheduleToken;

					strncpy (pNumber, pPointerToScheduleToken,
						lNumberLength);

					pNumber [lNumberLength]				= '\0';

					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						pPointerToScheduleToken				+=
							lNumberLength;
					else
						pPointerToScheduleToken				+=
							(lNumberLength + 1);

					lValue			= atol (pNumber);

					if (lValue < 1 || lValue > 12)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue, "month");

						return err;
					}
				}
				while (*pPointerToScheduleToken != '\0');

				/* PRIMA (strtok_r annidate non funzionano con windows)
				// comma separator
				if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				do
				{
					lValue			= atol (pFieldToken);

					if (lValue < 1 || lValue > 12)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue, "month");

						return err;
					}

					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
				while (false);
				*/
			}
			else if (strchr (pScheduleToken, '-') != (char *) NULL)
			{
				// values range
				long			lValueFrom;
				long			lValueTo;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				long			lNumberLength;


				lNumberLength		= strchr (pScheduleToken, '-') -
					pScheduleToken;

				strncpy (pNumber, pScheduleToken, lNumberLength);

				pNumber [lNumberLength]				= '\0';

				lValueFrom			= atol (pNumber);

				strcpy (pNumber, pScheduleToken + lNumberLength + 1);

				lValueTo			= atol (pNumber);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				// dash separator
				if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueFrom		 = atol (pFieldToken);

				if ((pFieldToken = strtok_r ((char *) NULL, "-",
					&pFieldTokenLast)) == (char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueTo		 = atol (pFieldToken);
				*/

				if (lValueFrom < 1 || lValueFrom > lValueTo)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueFrom, "month");

					return err;
				}

				if (lValueTo > 12)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueTo, "month");

					return err;
				}
			}
			else
			{	// single value or '*'

				if (pScheduleToken [0] != '*')
				{
					lMonth			= atol (pScheduleToken);

					if (lMonth < 1 || lMonth > 12)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lMonth, "month");

						return err;
					}
				}
			}

			// monthday
			if ((pScheduleToken = strtok_r ((char *) NULL, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			if (strchr (pScheduleToken, ',') != (char *) NULL)
			{
				long			lValue;
				Boolean_t		bIsLessThan30;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				char			*pPointerToScheduleToken;
				long			lNumberLength;


				pPointerToScheduleToken			= pScheduleToken;

				bIsLessThan30				= false;

				do
				{
					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						lNumberLength		=
							strlen (pPointerToScheduleToken);
					else
						lNumberLength		=
							strchr (pPointerToScheduleToken, ',') -
							pPointerToScheduleToken;

					strncpy (pNumber, pPointerToScheduleToken,
						lNumberLength);

					pNumber [lNumberLength]				= '\0';

					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						pPointerToScheduleToken				+=
							lNumberLength;
					else
						pPointerToScheduleToken				+=
							(lNumberLength + 1);

					lValue			= atol (pNumber);

					if (lValue < 1 || lValue > 31)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue,
							"monthday");

						return err;
					}

					if (lValue <= 30)
						bIsLessThan30				= true;
				}
				while (*pPointerToScheduleToken != '\0');

				/* PRIMA (strtok_r annidate non funzionano con windows)
				long			lValue;
				Boolean_t		bIsLessThan30;


				// comma separator
				if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				bIsLessThan30				= false;

				do
				{
					lValue			= atol (pFieldToken);

					if (lValue < 1 || lValue > 31)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue,
							"monthday");

						return err;
					}

					if (lValue <= 30)
						bIsLessThan30				= true;

					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
				while (false);
				*/

				// if year is *, month is february and day is 30 or 31
				// we have a loop
				if (!strcmp (pYear, "*") && lMonth == 2 && !bIsLessThan30)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG,
						2, lValue,
					"monthday (with '*' as year and february as month)");

					return err;
				}
			}
			else if (strchr (pScheduleToken, '-') != (char *) NULL)
			{
				// values range
				long			lValueFrom;
				long			lValueTo;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				long			lNumberLength;


				lNumberLength		= strchr (pScheduleToken, '-') -
					pScheduleToken;

				strncpy (pNumber, pScheduleToken, lNumberLength);

				pNumber [lNumberLength]				= '\0';

				lValueFrom			= atol (pNumber);

				strcpy (pNumber, pScheduleToken + lNumberLength + 1);

				lValueTo			= atol (pNumber);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				// dash separator
				if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueFrom		 = atol (pFieldToken);

				if ((pFieldToken = strtok_r ((char *) NULL, "-",
					&pFieldTokenLast)) == (char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueTo		 = atol (pFieldToken);
				*/

				if (lValueFrom < 1 || lValueFrom > lValueTo)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueFrom, "month");

					return err;
				}

				if (lValueTo > 31)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueTo, "month");

					return err;
				}

				// if year is *, month is february and day is 30 or 31
				// we have a loop
				if (!strcmp (pYear, "*") && lMonth == 2 && lValueFrom == 30 &&
					lValueTo == 31)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG,
						2, lValueFrom,
					"monthday (with '*' as year and february as month)");

					return err;
				}
			}
			else
			{	// single value or '*'

				if (pScheduleToken [0] != '*')
				{
					long		lMonthDay;

					lMonthDay		= atol (pScheduleToken);

					if (lMonthDay < 1 || lMonthDay > 31)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lMonthDay,
							"monthday");

						return err;
					}

					// if year is *, month is february and day is 30 or 31
					// we have a loop
					if (!strcmp (pYear, "*") && lMonth == 2 && lMonthDay == 30)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG,
							2, lMonthDay,
						"monthday (with '*' as year and february as month)");

						return err;
					}
				}
			}

			// weekday
			if ((pScheduleToken = strtok_r ((char *) NULL, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			if (strchr (pScheduleToken, ',') != (char *) NULL)
			{
				long			lValue;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				char			*pPointerToScheduleToken;
				long			lNumberLength;


				pPointerToScheduleToken			= pScheduleToken;

				do
				{
					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						lNumberLength		=
							strlen (pPointerToScheduleToken);
					else
						lNumberLength		=
							strchr (pPointerToScheduleToken, ',') -
							pPointerToScheduleToken;

					strncpy (pNumber, pPointerToScheduleToken,
						lNumberLength);

					pNumber [lNumberLength]				= '\0';

					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						pPointerToScheduleToken				+=
							lNumberLength;
					else
						pPointerToScheduleToken				+=
							(lNumberLength + 1);

					lValue			= atol (pNumber);

					if (lValue < 0 || lValue > 6)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue,
							"weekday");

						return err;
					}
				}
				while (*pPointerToScheduleToken != '\0');

				/* PRIMA (strtok_r annidate non funzionano con windows)
				long			lValue;


				// comma separator
				if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				do
				{
					lValue			= atol (pFieldToken);

					if (lValue < 0 || lValue > 6)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue,
							"weekday");

						return err;
					}

					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
				while (false);
				*/
			}
			else if (strchr (pScheduleToken, '-') != (char *) NULL)
			{
				// values range
				long			lValueFrom;
				long			lValueTo;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				long			lNumberLength;


				lNumberLength		= strchr (pScheduleToken, '-') -
					pScheduleToken;

				strncpy (pNumber, pScheduleToken, lNumberLength);

				pNumber [lNumberLength]				= '\0';

				lValueFrom			= atol (pNumber);

				strcpy (pNumber, pScheduleToken + lNumberLength + 1);

				lValueTo			= atol (pNumber);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				// dash separator
				if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueFrom		 = atol (pFieldToken);

				if ((pFieldToken = strtok_r ((char *) NULL, "-",
					&pFieldTokenLast)) == (char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueTo		 = atol (pFieldToken);
				*/

				if (lValueFrom < 0 || lValueFrom > lValueTo)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueFrom, "month");

					return err;
				}

				if (lValueTo > 6)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueTo, "month");

					return err;
				}
			}
			else
			{	// single value or '*'

				if (pScheduleToken [0] != '*')
				{
					long			lWeekDay;


					lWeekDay		= atol (pScheduleToken);

					if (lWeekDay < 0 || lWeekDay > 6)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lWeekDay,
							"weekday");

						return err;
					}
				}
			}

			// hour
			if ((pScheduleToken = strtok_r ((char *) NULL, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			if (strchr (pScheduleToken, ',') != (char *) NULL)
			{
				long			lValue;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				char			*pPointerToScheduleToken;
				long			lNumberLength;


				pPointerToScheduleToken			= pScheduleToken;

				do
				{
					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						lNumberLength		=
							strlen (pPointerToScheduleToken);
					else
						lNumberLength		=
							strchr (pPointerToScheduleToken, ',') -
							pPointerToScheduleToken;

					strncpy (pNumber, pPointerToScheduleToken,
						lNumberLength);

					pNumber [lNumberLength]				= '\0';

					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						pPointerToScheduleToken				+=
							lNumberLength;
					else
						pPointerToScheduleToken				+=
							(lNumberLength + 1);

					lValue			= atol (pNumber);

					if (lValue < 0 || lValue > 23)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue, "hour");

						return err;
					}
				}
				while (*pPointerToScheduleToken != '\0');

				/* PRIMA (strtok_r annidate non funzionano con windows)
				long			lValue;


				// comma separator
				if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				do
				{
					lValue			= atol (pFieldToken);

					if (lValue < 0 || lValue > 23)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue, "hour");

						return err;
					}

					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
				while (false);
				*/
			}
			else if (strchr (pScheduleToken, '-') != (char *) NULL)
			{
				// values range
				long			lValueFrom;
				long			lValueTo;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				long			lNumberLength;


				lNumberLength		= strchr (pScheduleToken, '-') -
					pScheduleToken;

				strncpy (pNumber, pScheduleToken, lNumberLength);

				pNumber [lNumberLength]				= '\0';

				lValueFrom			= atol (pNumber);

				strcpy (pNumber, pScheduleToken + lNumberLength + 1);

				lValueTo			= atol (pNumber);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				// dash separator
				if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueFrom		 = atol (pFieldToken);

				if ((pFieldToken = strtok_r ((char *) NULL, "-",
					&pFieldTokenLast)) == (char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueTo		 = atol (pFieldToken);
				*/

				if (lValueFrom < 0 || lValueFrom > lValueTo)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueFrom, "month");

					return err;
				}

				if (lValueTo > 23)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueTo, "month");

					return err;
				}
			}
			else
			{	// single value or '*'

				if (pScheduleToken [0] != '*')
				{
					long			lHour;
					
					
					lHour			= atol (pScheduleToken);

					if (lHour < 0 || lHour > 23)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lHour, "hour");

						return err;
					}
				}
			}

			// minute
			if ((pScheduleToken = strtok_r ((char *) NULL, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			if (strchr (pScheduleToken, ',') != (char *) NULL)
			{
				long			lValue;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				char			*pPointerToScheduleToken;
				long			lNumberLength;


				pPointerToScheduleToken			= pScheduleToken;

				do
				{
					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						lNumberLength		=
							strlen (pPointerToScheduleToken);
					else
						lNumberLength		=
							strchr (pPointerToScheduleToken, ',') -
							pPointerToScheduleToken;

					strncpy (pNumber, pPointerToScheduleToken,
						lNumberLength);

					pNumber [lNumberLength]				= '\0';

					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						pPointerToScheduleToken				+=
							lNumberLength;
					else
						pPointerToScheduleToken				+=
							(lNumberLength + 1);

					lValue			= atol (pNumber);

					if (lValue < 0 || lValue > 59)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue,
							"minute");

						return err;
					}
				}
				while (*pPointerToScheduleToken != '\0');

				/* PRIMA (strtok_r annidate non funzionano con windows)
				long			lValue;


				// comma separator
				if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				do
				{
					lValue			= atol (pFieldToken);

					if (lValue < 0 || lValue > 59)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue,
							"minute");

						return err;
					}

					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
				while (false);
				*/
			}
			else if (strchr (pScheduleToken, '-') != (char *) NULL)
			{
				// values range
				long			lValueFrom;
				long			lValueTo;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				long			lNumberLength;


				lNumberLength		= strchr (pScheduleToken, '-') -
					pScheduleToken;

				strncpy (pNumber, pScheduleToken, lNumberLength);

				pNumber [lNumberLength]				= '\0';

				lValueFrom			= atol (pNumber);

				strcpy (pNumber, pScheduleToken + lNumberLength + 1);

				lValueTo			= atol (pNumber);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				// dash separator
				if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueFrom		 = atol (pFieldToken);

				if ((pFieldToken = strtok_r ((char *) NULL, "-",
					&pFieldTokenLast)) == (char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueTo		 = atol (pFieldToken);
				*/

				if (lValueFrom < 0 || lValueFrom > lValueTo)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueFrom, "month");

					return err;
				}

				if (lValueTo > 59)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueTo, "month");

					return err;
				}
			}
			else
			{	// single value or '*'

				if (pScheduleToken [0] != '*')
				{
					long			lMinute;
					
					
					lMinute			= atol (pScheduleToken);

					if (lMinute < 0 || lMinute > 59)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lMinute,
							"minute");

						return err;
					}
				}
			}

			// second
			if ((pScheduleToken = strtok_r ((char *) NULL, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			if (strchr (pScheduleToken, ',') != (char *) NULL)
			{
				long			lValue;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				char			*pPointerToScheduleToken;
				long			lNumberLength;


				pPointerToScheduleToken			= pScheduleToken;

				do
				{
					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						lNumberLength		=
							strlen (pPointerToScheduleToken);
					else
						lNumberLength		=
							strchr (pPointerToScheduleToken, ',') -
							pPointerToScheduleToken;

					strncpy (pNumber, pPointerToScheduleToken,
						lNumberLength);

					pNumber [lNumberLength]				= '\0';

					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						pPointerToScheduleToken				+=
							lNumberLength;
					else
						pPointerToScheduleToken				+=
							(lNumberLength + 1);

					lValue			= atol (pNumber);

					if (lValue < 0 || lValue > 59)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue,
							"second");

						return err;
					}
				}
				while (*pPointerToScheduleToken != '\0');

				/* PRIMA (strtok_r annidate non funzionano con windows)
				long			lValue;


				// comma separator
				if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				do
				{
					lValue			= atol (pFieldToken);

					if (lValue < 0 || lValue > 59)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValue,
							"second");

						return err;
					}

					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
				while (false);
				*/
			}
			else if (strchr (pScheduleToken, '-') != (char *) NULL)
			{
				// values range
				long			lValueFrom;
				long			lValueTo;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				long			lNumberLength;


				lNumberLength		= strchr (pScheduleToken, '-') -
					pScheduleToken;

				strncpy (pNumber, pScheduleToken, lNumberLength);

				pNumber [lNumberLength]				= '\0';

				lValueFrom			= atol (pNumber);

				strcpy (pNumber, pScheduleToken + lNumberLength + 1);

				lValueTo			= atol (pNumber);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				// dash separator
				if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueFrom		 = atol (pFieldToken);

				if ((pFieldToken = strtok_r ((char *) NULL, "-",
					&pFieldTokenLast)) == (char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueTo		 = atol (pFieldToken);
				*/

				if (lValueFrom < 0 || lValueFrom > lValueTo)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueFrom, "month");

					return err;
				}

				if (lValueTo > 59)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_SCHEDULEFIELDRANGE_WRONG, 2, lValueTo, "month");

					return err;
				}
			}
			else
			{	// single value or '*'

				if (pScheduleToken [0] != '*')
				{
					long			lSecond;


					lSecond		= atol (pScheduleToken);

					if (lSecond < 0 || lSecond > 59)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_SCHEDULEFIELDRANGE_WRONG, 2, lSecond,
							"second");

						return err;
					}
				}
			}
		}
	}

	strcpy (_pCalendarSchedule, pSchedule);
	_ttTimesType		= SCH_TYPE_CALENDAR;

	// _bCurrentDaylightSavingTime
	strcpy (_pCurrentExpirationDateTime, "");
	// _bNextDaylightSavingTime
	strcpy (_pNextExpirationDateTime, "");

	if (_mtTimesMutex. init (PMutex:: MUTEX_FAST) != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_INIT_FAILED);

		return err;
	}

	if (pClassName != (const char *) NULL)
	{
		if (strlen (pClassName) > SCH_MAXCLASSNAMELENGTH - 1)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_CLASSNAME_TOOLONG);

			if (_mtTimesMutex. finish () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_INIT_FAILED);
			}

			return err;
		}

		strcpy (_pClassName, pClassName);
	}
	else
		strcpy (_pClassName, "");


	_schTimesStatus		= SCHTIMES_INITIALIZED;


	return errNoError;
}


Error Times:: finish (void)

{

	if (_schTimesStatus == SCHTIMES_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		return err;
	}

	if (_mtTimesMutex. finish () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_FINISH_FAILED);

		return err;
	}

	_schTimesStatus		= SCHTIMES_BUILDED;


	return errNoError;
}


Error Times:: start (const char *pStartDateTime)

{

	if (_mtTimesMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schTimesStatus != SCHTIMES_INITIALIZED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		return err;
	}

	if (pStartDateTime != (const char *) NULL)
	{
		unsigned long		ulSrcYear;
		unsigned long		ulSrcMonth;
		unsigned long		ulSrcDay;
		unsigned long		ulSrcHour;
		unsigned long		ulSrcMinutes;
		unsigned long		ulSrcSeconds;
		unsigned long		ulSrcMilliSeconds;

		unsigned long		ulDestYear;
		unsigned long		ulDestMonth;
		unsigned long		ulDestDay;
		unsigned long		ulDestHour;
		unsigned long		ulDestMinutes;
		unsigned long		ulDestSeconds;
		unsigned long		ulDestMilliSeconds;


		if (sscanf (pStartDateTime,
			"%4lu-%2lu-%2lu %2lu:%2lu:%2lu:%4lu",
			&ulSrcYear,
			&ulSrcMonth,
			&ulSrcDay,
			&ulSrcHour,
			&ulSrcMinutes,
			&ulSrcSeconds,
			&ulSrcMilliSeconds) != 7)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SSCANF_FAILED);

			if (_mtTimesMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		ulDestMilliSeconds		= ulSrcMilliSeconds;

		if (DateTime:: addSeconds (
			ulSrcYear,
			ulSrcMonth,
			ulSrcDay,
			ulSrcHour,
			ulSrcMinutes,
			ulSrcSeconds,
			-1,
			0,
			&ulDestYear,
			&ulDestMonth,
			&ulDestDay,
			&ulDestHour,
			&ulDestMinutes,
			&ulDestSeconds,
			&_bCurrentDaylightSavingTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_ADDSECONDS_FAILED);

			if (_mtTimesMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		sprintf (_pCurrentExpirationDateTime,
			"%04lu-%02lu-%02lu %02lu:%02lu:%02lu:%04lu",
			ulDestYear,
			ulDestMonth,
			ulDestDay,
			ulDestHour,
			ulDestMinutes,
			ulDestSeconds,
			ulDestMilliSeconds);
	}
	else
	{
		tm					tmDateTime;
		unsigned long		ulMilliSecs;


		if (DateTime:: get_tm_LocalTime (
			&tmDateTime, &ulMilliSecs) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_GET_TM_LOCALTIME_FAILED);

			if (_mtTimesMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return err;
		}

		if (tmDateTime. tm_isdst == 1)
			_bCurrentDaylightSavingTime			= true;
		else
			_bCurrentDaylightSavingTime			= false;

		// the format date is yyyy-mm-dd-hh24-mi-ss-milliss
		sprintf (_pCurrentExpirationDateTime,
			"%04lu-%02lu-%02lu %02lu:%02lu:%02lu:%04lu",
			(unsigned long) (tmDateTime. tm_year + 1900),
			(unsigned long) (tmDateTime. tm_mon + 1),
			(unsigned long) (tmDateTime. tm_mday),
			(unsigned long) (tmDateTime. tm_hour),
			(unsigned long) (tmDateTime. tm_min),
			(unsigned long) (tmDateTime. tm_sec),
			ulMilliSecs);
	}

	if (_ttTimesType == SCH_TYPE_PERIODIC)
	{
		Error		errUpdateNextPeriodicExpirationDateTime;

		if ((errUpdateNextPeriodicExpirationDateTime =
			updateNextPeriodicExpirationDateTime ()) != errNoError)
		{
			if ((long) errUpdateNextPeriodicExpirationDateTime !=
				SCH_REACHED_LASTTIMEOUT)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_TIMES_UPDATENEXTPERIODICEXPIRATIONDATETIME_FAILED);

				if (_mtTimesMutex. unLock () != errNoError)
				{
					Error err = PThreadErrors (__FILE__, __LINE__,
						THREADLIB_PMUTEX_UNLOCK_FAILED);
				}

				return err;
			}
		}
	}
	else
	{		// SCH_TYPE_CALENDAR
		Error				errUpdateNextCalendarExpirationDateTime;

		if ((errUpdateNextCalendarExpirationDateTime =
			updateNextCalendarExpirationDateTime ()) != errNoError)
		{
			// Error err = SchedulerErrors (__FILE__, __LINE__,
			//	SCH_TIMES_UPDATENEXTCALENDAREXPIRATIONDATETIME_FAILED);

			if (_mtTimesMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);
			}

			return errUpdateNextCalendarExpirationDateTime;
		}
	}

	if (_mtTimesMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}

	_schTimesStatus		= SCHTIMES_STARTED;


	return errNoError;
}


Error Times:: stop (void)

{

	if (_mtTimesMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schTimesStatus != SCHTIMES_STARTED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		return err;
	}

	_schTimesStatus		= SCHTIMES_INITIALIZED;

	if (_mtTimesMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Times:: isStarted (Boolean_p pbIsStarted)

{

	if (_mtTimesMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schTimesStatus == SCHTIMES_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		return err;
	}

	if (pbIsStarted == (Boolean_p) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}

		return err;
	}

	if (_schTimesStatus == SCHTIMES_STARTED)
		*pbIsStarted		= true;
	else
		*pbIsStarted		= false;

	if (_mtTimesMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Times:: isExpiredTime (Boolean_p pbIsExpired)

{

	#ifdef WIN32
	#else
		struct timeval	tvTimeval;
	#endif
	char				pDateTime [SCH_MAXDATELENGTH];
	tm					tmDateTime;
	unsigned long		ulMilliSecs;


	if (_mtTimesMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schTimesStatus != SCHTIMES_STARTED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		return err;
	}

	if (pbIsExpired == (Boolean_p) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}

		return err;
	}

	if (DateTime:: get_tm_LocalTime (
		&tmDateTime, &ulMilliSecs) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_GET_TM_LOCALTIME_FAILED);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);
		}

		return err;
	}

	// if (tmDateTime. tm_isdst == 1 && !_bNextDaylightSavingTime)	// this check is wrong because in case of milliseconds period, tmDateTime. tm_isdst == 1 could not mean 'moving from DST to No DST' but could mean 'moving from No DST and we just entered in DST from few millisecs'.
	// So the correct check uses instead _bCurrentDaylightSavingTime
	if (tmDateTime. tm_isdst == 1 && _bCurrentDaylightSavingTime &&
		!_bNextDaylightSavingTime)
	{
		//  Scenario: 02:59:00 (DST) ... 03:00:00=02:00:00 (No DST) ...
		//
		// it means that the next time will change the daylight saving time
		// but the current time has still to change it.
		// In this case, we cannot compare _pNextExpirationDateTime
		// with the current date time because they belong to different
		// daylight saving time.
		// ------------------------------
		// 2010-11-04. Next statement is wrong:
		// In this scenario, we have to force the update of
		// _pNextExpirationDateTime in order to have both
		// _bCurrentDaylightSavingTime and _bNextDaylightSavingTime belonging
		// to the same daylight saving time.
		// To have the update we have to set *pbIsExpired to true otherwise
		// the Scheduler will not do any update.
		// -------------------------
		//
		// 2010-11-04: To be sure we are moving from DST to NoDST
		// (from 3am to 2am) we need all the above three conditions.
		//

		*pbIsExpired	= false;

		// In this scenario we are moving from 'DST' to
		// 'No DST' that it means from 3am to 2am (October 25).
		// For example, now is 2:59:10 and the period is 1 minute.
		// 	So NextTime is 2:00:10. In this case NextTime will expires
		// 	a lot of times between 2:59:10 and 2:59:59 and this is wrong.
		//
		// 	2010-11-04: For this reason, until the current time
		// 	will not be NoDST, we have to wait (*pbIsExpired = false)
		// 	because for sure the Times did not expire yet
	}
	else if (tmDateTime. tm_isdst == 0 && _bCurrentDaylightSavingTime &&
		_bNextDaylightSavingTime)
	{
		// 2011-11-02:
		//  Scenario: 02:59:00 (DST) ... 03:00:00=02:00:00 (No DST) ...
		//  This condition (above if) means:
		//  - the real time is in No DST (i.e.: 02:01:00 after 3am)
		//  - but still _pCurrentExpirationDateTime and _pNextExpirationDateTime
		//      are still in the DST, it means i.e. at 2:58:00

		*pbIsExpired    = true;
	}
	else
	{
		// the format date is yyyy-mm-dd-hh24-mi-ss-milliss
		sprintf (pDateTime, "%04lu-%02lu-%02lu %02lu:%02lu:%02lu:%04lu",
			(unsigned long) (tmDateTime. tm_year + 1900),
			(unsigned long) (tmDateTime. tm_mon + 1),
			(unsigned long) (tmDateTime. tm_mday),
			(unsigned long) (tmDateTime. tm_hour),
			(unsigned long) (tmDateTime. tm_min),
			(unsigned long) (tmDateTime. tm_sec),
			ulMilliSecs);

		if (strcmp (_pNextExpirationDateTime, pDateTime) <= 0)
			*pbIsExpired	= true;
		else
			*pbIsExpired	= false;
	}

	if (_mtTimesMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Times:: getClassName (char *pClassName)

{

	if (_mtTimesMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schTimesStatus == SCHTIMES_BUILDED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		return err;
	}

	if (pClassName == (char *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}

		return err;
	}

	strcpy (pClassName, _pClassName);

	if (_mtTimesMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Times:: getNextExpirationDateTime (char *pNextExpirationDateTime)

{

	if (_mtTimesMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schTimesStatus != SCHTIMES_STARTED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		return err;
	}

	if (pNextExpirationDateTime == (char *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			return err;
		}

		return err;
	}

	strcpy (pNextExpirationDateTime, _pNextExpirationDateTime);

	if (_mtTimesMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Times:: updateNextExpirationDateTime (void)

{

	if (_mtTimesMutex. lock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_LOCK_FAILED);

		return err;
	}

	if (_schTimesStatus != SCHTIMES_STARTED)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_OPERATION_NOTALLOWED, 1, _schTimesStatus);

		if (_mtTimesMutex. unLock () != errNoError)
		{
			Error err = PThreadErrors (__FILE__, __LINE__,
				THREADLIB_PMUTEX_UNLOCK_FAILED);

			// return err;
		}

		return err;
	}

	_bCurrentDaylightSavingTime			= _bNextDaylightSavingTime;
	strcpy (_pCurrentExpirationDateTime, _pNextExpirationDateTime);

	if (_ttTimesType == SCH_TYPE_PERIODIC)
	{
		Error				errUpdateNextPeriodicExpirationDateTime;


		if ((errUpdateNextPeriodicExpirationDateTime =
			updateNextPeriodicExpirationDateTime ()) != errNoError)
		{
			if ((long) errUpdateNextPeriodicExpirationDateTime ==
				SCH_REACHED_LASTTIMEOUT)
				_schTimesStatus		= SCHTIMES_INITIALIZED;

			if (_mtTimesMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);

				return err;
			}

			return errUpdateNextPeriodicExpirationDateTime;
		}
	}
	else		 // SCH_TYPE_CALENDAR
	{
		Error				errUpdateNextCalendarExpirationDateTime;


		if ((errUpdateNextCalendarExpirationDateTime =
			updateNextCalendarExpirationDateTime ()) != errNoError)
		{
			if ((long) errUpdateNextCalendarExpirationDateTime ==
				SCH_REACHED_LASTTIMEOUT)
				_schTimesStatus		= SCHTIMES_INITIALIZED;

			if (_mtTimesMutex. unLock () != errNoError)
			{
				Error err = PThreadErrors (__FILE__, __LINE__,
					THREADLIB_PMUTEX_UNLOCK_FAILED);

				return err;
			}

			return errUpdateNextCalendarExpirationDateTime;
		}
	}

	if (_mtTimesMutex. unLock () != errNoError)
	{
		Error err = PThreadErrors (__FILE__, __LINE__,
			THREADLIB_PMUTEX_UNLOCK_FAILED);

		return err;
	}


	return errNoError;
}


Error Times:: updateNextPeriodicExpirationDateTime (void)

{

	if (addMilliSecondsToDateTime (
		_pNextExpirationDateTime, &_bNextDaylightSavingTime,
		_pCurrentExpirationDateTime, _bCurrentDaylightSavingTime,
		_ulPeriodInMilliSeconds) != errNoError)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_TIMES_ADDSECONDSTODATETIME_FAILED);

		return err;
	}


	return errNoError;
}


Error Times:: addMilliSecondsToDateTime (
	char *pNextDateTime, Boolean_p pbNextDaylightSavingTime,
	const char *pCurrentDateTime, long lCurrentDaylightSavingTime,
	unsigned long ulMilliSeconds)

{

	unsigned long		ulCurrentYear;
	unsigned long		ulCurrentMonth;
	unsigned long		ulCurrentDay;
	unsigned long		ulCurrentHour;
	unsigned long		ulCurrentMinutes;
	unsigned long		ulCurrentSeconds;
	unsigned long		ulCurrentMilliSeconds;

	unsigned long		ulSecondsToAdd;

	unsigned long		ulNextYear;
	unsigned long		ulNextMonth;
	unsigned long		ulNextDay;
	unsigned long		ulNextHour;
	unsigned long		ulNextMinutes;
	unsigned long		ulNextSeconds;
	unsigned long		ulNextMilliSeconds;


	if (pNextDateTime == (char *) NULL ||
		pCurrentDateTime == (char *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		return err;
	}

	if (sscanf (pCurrentDateTime,
		"%4lu-%2lu-%2lu %2lu:%2lu:%2lu:%4lu",
		&ulCurrentYear,
		&ulCurrentMonth,
		&ulCurrentDay,
		&ulCurrentHour,
		&ulCurrentMinutes,
		&ulCurrentSeconds,
		&ulCurrentMilliSeconds) != 7)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SSCANF_FAILED);

		return err;
	}

	ulSecondsToAdd		= ((ulCurrentMilliSeconds + ulMilliSeconds) / 1000);

	ulNextMilliSeconds	=
		(ulCurrentMilliSeconds + ulMilliSeconds) -
		(ulSecondsToAdd * 1000);

	if (DateTime:: addSeconds (
		ulCurrentYear,
		ulCurrentMonth,
		ulCurrentDay,
		ulCurrentHour,
		ulCurrentMinutes,
		ulCurrentSeconds,
		lCurrentDaylightSavingTime,
		(long long) ulSecondsToAdd,
		&ulNextYear,
		&ulNextMonth,
		&ulNextDay,
		&ulNextHour,
		&ulNextMinutes,
		&ulNextSeconds,
		pbNextDaylightSavingTime) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_ADDSECONDS_FAILED);

		return err;
	}

	sprintf (pNextDateTime, "%04lu-%02lu-%02lu %02lu:%02lu:%02lu:%04lu",
		ulNextYear,
		ulNextMonth,
		ulNextDay,
		ulNextHour,
		ulNextMinutes,
		ulNextSeconds,
		ulNextMilliSeconds);


	return errNoError;

/*
	tm					tmDateTime;
	time_t				lUtcTime;
	unsigned long		ulSrcMilliSeconds;


	if (pDestDateTime == (char *) NULL ||
		pSrcDateTime == (char *) NULL)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_ACTIVATION_WRONG);

		return err;
	}

	if (sscanf (pSrcDateTime,
		"%4d-%2d-%2d %2d:%2d:%2d:%4d",
		&(tmDateTime. tm_year),
		&(tmDateTime. tm_mon),
		&(tmDateTime. tm_mday),
		&(tmDateTime. tm_hour),
		&(tmDateTime. tm_min),
		&(tmDateTime. tm_sec),
		&ulSrcMilliSeconds) != 7)
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_SSCANF_FAILED);

		return err;
	}

	tmDateTime. tm_year		-= 1900;
	tmDateTime. tm_mon		-= 1;

	// A negative value for tm_isdst causes mktime() to attempt to determine
	// whether Daylight Saving Time is in effect for the specified time.
	tmDateTime. tm_isdst	= -1;

	lUtcTime = mktime (&tmDateTime);

	lUtcTime	+= ((ulSrcMilliSeconds + ulMilliSeconds) / 1000);

	#if defined(__hpux) && defined(_CMA__HP)
		if (localtime_r (&lUtcTime, &tmDateTime))
	#else
		if (localtime_r (&lUtcTime, &tmDateTime) == (struct tm *) NULL)
	#endif
	{
		Error err = SchedulerErrors (__FILE__, __LINE__,
			SCH_LOCALTIME_R_FAILED);

		return err;
	}

	sprintf (pDestDateTime, "%04d-%02d-%02d %02d:%02d:%02d:%04d",
		tmDateTime. tm_year + 1900,
		tmDateTime. tm_mon + 1,
		tmDateTime. tm_mday,
		tmDateTime. tm_hour,
		tmDateTime. tm_min,
		tmDateTime. tm_sec,
		(ulSrcMilliSeconds + ulMilliSeconds) -
		(((ulSrcMilliSeconds + ulMilliSeconds) / 1000) * 1000));

	return errNoError;
*/

}


Error Times:: updateNextCalendarExpirationDateTime (void)

{

	Boolean_t			bIsDateTimeValid;
	tm					tmNextExpirationDateTime;
	char				pSchedule [SCH_MAXSCHEDULELENGTH];
	char				*pScheduleToken;
	char				*pScheduleTokenLast;
	unsigned long		ulMilliSeconds;


	// initialize the tm struct (the tm_wday field too)
	{
		time_t				lUtcTime;
		unsigned long		ulLocalYear;
		unsigned long		ulLocalMonth;
		unsigned long		ulLocalDay;
		unsigned long		ulLocalHour;
		unsigned long		ulLocalMinutes;
		unsigned long		ulLocalSeconds;



		if (sscanf (_pCurrentExpirationDateTime,
			"%4lu-%2lu-%2lu %2lu:%2lu:%2lu:%4lu",
			&ulLocalYear,
			&ulLocalMonth,
			&ulLocalDay,
			&ulLocalHour,
			&ulLocalMinutes,
			&ulLocalSeconds,
			&ulMilliSeconds) != 7)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_SSCANF_FAILED);

			return err;
		}

		tmNextExpirationDateTime. tm_year		= ulLocalYear;
		tmNextExpirationDateTime. tm_mon		= ulLocalMonth;
		tmNextExpirationDateTime. tm_mday		= ulLocalDay;
		tmNextExpirationDateTime. tm_hour		= ulLocalHour;
		tmNextExpirationDateTime. tm_min		= ulLocalMinutes;
		tmNextExpirationDateTime. tm_sec		= ulLocalSeconds;

		tmNextExpirationDateTime. tm_year		-= 1900;
		tmNextExpirationDateTime. tm_mon		-= 1;

		//	A negative value for tm_isdst causes mktime() to attempt
		//	to determine whether Daylight Saving Time is in effect
		//	for the specified time.
		tmNextExpirationDateTime. tm_isdst	= _bCurrentDaylightSavingTime;

		lUtcTime = mktime (&tmNextExpirationDateTime);

		#if defined(__hpux) && defined(_CMA__HP)
			if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
		#else
			if (localtime_r (&lUtcTime, &tmNextExpirationDateTime) ==
				(struct tm *) NULL)
		#endif
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_LOCALTIME_R_FAILED);

			return err;
		}
	}

	// add 1 sec
	{
		time_t				lUtcTime;

		lUtcTime		= mktime (&tmNextExpirationDateTime);

		lUtcTime		+= 1;

		#if defined(__hpux) && defined(_CMA__HP)
			if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
		#else
			if (localtime_r (&lUtcTime, &tmNextExpirationDateTime) ==
				(struct tm *) NULL)
		#endif
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_LOCALTIME_R_FAILED);

			return err;
		}
	}

	bIsDateTimeValid		= false;

	while (!bIsDateTimeValid)
	{
		strcpy (pSchedule, _pCalendarSchedule);

		// year
		if ((pScheduleToken = strtok_r (pSchedule, " ", &pScheduleTokenLast)) ==
			(char *) NULL)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_CALENDARSCHEDULE_WRONG);

			return err;
		}

		if (strchr (pScheduleToken, ',') != (char *) NULL)
		{
			Boolean_t		bIsYearValid;
			long			lMaxYear;
			long			lValue;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			char			*pPointerToScheduleToken;
			long			lNumberLength;


			pPointerToScheduleToken			= pScheduleToken;

			lMaxYear			= -1;
			bIsYearValid		= false;
			do
			{
				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					lNumberLength		=
						strlen (pPointerToScheduleToken);
				else
					lNumberLength		=
						strchr (pPointerToScheduleToken, ',') -
						pPointerToScheduleToken;

				strncpy (pNumber, pPointerToScheduleToken,
					lNumberLength);

				pNumber [lNumberLength]				= '\0';

				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					pPointerToScheduleToken				+=
						lNumberLength;
				else
					pPointerToScheduleToken				+=
						(lNumberLength + 1);

				lValue			= atol (pNumber);

				if (lValue > lMaxYear)
					lMaxYear	= lValue;

				if (lValue == tmNextExpirationDateTime. tm_year + 1900)
					bIsYearValid	= true;
				else
				{
					if (*pPointerToScheduleToken == '\0')
						break;
				}
			}
			while (!bIsYearValid);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			Boolean_t		bIsYearValid;
			long			lValue;
			long			lMaxYear;


			// comma separator
			if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lMaxYear			= -1;
			bIsYearValid		= false;
			do
			{
				lValue			= atol (pFieldToken);

				if (lValue > lMaxYear)
					lMaxYear	= lValue;

				if (lValue == tmNextExpirationDateTime. tm_year + 1900)
					bIsYearValid	= true;
				else
				{
					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
			}
			while (!bIsYearValid);
			*/

			if (!bIsYearValid)
			{
				if (lMaxYear < tmNextExpirationDateTime. tm_year + 1900)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_REACHED_LASTTIMEOUT);

					return err;
				}

				//	initialize tmNextExpirationDateTime to the first day
				//	of the next year
				{
					time_t				lUtcTime;

					tmNextExpirationDateTime. tm_year		+= 1;
					tmNextExpirationDateTime. tm_mon		= 0;
					tmNextExpirationDateTime. tm_mday		= 1;
					tmNextExpirationDateTime. tm_hour		= 0;
					tmNextExpirationDateTime. tm_min		= 0;
					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}
				}

				continue;
			}
		}
		else if (strchr (pScheduleToken, '-') != (char *) NULL)
		{
			// values range
			long			lValueFrom;
			long			lValueTo;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			long			lNumberLength;


			lNumberLength		= strchr (pScheduleToken, '-') -
				pScheduleToken;

			strncpy (pNumber, pScheduleToken, lNumberLength);

			pNumber [lNumberLength]				= '\0';

			lValueFrom			= atol (pNumber);

			strcpy (pNumber, pScheduleToken + lNumberLength + 1);

			lValueTo			= atol (pNumber);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			// dash separator
			if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueFrom		 = atol (pFieldToken);

			if ((pFieldToken = strtok_r ((char *) NULL, "-",
				&pFieldTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueTo		 = atol (pFieldToken);
			*/

			if (!(lValueFrom <= tmNextExpirationDateTime. tm_year + 1900 &&
				tmNextExpirationDateTime. tm_year + 1900 <= lValueTo))
			{
				if (lValueTo < tmNextExpirationDateTime. tm_year + 1900)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_REACHED_LASTTIMEOUT);

					return err;
				}

				//	initialize tmNextExpirationDateTime to the first day
				//	of the next year
				{
					time_t				lUtcTime;

					tmNextExpirationDateTime. tm_year		+= 1;
					tmNextExpirationDateTime. tm_mon		= 0;
					tmNextExpirationDateTime. tm_mday		= 1;
					tmNextExpirationDateTime. tm_hour		= 0;
					tmNextExpirationDateTime. tm_min		= 0;
					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}
				}

				continue;
			}
		}
		else
		{	// single value or '*'

			if (pScheduleToken [0] != '*')
			{
				long		lYear;


				lYear			= atol (pScheduleToken);

				if (lYear != tmNextExpirationDateTime. tm_year + 1900)
				{
					if (lYear < tmNextExpirationDateTime. tm_year + 1900)
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_REACHED_LASTTIMEOUT);

						return err;
					}

					//	initialize tmNextExpirationDateTime to the first day
					//	of the next year
					{
						time_t				lUtcTime;

						tmNextExpirationDateTime. tm_year		+= 1;
						tmNextExpirationDateTime. tm_mon		= 0;
						tmNextExpirationDateTime. tm_mday		= 1;
						tmNextExpirationDateTime. tm_hour		= 0;
						tmNextExpirationDateTime. tm_min		= 0;
						tmNextExpirationDateTime. tm_sec		= 0;

						//	A negative value for tm_isdst causes mktime()
						//	to attempt to determine whether
						//	Daylight Saving Time is in effect for the specified
						//	time.
						tmNextExpirationDateTime. tm_isdst	= -1;

						lUtcTime		= mktime (&tmNextExpirationDateTime);

						#if defined(__hpux) && defined(_CMA__HP)
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime))
						#else
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime) ==
								(struct tm *) NULL)
						#endif
						{
							Error err = SchedulerErrors (__FILE__, __LINE__,
								SCH_LOCALTIME_R_FAILED);

							return err;
						}
					}

					continue;
				}
			}
		}

		// month
		if ((pScheduleToken = strtok_r ((char *) NULL, " ",
			&pScheduleTokenLast)) == (char *) NULL)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_CALENDARSCHEDULE_WRONG);

			return err;
		}

		if (strchr (pScheduleToken, ',') != (char *) NULL)
		{
			Boolean_t		bIsMonthValid;
			long			lValue;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			char			*pPointerToScheduleToken;
			long			lNumberLength;


			pPointerToScheduleToken			= pScheduleToken;

			bIsMonthValid		= false;
			do
			{
				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					lNumberLength		=
						strlen (pPointerToScheduleToken);
				else
					lNumberLength		=
						strchr (pPointerToScheduleToken, ',') -
						pPointerToScheduleToken;

				strncpy (pNumber, pPointerToScheduleToken,
					lNumberLength);

				pNumber [lNumberLength]				= '\0';

				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					pPointerToScheduleToken				+=
						lNumberLength;
				else
					pPointerToScheduleToken				+=
						(lNumberLength + 1);

				lValue			= atol (pNumber);

				if (lValue == tmNextExpirationDateTime. tm_mon + 1)
					bIsMonthValid	= true;
				else
				{
					if (*pPointerToScheduleToken == '\0')
						break;
				}
			}
			while (!bIsMonthValid);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			Boolean_t		bIsMonthValid;
			long			lValue;


			// comma separator
			if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			bIsMonthValid		= false;
			do
			{
				lValue			= atol (pFieldToken);

				if (lValue == tmNextExpirationDateTime. tm_mon + 1)
					bIsMonthValid	= true;
				else
				{
					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
			}
			while (!bIsMonthValid);
			*/

			if (!bIsMonthValid)
			{
				//	initialize tmNextExpirationDateTime to the first minute
				//	of the next month
				{
					time_t				lUtcTime;

					if (tmNextExpirationDateTime. tm_mon == 11)
					{
						tmNextExpirationDateTime. tm_mon		= 0;
						tmNextExpirationDateTime. tm_year		+= 1;
					}
					else
						tmNextExpirationDateTime. tm_mon		+= 1;
					tmNextExpirationDateTime. tm_mday		= 1;
					tmNextExpirationDateTime. tm_hour		= 0;
					tmNextExpirationDateTime. tm_min		= 0;
					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}
				}

				continue;
			}
		}
		else if (strchr (pScheduleToken, '-') != (char *) NULL)
		{
			// values range
			long			lValueFrom;
			long			lValueTo;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			long			lNumberLength;


			lNumberLength		= strchr (pScheduleToken, '-') -
				pScheduleToken;

			strncpy (pNumber, pScheduleToken, lNumberLength);

			pNumber [lNumberLength]				= '\0';

			lValueFrom			= atol (pNumber);

			strcpy (pNumber, pScheduleToken + lNumberLength + 1);

			lValueTo			= atol (pNumber);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			// dash separator
			if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueFrom		 = atol (pFieldToken);

			if ((pFieldToken = strtok_r ((char *) NULL, "-",
				&pFieldTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueTo		 = atol (pFieldToken);
			*/

			if (!(lValueFrom <= tmNextExpirationDateTime. tm_mon + 1 &&
				tmNextExpirationDateTime. tm_mon + 1 <= lValueTo))
			{
				//	initialize tmNextExpirationDateTime to the first minute
				//	of the next month
				{
					time_t				lUtcTime;

					if (tmNextExpirationDateTime. tm_mon == 11)
					{
						tmNextExpirationDateTime. tm_mon		= 0;
						tmNextExpirationDateTime. tm_year		+= 1;
					}
					else
						tmNextExpirationDateTime. tm_mon		+= 1;
					tmNextExpirationDateTime. tm_mday		= 1;
					tmNextExpirationDateTime. tm_hour		= 0;
					tmNextExpirationDateTime. tm_min		= 0;
					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime = mktime (&tmNextExpirationDateTime);

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}
				}

				continue;
			}
		}
		else
		{	// single value or '*'

			if (pScheduleToken [0] != '*')
			{
				long			lMonth;

				
				lMonth			= atol (pScheduleToken);
				
				if (lMonth != tmNextExpirationDateTime. tm_mon + 1)
				{
					//	initialize tmNextExpirationDateTime to the first minute
					//	of the next month
					{
						time_t				lUtcTime;

						if (tmNextExpirationDateTime. tm_mon == 11)
						{
							tmNextExpirationDateTime. tm_mon		= 0;
							tmNextExpirationDateTime. tm_year		+= 1;
						}
						else
							tmNextExpirationDateTime. tm_mon		+= 1;
						tmNextExpirationDateTime. tm_mday		= 1;
						tmNextExpirationDateTime. tm_hour		= 0;
						tmNextExpirationDateTime. tm_min		= 0;
						tmNextExpirationDateTime. tm_sec		= 0;

						//	A negative value for tm_isdst causes mktime()
						//	to attempt to determine whether
						//	Daylight Saving Time is in effect
						//	for the specified time.
						tmNextExpirationDateTime. tm_isdst	= -1;

						lUtcTime = mktime (&tmNextExpirationDateTime);

						#if defined(__hpux) && defined(_CMA__HP)
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime))
						#else
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime) ==
								(struct tm *) NULL)
						#endif
						{
							Error err = SchedulerErrors (__FILE__, __LINE__,
								SCH_LOCALTIME_R_FAILED);

							return err;
						}
					}

					continue;
				}
			}
		}

		// monthday and weekday

		{
			Boolean_t		bIsNecessaryAddDayForMonthDay;
			Boolean_t		bIsNecessaryAddDayForWeekDay;
			Boolean_t		bIsMonthDayStar;
			Boolean_t		bIsWeekDayStar;
			Boolean_t		bIsNecessaryAddDay;


			bIsNecessaryAddDayForMonthDay		= false;
			bIsNecessaryAddDayForWeekDay		= false;

			// monthday
			if ((pScheduleToken = strtok_r ((char *) NULL, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			if (strchr (pScheduleToken, ',') != (char *) NULL)
			{
				Boolean_t		bIsMonthDayValid;
				long			lValue;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				char			*pPointerToScheduleToken;
				long			lNumberLength;


				pPointerToScheduleToken			= pScheduleToken;

				bIsMonthDayStar			= false;

				bIsMonthDayValid		= false;
				do
				{
					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						lNumberLength		=
							strlen (pPointerToScheduleToken);
					else
						lNumberLength		=
							strchr (pPointerToScheduleToken, ',') -
							pPointerToScheduleToken;

					strncpy (pNumber, pPointerToScheduleToken,
						lNumberLength);

					pNumber [lNumberLength]				= '\0';

					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						pPointerToScheduleToken				+=
							lNumberLength;
					else
						pPointerToScheduleToken				+=
							(lNumberLength + 1);

					lValue			= atol (pNumber);

					if (lValue == tmNextExpirationDateTime. tm_mday)
						bIsMonthDayValid	= true;
					else
					{
						if (*pPointerToScheduleToken == '\0')
							break;
					}
				}
				while (!bIsMonthDayValid);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				Boolean_t		bIsMonthDayValid;
				long			lValue;


				bIsMonthDayStar			= false;

				// comma separator
				if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				bIsMonthDayValid		= false;
				do
				{
					lValue			= atol (pFieldToken);

					if (lValue == tmNextExpirationDateTime. tm_mday)
						bIsMonthDayValid	= true;
					else
					{
						if ((pFieldToken = strtok_r ((char *) NULL, ",",
							&pFieldTokenLast)) == (char *) NULL)
							break;
					}
				}
				while (!bIsMonthDayValid);
				*/

				if (!bIsMonthDayValid)
				{
					bIsNecessaryAddDayForMonthDay		= true;
				}
			}
			else if (strchr (pScheduleToken, '-') != (char *) NULL)
			{
				// values range
				long			lValueFrom;
				long			lValueTo;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				long			lNumberLength;


				bIsMonthDayStar			= false;

				lNumberLength		= strchr (pScheduleToken, '-') -
					pScheduleToken;

				strncpy (pNumber, pScheduleToken, lNumberLength);

				pNumber [lNumberLength]				= '\0';

				lValueFrom			= atol (pNumber);

				strcpy (pNumber, pScheduleToken + lNumberLength + 1);

				lValueTo			= atol (pNumber);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				bIsMonthDayStar			= false;

				// dash separator
				if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueFrom		 = atol (pFieldToken);

				if ((pFieldToken = strtok_r ((char *) NULL, "-",
					&pFieldTokenLast)) == (char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueTo		 = atol (pFieldToken);
				*/

				if (!(lValueFrom <= tmNextExpirationDateTime. tm_mday &&
					tmNextExpirationDateTime. tm_mday <= lValueTo))
				{
					bIsNecessaryAddDayForMonthDay		= true;
				}
			}
			else
			{	// single value or '*'

				if (pScheduleToken [0] != '*')
				{
					long			lMonthDay;

				
					bIsMonthDayStar			= false;

					lMonthDay		= atol (pScheduleToken);
				
					if (lMonthDay != tmNextExpirationDateTime. tm_mday)
					{
						bIsNecessaryAddDayForMonthDay		= true;
					}
				}
				else
					bIsMonthDayStar			= true;
			}

			// weekday
			if ((pScheduleToken = strtok_r ((char *) NULL, " ",
				&pScheduleTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			if (strchr (pScheduleToken, ',') != (char *) NULL)
			{
				Boolean_t		bIsWeekDayValid;
				long			lValue;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				char			*pPointerToScheduleToken;
				long			lNumberLength;


				pPointerToScheduleToken			= pScheduleToken;

				bIsWeekDayStar			= false;

				bIsWeekDayValid			= false;
				do
				{
					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						lNumberLength		=
							strlen (pPointerToScheduleToken);
					else
						lNumberLength		=
							strchr (pPointerToScheduleToken, ',') -
							pPointerToScheduleToken;

					strncpy (pNumber, pPointerToScheduleToken,
						lNumberLength);

					pNumber [lNumberLength]				= '\0';

					if (strchr (pPointerToScheduleToken, ',') ==
						(char *) NULL)
						pPointerToScheduleToken				+=
							lNumberLength;
					else
						pPointerToScheduleToken				+=
							(lNumberLength + 1);

					lValue			= atol (pNumber);

					if (lValue == tmNextExpirationDateTime. tm_wday)
						bIsWeekDayValid	= true;
					else
					{
						if (*pPointerToScheduleToken == '\0')
							break;
					}
				}
				while (!bIsWeekDayValid);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				Boolean_t		bIsWeekDayValid;
				long			lValue;


				bIsWeekDayStar			= false;

				// comma separator
				if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				bIsWeekDayValid		= false;
				do
				{
					lValue			= atol (pFieldToken);

					if (lValue == tmNextExpirationDateTime. tm_wday)
						bIsWeekDayValid	= true;
					else
					{
						if ((pFieldToken = strtok_r ((char *) NULL, ",",
							&pFieldTokenLast)) == (char *) NULL)
							break;
					}
				}
				while (!bIsWeekDayValid);
				*/

				if (!bIsWeekDayValid)
				{
					bIsNecessaryAddDayForWeekDay		= true;
				}
			}
			else if (strchr (pScheduleToken, '-') != (char *) NULL)
			{
				// values range
				long			lValueFrom;
				long			lValueTo;
				char			pNumber [SCH_MAXSCHEDULELENGTH];
				long			lNumberLength;


				bIsWeekDayStar			= false;

				lNumberLength		= strchr (pScheduleToken, '-') -
					pScheduleToken;

				strncpy (pNumber, pScheduleToken, lNumberLength);

				pNumber [lNumberLength]				= '\0';

				lValueFrom			= atol (pNumber);

				strcpy (pNumber, pScheduleToken + lNumberLength + 1);

				lValueTo			= atol (pNumber);

				/* PRIMA (strtok_r annidate non funzionano con windows)
				bIsWeekDayStar			= false;

				// dash separator
				if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
					(char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueFrom		 = atol (pFieldToken);

				if ((pFieldToken = strtok_r ((char *) NULL, "-",
					&pFieldTokenLast)) == (char *) NULL)
				{
					Error err = SchedulerErrors (__FILE__, __LINE__,
						SCH_CALENDARSCHEDULE_WRONG);

					return err;
				}

				lValueTo		 = atol (pFieldToken);
				*/

				if (!(lValueFrom <= tmNextExpirationDateTime. tm_wday &&
					tmNextExpirationDateTime. tm_wday <= lValueTo))
				{
					bIsNecessaryAddDayForWeekDay		= true;
				}
			}
			else
			{	// single value or '*'

				if (pScheduleToken [0] != '*')
				{
					long			lWeekDay;


					bIsWeekDayStar			= false;

					lWeekDay		= atol (pScheduleToken);

					if (lWeekDay != tmNextExpirationDateTime. tm_wday)
					{
						bIsNecessaryAddDayForWeekDay		= true;
					}
				}
				else
					bIsWeekDayStar			= true;
			}

			if (bIsMonthDayStar && bIsWeekDayStar)
				bIsNecessaryAddDay		= false;
			else if (!bIsMonthDayStar && bIsWeekDayStar)
			{
				if (bIsNecessaryAddDayForMonthDay)
					bIsNecessaryAddDay		= true;
				else
					bIsNecessaryAddDay		= false;
			}
			else if (bIsMonthDayStar && !bIsWeekDayStar)
			{
				if (bIsNecessaryAddDayForWeekDay)
					bIsNecessaryAddDay		= true;
				else
					bIsNecessaryAddDay		= false;
			}
			else
			{		// !bIsMonthDayStar && !bIsWeekDayStar
				if (bIsNecessaryAddDayForMonthDay &&
					bIsNecessaryAddDayForWeekDay)
					bIsNecessaryAddDay		= true;
				else
					bIsNecessaryAddDay		= false;
			}

			if (bIsNecessaryAddDay)
			{
				// add 1 day
				{
					time_t				lUtcTime;

					tmNextExpirationDateTime. tm_hour		= 0;
					tmNextExpirationDateTime. tm_min		= 0;
					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					lUtcTime		+= (60 * 60 * 24);

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}

					// gestione ora legale
					switch (tmNextExpirationDateTime. tm_hour)
					{
						case 0:
							// OK

							break;
						case 23:
							//	gestione ora legale: 3 -> 2.
							//	Anche se si sommano 24 ore,
							//	in pratica se ne sommano 23
							lUtcTime		+= (60 * 60);

							#if defined(__hpux) && defined(_CMA__HP)
								if (localtime_r (&lUtcTime,
									&tmNextExpirationDateTime))
							#else
								if (localtime_r (&lUtcTime,
									&tmNextExpirationDateTime) ==
									(struct tm *) NULL)
							#endif
							{
								Error err = SchedulerErrors (__FILE__, __LINE__,
									SCH_LOCALTIME_R_FAILED);

								return err;
							}

							break;
						case 1:
							//	gestione ora legale: 2 -> 3.
							//	Anche se si sommano 24 ore,
							//	in pratica se ne sommano 25

							lUtcTime		-= (60 * 60);

							#if defined(__hpux) && defined(_CMA__HP)
								if (localtime_r (&lUtcTime,
									&tmNextExpirationDateTime))
							#else
								if (localtime_r (&lUtcTime,
									&tmNextExpirationDateTime) ==
									(struct tm *) NULL)
							#endif
							{
								Error err = SchedulerErrors (__FILE__, __LINE__,
									SCH_LOCALTIME_R_FAILED);

								return err;
							}

							break;
						default:
								Error err = SchedulerErrors (__FILE__, __LINE__,
									SCH_LOCALTIME_R_FAILED);

								return err;
					}
				}

				continue;
			}
		}

		// hour
		if ((pScheduleToken = strtok_r ((char *) NULL, " ",
			&pScheduleTokenLast)) == (char *) NULL)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_CALENDARSCHEDULE_WRONG);

			return err;
		}

		if (strchr (pScheduleToken, ',') != (char *) NULL)
		{
			Boolean_t		bIsHourValid;
			long			lValue;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			char			*pPointerToScheduleToken;
			long			lNumberLength;


			pPointerToScheduleToken			= pScheduleToken;

			bIsHourValid		= false;
			do
			{
				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					lNumberLength		=
						strlen (pPointerToScheduleToken);
				else
					lNumberLength		=
						strchr (pPointerToScheduleToken, ',') -
						pPointerToScheduleToken;

				strncpy (pNumber, pPointerToScheduleToken,
					lNumberLength);

				pNumber [lNumberLength]				= '\0';

				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					pPointerToScheduleToken				+=
						lNumberLength;
				else
					pPointerToScheduleToken				+=
						(lNumberLength + 1);

				lValue			= atol (pNumber);

				if (lValue == tmNextExpirationDateTime. tm_hour)
					bIsHourValid	= true;
				else
				{
					if (*pPointerToScheduleToken == '\0')
						break;
				}
			}
			while (!bIsHourValid);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			Boolean_t		bIsHourValid;
			long			lValue;


			// comma separator
			if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			bIsHourValid		= false;
			do
			{
				lValue			= atol (pFieldToken);

				if (lValue == tmNextExpirationDateTime. tm_hour)
					bIsHourValid	= true;
				else
				{
					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
			}
			while (!bIsHourValid);
			*/

			if (!bIsHourValid)
			{
				// add 1 hour
				{
					time_t				lUtcTime;
					unsigned long		ulInitialHour;

					tmNextExpirationDateTime. tm_min		= 0;
					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					ulInitialHour	= tmNextExpirationDateTime. tm_hour;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					lUtcTime		+= (60 * 60);

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}

					//	gestione ora legale: 3 -> 2
					if (ulInitialHour == tmNextExpirationDateTime. tm_hour)
					{
						//	gestione ora legale: 3 -> 2
						//	se alle 2 si somma 1 ora si ottiene ancora 2
						//	quindi dobbiamo sommare un'altra ora

						lUtcTime		+= (60 * 60);

						#if defined(__hpux) && defined(_CMA__HP)
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime))
						#else
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime) ==
								(struct tm *) NULL)
						#endif
						{
							Error err = SchedulerErrors (__FILE__, __LINE__,
								SCH_LOCALTIME_R_FAILED);

							return err;
						}
					}
				}

				continue;
			}
		}
		else if (strchr (pScheduleToken, '-') != (char *) NULL)
		{
			// values range
			long			lValueFrom;
			long			lValueTo;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			long			lNumberLength;


			lNumberLength		= strchr (pScheduleToken, '-') -
				pScheduleToken;

			strncpy (pNumber, pScheduleToken, lNumberLength);

			pNumber [lNumberLength]				= '\0';

			lValueFrom			= atol (pNumber);

			strcpy (pNumber, pScheduleToken + lNumberLength + 1);

			lValueTo			= atol (pNumber);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			// dash separator
			if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueFrom		 = atol (pFieldToken);

			if ((pFieldToken = strtok_r ((char *) NULL, "-",
				&pFieldTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueTo		 = atol (pFieldToken);
			*/

			if (!(lValueFrom <= tmNextExpirationDateTime. tm_hour &&
				tmNextExpirationDateTime. tm_hour <= lValueTo))
			{
				// add 1 hour
				{
					time_t				lUtcTime;
					unsigned long		ulInitialHour;

					tmNextExpirationDateTime. tm_min		= 0;
					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					ulInitialHour	= tmNextExpirationDateTime. tm_hour;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					lUtcTime		+= (60 * 60);

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}

					//	gestione ora legale: 3 -> 2.
					//	se alle 2 si somma 1 ora si ottiene ancora 2
					//	quindi dobbiamo sommare un'altra ora
					if (ulInitialHour == tmNextExpirationDateTime. tm_hour)
					{
						lUtcTime		+= (60 * 60);

						#if defined(__hpux) && defined(_CMA__HP)
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime))
						#else
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime) ==
								(struct tm *) NULL)
						#endif
						{
							Error err = SchedulerErrors (__FILE__, __LINE__,
								SCH_LOCALTIME_R_FAILED);

							return err;
						}
					}
				}

				continue;
			}
		}
		else
		{	// single value or '*'

			if (pScheduleToken [0] != '*')
			{
				long			lHour;
				
				lHour		= atol (pScheduleToken);
				
				if (lHour != tmNextExpirationDateTime. tm_hour)
				{
					// add 1 hour
					{
						time_t				lUtcTime;
						unsigned long		ulInitialHour;

						tmNextExpirationDateTime. tm_min		= 0;
						tmNextExpirationDateTime. tm_sec		= 0;

						//	A negative value for tm_isdst causes mktime()
						//	to attempt to determine whether
						//	Daylight Saving Time is in effect
						//	for the specified time.
						tmNextExpirationDateTime. tm_isdst	= -1;

						ulInitialHour	= tmNextExpirationDateTime. tm_hour;

						lUtcTime		= mktime (&tmNextExpirationDateTime);

						lUtcTime		+= (60 * 60);

						#if defined(__hpux) && defined(_CMA__HP)
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime))
						#else
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime) ==
								(struct tm *) NULL)
						#endif
						{
							Error err = SchedulerErrors (__FILE__, __LINE__,
								SCH_LOCALTIME_R_FAILED);

							return err;
						}

						//	gestione ora legale: 3 -> 2 and 2 -> 4.
						//	se alle 2 si somma 1 ora si ottiene ancora 2
						//	quindi dobbiamo sommare un'altra ora
						if (ulInitialHour == tmNextExpirationDateTime. tm_hour)
						{
							lUtcTime		+= (60 * 60);

							#if defined(__hpux) && defined(_CMA__HP)
								if (localtime_r (&lUtcTime,
									&tmNextExpirationDateTime))
							#else
								if (localtime_r (&lUtcTime,
									&tmNextExpirationDateTime) ==
									(struct tm *) NULL)
							#endif
							{
								Error err = SchedulerErrors (__FILE__, __LINE__,
									SCH_LOCALTIME_R_FAILED);

								return err;
							}
						}
					}

					continue;
				}
			}
		}

		// minute
		if ((pScheduleToken = strtok_r ((char *) NULL, " ",
			&pScheduleTokenLast)) == (char *) NULL)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_CALENDARSCHEDULE_WRONG);

			return err;
		}

		if (strchr (pScheduleToken, ',') != (char *) NULL)
		{
			Boolean_t		bIsMinuteValid;
			long			lValue;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			char			*pPointerToScheduleToken;
			long			lNumberLength;


			pPointerToScheduleToken			= pScheduleToken;

			bIsMinuteValid		= false;
			do
			{
				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					lNumberLength		=
						strlen (pPointerToScheduleToken);
				else
					lNumberLength		=
						strchr (pPointerToScheduleToken, ',') -
						pPointerToScheduleToken;

				strncpy (pNumber, pPointerToScheduleToken,
					lNumberLength);

				pNumber [lNumberLength]				= '\0';

				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					pPointerToScheduleToken				+=
						lNumberLength;
				else
					pPointerToScheduleToken				+=
						(lNumberLength + 1);

				lValue			= atol (pNumber);

				if (lValue == tmNextExpirationDateTime. tm_min)
					bIsMinuteValid		= true;
				else
				{
					if (*pPointerToScheduleToken == '\0')
						break;
				}
			}
			while (!bIsMinuteValid);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			Boolean_t		bIsMinuteValid;
			long			lValue;


			// comma separator
			if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			bIsMinuteValid		= false;
			do
			{
				lValue			= atol (pFieldToken);

				if (lValue == tmNextExpirationDateTime. tm_min)
					bIsMinuteValid		= true;
				else
				{
					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
			}
			while (!bIsMinuteValid);
			*/

			if (!bIsMinuteValid)
			{
				// add 1 min
				{
					time_t				lUtcTime;

					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					lUtcTime		+= 60;

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}
				}

				continue;
			}
		}
		else if (strchr (pScheduleToken, '-') != (char *) NULL)
		{
			// values range
			long			lValueFrom;
			long			lValueTo;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			long			lNumberLength;


			lNumberLength		= strchr (pScheduleToken, '-') -
				pScheduleToken;

			strncpy (pNumber, pScheduleToken, lNumberLength);

			pNumber [lNumberLength]				= '\0';

			lValueFrom			= atol (pNumber);

			strcpy (pNumber, pScheduleToken + lNumberLength + 1);

			lValueTo			= atol (pNumber);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			// dash separator
			if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueFrom		 = atol (pFieldToken);

			if ((pFieldToken = strtok_r ((char *) NULL, "-",
				&pFieldTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueTo		 = atol (pFieldToken);
			*/

			if (!(lValueFrom <= tmNextExpirationDateTime. tm_min &&
				tmNextExpirationDateTime. tm_min <= lValueTo))
			{
				// add 1 minute
				{
					time_t				lUtcTime;

					tmNextExpirationDateTime. tm_sec		= 0;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					lUtcTime		+= 60;

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}
				}

				continue;
			}
		}
		else
		{	// single value or '*'

			if (pScheduleToken [0] != '*')
			{
				long			lMinute;
				
				
				lMinute			= atol (pScheduleToken);
				
				if (lMinute != tmNextExpirationDateTime. tm_min)
				{
					// add 1 minute
					{
						time_t				lUtcTime;

						tmNextExpirationDateTime. tm_sec		= 0;

						//	A negative value for tm_isdst causes mktime()
						//	to attempt to determine whether
						//	Daylight Saving Time is in effect
						//	for the specified time.
						tmNextExpirationDateTime. tm_isdst	= -1;

						lUtcTime		= mktime (&tmNextExpirationDateTime);

						lUtcTime		+= 60;

						#if defined(__hpux) && defined(_CMA__HP)
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime))
						#else
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime) ==
								(struct tm *) NULL)
						#endif
						{
							Error err = SchedulerErrors (__FILE__, __LINE__,
								SCH_LOCALTIME_R_FAILED);

							return err;
						}
					}

					continue;
				}
			}
		}

		// second
		if ((pScheduleToken = strtok_r ((char *) NULL, " ",
			&pScheduleTokenLast)) == (char *) NULL)
		{
			Error err = SchedulerErrors (__FILE__, __LINE__,
				SCH_CALENDARSCHEDULE_WRONG);

			return err;
		}

		if (strchr (pScheduleToken, ',') != (char *) NULL)
		{
			Boolean_t		bIsSecondValid;
			long			lValue;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			char			*pPointerToScheduleToken;
			long			lNumberLength;


			pPointerToScheduleToken			= pScheduleToken;

			bIsSecondValid		= false;
			do
			{
				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					lNumberLength		=
						strlen (pPointerToScheduleToken);
				else
					lNumberLength		=
						strchr (pPointerToScheduleToken, ',') -
						pPointerToScheduleToken;

				strncpy (pNumber, pPointerToScheduleToken,
					lNumberLength);

				pNumber [lNumberLength]				= '\0';

				if (strchr (pPointerToScheduleToken, ',') ==
					(char *) NULL)
					pPointerToScheduleToken				+=
						lNumberLength;
				else
					pPointerToScheduleToken				+=
						(lNumberLength + 1);

				lValue			= atol (pNumber);

				if (lValue == tmNextExpirationDateTime. tm_sec)
					bIsSecondValid		= true;
				else
				{
					if (*pPointerToScheduleToken == '\0')
						break;
				}
			}
			while (!bIsSecondValid);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			Boolean_t		bIsSecondValid;
			long			lValue;


			// comma separator
			if ((pFieldToken = strtok_r (pField, ",", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			bIsSecondValid		= false;
			do
			{
				lValue			= atol (pFieldToken);

				if (lValue == tmNextExpirationDateTime. tm_sec)
					bIsSecondValid		= true;
				else
				{
					if ((pFieldToken = strtok_r ((char *) NULL, ",",
						&pFieldTokenLast)) == (char *) NULL)
						break;
				}
			}
			while (!bIsSecondValid);
			*/

			if (!bIsSecondValid)
			{
				// add 1 second
				{
					time_t				lUtcTime;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					lUtcTime		+= 1;

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}
				}

				continue;
			}
		}
		else if (strchr (pScheduleToken, '-') != (char *) NULL)
		{
			// values range
			long			lValueFrom;
			long			lValueTo;
			char			pNumber [SCH_MAXSCHEDULELENGTH];
			long			lNumberLength;


			lNumberLength		= strchr (pScheduleToken, '-') -
				pScheduleToken;

			strncpy (pNumber, pScheduleToken, lNumberLength);

			pNumber [lNumberLength]				= '\0';

			lValueFrom			= atol (pNumber);

			strcpy (pNumber, pScheduleToken + lNumberLength + 1);

			lValueTo			= atol (pNumber);

			/* PRIMA (strtok_r annidate non funzionano con windows)
			// dash separator
			if ((pFieldToken = strtok_r (pField, "-", &pFieldTokenLast)) ==
				(char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueFrom		 = atol (pFieldToken);

			if ((pFieldToken = strtok_r ((char *) NULL, "-",
				&pFieldTokenLast)) == (char *) NULL)
			{
				Error err = SchedulerErrors (__FILE__, __LINE__,
					SCH_CALENDARSCHEDULE_WRONG);

				return err;
			}

			lValueTo		 = atol (pFieldToken);
			*/

			if (!(lValueFrom <= tmNextExpirationDateTime. tm_sec &&
				tmNextExpirationDateTime. tm_sec <= lValueTo))
			{
				// add 1 second
				{
					time_t				lUtcTime;

					//	A negative value for tm_isdst causes mktime() to attempt
					//	to determine whether Daylight Saving Time is in effect
					//	for the specified time.
					tmNextExpirationDateTime. tm_isdst	= -1;

					lUtcTime		= mktime (&tmNextExpirationDateTime);

					lUtcTime		+= 1;

					#if defined(__hpux) && defined(_CMA__HP)
						if (localtime_r (&lUtcTime, &tmNextExpirationDateTime))
					#else
						if (localtime_r (&lUtcTime,
							&tmNextExpirationDateTime) == (struct tm *) NULL)
					#endif
					{
						Error err = SchedulerErrors (__FILE__, __LINE__,
							SCH_LOCALTIME_R_FAILED);

						return err;
					}
				}

				continue;
			}
		}
		else
		{	// single value or '*'

			if (pScheduleToken [0] != '*')
			{
				long			lSecond;


				lSecond		= atol (pScheduleToken);

				if (lSecond != tmNextExpirationDateTime. tm_sec)
				{
					// add 1 sec
					{
						time_t				lUtcTime;

						//	A negative value for tm_isdst causes mktime()
						//	to attempt to determine whether
						//	Daylight Saving Time is in effect
						//	for the specified time.
						tmNextExpirationDateTime. tm_isdst	= -1;

						lUtcTime		= mktime (&tmNextExpirationDateTime);

						lUtcTime		+= 1;

						#if defined(__hpux) && defined(_CMA__HP)
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime))
						#else
							if (localtime_r (&lUtcTime,
								&tmNextExpirationDateTime) ==
								(struct tm *) NULL)
						#endif
						{
							Error err = SchedulerErrors (__FILE__, __LINE__,
								SCH_LOCALTIME_R_FAILED);

							return err;
						}
					}

					continue;
				}
			}
		}

		bIsDateTimeValid		= true;
	}

	sprintf (_pNextExpirationDateTime,
		"%04lu-%02lu-%02lu %02lu:%02lu:%02lu:%04lu",
		(unsigned long) (tmNextExpirationDateTime. tm_year + 1900),
		(unsigned long) (tmNextExpirationDateTime. tm_mon + 1),
		(unsigned long) (tmNextExpirationDateTime. tm_mday),
		(unsigned long) (tmNextExpirationDateTime. tm_hour),
		(unsigned long) (tmNextExpirationDateTime. tm_min),
		(unsigned long) (tmNextExpirationDateTime. tm_sec),
		ulMilliSeconds);

	_bNextDaylightSavingTime		= tmNextExpirationDateTime. tm_isdst;


	return errNoError;
}

