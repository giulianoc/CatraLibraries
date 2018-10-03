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


#include <string>

#include "DateTime.h"
#ifdef WIN32
	#include <Windows.h>
#else
	#include <sys/time.h>
#endif
#ifdef _REENTRANT
	#include <pthread.h>	// for localtime_r
#endif
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <errno.h>


DateTime:: DateTime (void)

{

}


DateTime:: ~DateTime (void)

{

}



DateTime:: DateTime (const DateTime &)

{

	assert (1==0);

	// to do

}


DateTime &DateTime:: operator = (const DateTime &)

{

	assert (1==0);

	// to do

	return *this;

}


Error DateTime:: nowUTCInMilliSecs (
	unsigned long long *pullNowUTCInSecs,
	unsigned long *pulAdditionalMilliSecs,
	long *plTimeZoneDifferenceInHours)

{

	#ifdef WIN32
		SYSTEMTIME			stSystemTime;
	#else
		struct timeval		tvTimeval;
	#endif


	#ifdef WIN32
		GetLocalTime (&stSystemTime);

		(*pullNowUTCInSecs)					= time (NULL);
		(*pulAdditionalMilliSecs)			= stSystemTime. wMilliseconds;

		if (plTimeZoneDifferenceInHours != (long *) NULL)
		{
			if (DateTime:: getTimeZoneInformation (
				plTimeZoneDifferenceInHours) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_DATETIME_GETTIMEZONEINFORMATION_FAILED);

				return err;
			}
		}
	#else
		if (plTimeZoneDifferenceInHours != (long *) NULL)
		{
			struct timezone				tz;


			gettimeofday (&tvTimeval, &tz);

			*plTimeZoneDifferenceInHours	= ((tz. tz_minuteswest / 60) * -1);
		}
		else
		{
			gettimeofday (&tvTimeval, NULL);
		}

		(*pullNowUTCInSecs)					= tvTimeval. tv_sec;
		(*pulAdditionalMilliSecs)			= (tvTimeval. tv_usec / 1000);
	#endif


	return errNoError;
}


Error DateTime:: nowUTCInMilliSecs (
	unsigned long long *pullNowUTCInMilliSecs,
	long *plTimeZoneDifferenceInHours)

{

	unsigned long long	ullNowUTCInSecs;
	unsigned long		ulAdditionalMilliSecs;


	if (DateTime:: nowUTCInMilliSecs (
		&ullNowUTCInSecs,
		&ulAdditionalMilliSecs,
		plTimeZoneDifferenceInHours) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

		return err;
	}


	(*pullNowUTCInMilliSecs)			= ullNowUTCInSecs;
	(*pullNowUTCInMilliSecs)			*= 1000;
	(*pullNowUTCInMilliSecs)			+= ulAdditionalMilliSecs;



	return errNoError;
}


Error DateTime:: nowLocalInMilliSecs (
	unsigned long long *pullNowLocalInMilliSecs)

{

	unsigned long long	ullNowUTCInMilliSecs;
	long					lTimeZoneDifferenceInHours;


	if (DateTime:: nowUTCInMilliSecs (&ullNowUTCInMilliSecs,
		&lTimeZoneDifferenceInHours) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED);

		return err;
	}

	*pullNowLocalInMilliSecs			=
		ullNowUTCInMilliSecs + (lTimeZoneDifferenceInHours * 3600 * 1000);


	return errNoError;
}


Error DateTime:: nowLocalTime (char *pDateTime, unsigned long ulBufferSize,
	unsigned long ulTextFormat)

{
	tm					tmDateTime;
	unsigned long		ulMilliSecs;


	if ((ulTextFormat == 1 && ulBufferSize <= 20) ||
		(ulTextFormat == 2 && ulBufferSize <= 11) ||
		(ulTextFormat != 1 && ulTextFormat != 2))
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (DateTime:: get_tm_LocalTime (
		&tmDateTime, &ulMilliSecs) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_GET_TM_LOCALTIME_FAILED);

		return err;
	}

	if (ulTextFormat == 1)
	{
		sprintf (pDateTime,
			"%04lu-%02lu-%02lu %02lu:%02lu:%02lu",
			(unsigned long) (tmDateTime. tm_year + 1900),
			(unsigned long) (tmDateTime. tm_mon + 1),
			(unsigned long) (tmDateTime. tm_mday),
			(unsigned long) (tmDateTime. tm_hour),
			(unsigned long) (tmDateTime. tm_min),
			(unsigned long) (tmDateTime. tm_sec));
	}
	else if (ulTextFormat == 2)
	{
		sprintf (pDateTime,
			"%04lu_%02lu_%02lu",
			(unsigned long) (tmDateTime. tm_year + 1900),
			(unsigned long) (tmDateTime. tm_mon + 1),
			(unsigned long) (tmDateTime. tm_mday));
	}



	return errNoError;
}


Error DateTime:: getTimeZoneInformation (long *plTimeZoneDifferenceInHours)

{

	#ifdef WIN32
		TIME_ZONE_INFORMATION		tzInfo;

		if (::GetTimeZoneInformation (&tzInfo) == TIME_ZONE_ID_INVALID)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETTIMEZONEINFORMATION_FAILED);

			return err;
		}

		*plTimeZoneDifferenceInHours	= ((tzInfo. Bias / 60) * (-1));
	#else
		struct timeval				tv;
		struct timezone				tz;

		if (::gettimeofday (&tv, &tz) != 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETTIMEOFDAY_FAILED);

			return err;
		}

		*plTimeZoneDifferenceInHours	= ((tz. tz_minuteswest / 60) * (-1));
	#endif


	return errNoError;
}


long DateTime::getTimeZoneInformation (void)

{

	long lTimeZoneDifferenceInHours;
	Error errDateTime;

	if ((errDateTime = DateTime::getTimeZoneInformation (&lTimeZoneDifferenceInHours)) != errNoError)
	{
		throw runtime_error(string("DateTime:: getTimeZoneInformation failed: ")
			+ (const char *) errDateTime);
	}

	return lTimeZoneDifferenceInHours;
}


Error DateTime:: get_tm_LocalTime (
	tm *ptmDateTime, unsigned long *pulMilliSecs)

{

	#ifdef WIN32
		time_t				tTime;
		SYSTEMTIME			stSystemTime;


		tTime			= time (NULL);

		if (DateTime:: convertFromUTCToLocal (
			tTime, ptmDateTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_CONVERTFROMUTCTOLOCAL_FAILED);

			return err;
		}

		GetLocalTime (&stSystemTime);

		*pulMilliSecs		= stSystemTime. wMilliseconds;
	#else
		struct timeval		tvTimeval;


		if (gettimeofday (&tvTimeval, NULL) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETTIMEOFDAY_FAILED);

			return err;
		}

		if (DateTime:: convertFromUTCToLocal (
			tvTimeval. tv_sec, ptmDateTime) != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_DATETIME_CONVERTFROMUTCTOLOCAL_FAILED);

			return err;
		}


		*pulMilliSecs		= tvTimeval. tv_usec / 1000;
	#endif


	return errNoError;
}


Error DateTime:: convertFromLocalToUTC (
	tm *ptmDateTime, time_t *ptUTCTime)

{

	*ptUTCTime			= mktime (ptmDateTime);


	return errNoError;
}


Error DateTime:: convertFromLocalToUTC (
	tm *ptmLocalDateTime, tm *ptmUTCDateTime)

{

	time_t					tUTCTime;


	if (convertFromLocalToUTC (ptmLocalDateTime, &tUTCTime) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_CONVERTFROMLOCALTOUTC_FAILED);

		return err;
	}

	if (convertFromUTCInSecondsToBreakDownUTC (tUTCTime, ptmUTCDateTime) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_CONVERTFROMUTCINSECONDSTOBREAKDOWNUTC_FAILED);

		return err;
	}



	return errNoError;
}


Error DateTime:: convertFromUTCInSecondsToBreakDownUTC (
	time_t tUTCTime, tm *ptmUTCDateTime)

{

	gmtime_r (&tUTCTime, ptmUTCDateTime);


	return errNoError;
}


Error DateTime:: convertFromLocalDateTimeToLocalInSecs (
	unsigned long ulYear,
	unsigned long ulMon,
	unsigned long ulDay,
	unsigned long ulHour,
	unsigned long ulMin,
	unsigned long ulSec,
	long lDaylightSavingTime,
	unsigned long long *pullLocalInSecs)

{

	long					lTimeZoneDifferenceInHours;
	time_t					tUTCTime;
	tm						tmDateTime;


	tmDateTime. tm_year		= ulYear - 1900;
	tmDateTime. tm_mon		= ulMon - 1;
	tmDateTime. tm_mday		= ulDay;
	tmDateTime. tm_hour		= ulHour;
	tmDateTime. tm_min		= ulMin;
	tmDateTime. tm_sec		= ulSec;

	//	Negative value if status of daylight saving time is unknown.
	// (does it mean it attempt to determine whether Daylight Saving Time is in effect
	//	for the specified time?)
	tmDateTime. tm_isdst	= lDaylightSavingTime;

	// Simulate as the date is a local date
	if (DateTime:: convertFromLocalToUTC (
		&tmDateTime, &tUTCTime) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_CONVERTFROMLOCALTOUTC_FAILED);

		return err;
	}

	if (DateTime:: getTimeZoneInformation (
		&lTimeZoneDifferenceInHours) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_CONVERTFROMLOCALTOUTC_FAILED);

		return err;
	}

	*pullLocalInSecs			= ((unsigned long long) tUTCTime) +
		(((unsigned long long) lTimeZoneDifferenceInHours) * 3600);


	return errNoError;
}


Error DateTime:: convertFromUTCToLocal (
	time_t tUTCTime, tm *ptmLocalDateTime)

{

	#ifdef _REENTRANT
		#if defined(__hpux) && defined(_CMA__HP)
			if (localtime_r (&tUTCTime, ptmLocalDateTime))
		#else
			if (localtime_r (&tUTCTime, ptmLocalDateTime) ==
				(struct tm *) NULL)
		#endif
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_LOCALTIME_R_FAILED);

			return err;
		}
	#else
		if ((ptmLocalDateTime = localtime (&tUTCTime)) == (struct tm *) NULL)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_LOCALTIME_FAILED);

			return err;
		}
	#endif


	return errNoError;
}


Error DateTime:: addSeconds (
	unsigned long ulSrcYear,
	unsigned long ulSrcMonth,
	unsigned long ulSrcDay,
	unsigned long ulSrcHour,
	unsigned long ulSrcMinutes,
	unsigned long ulSrcSeconds,
	long lSrcDaylightSavingTime,
	long long llSecondsToAdd,
	unsigned long *pulDestYear,
	unsigned long *pulDestMonth,
	unsigned long *pulDestDay,
	unsigned long *pulDestHour,
	unsigned long *pulDestMinutes,
	unsigned long *pulDestSeconds,
	Boolean_p pbDestDaylightSavingTime)

{

	// initialize the tm struct (the tm_wday field too)
	time_t				tUtcTime;
	tm					tmDateTime;


	if (llSecondsToAdd == 0)
	{
		*pulDestYear				= ulSrcYear;
		*pulDestMonth				= ulSrcMonth;
		*pulDestDay					= ulSrcDay;
		*pulDestHour				= ulSrcHour;
		*pulDestMinutes				= ulSrcMinutes;
		*pulDestSeconds				= ulSrcSeconds;
		*pbDestDaylightSavingTime	= lSrcDaylightSavingTime;

		return errNoError;
	}

	tmDateTime. tm_year				= ulSrcYear;
	tmDateTime. tm_mon				= ulSrcMonth;
	tmDateTime. tm_mday				= ulSrcDay;
	tmDateTime. tm_hour				= ulSrcHour;
	tmDateTime. tm_min				= ulSrcMinutes;
	tmDateTime. tm_sec				= ulSrcSeconds;

	tmDateTime. tm_year				-= 1900;
	tmDateTime. tm_mon				-= 1;

	//	A negative value for tm_isdst causes mktime() to attempt
	//	to determine whether Daylight Saving Time is in effect
	//	for the specified time.
	tmDateTime. tm_isdst			= lSrcDaylightSavingTime;

	tUtcTime						= mktime (&tmDateTime);

	tUtcTime						= tUtcTime + llSecondsToAdd;

	if (DateTime:: convertFromUTCToLocal (
		tUtcTime, &tmDateTime) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_CONVERTFROMUTCTOLOCAL_FAILED);

		return err;
	}


	*pulDestYear						= tmDateTime. tm_year + 1900;
	*pulDestMonth						= tmDateTime. tm_mon + 1;
	*pulDestDay							= tmDateTime. tm_mday;
	*pulDestHour						= tmDateTime. tm_hour;
	*pulDestMinutes						= tmDateTime. tm_min;
	*pulDestSeconds						= tmDateTime. tm_sec;

	if (tmDateTime. tm_isdst == 1)
		*pbDestDaylightSavingTime			= true;
	else
		*pbDestDaylightSavingTime			= false;


	return errNoError;
}


Error DateTime:: isLeapYear (unsigned long ulYear,
	Boolean_p pbIsLeapYear)

{

	if (ulYear % 4 != 0)
		*pbIsLeapYear		= false;

	if (ulYear % 400 == 0)
		*pbIsLeapYear		= true;

	if (ulYear % 100 == 0)
		*pbIsLeapYear		= false;

	*pbIsLeapYear		= true;


	return errNoError;
}


Error DateTime:: getLastDayOfMonth (
	unsigned long ulYear, unsigned long ulMonth,
	unsigned long *pulLastDayOfMonth)

{

	Boolean_t				bIsLeapYear;
	int						piDaysInMonths[] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};


	if (ulMonth <= 0 || ulMonth > 12)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (DateTime:: isLeapYear (ulYear, &bIsLeapYear) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_DATETIME_ISLEAPYEAR_FAILED);

		return err;
	}

	*pulLastDayOfMonth		= piDaysInMonths [ulMonth - 1];

	if (ulMonth == 2 && bIsLeapYear)	// february of a leap year
		(*pulLastDayOfMonth)		+= 1;


	return errNoError;
}

