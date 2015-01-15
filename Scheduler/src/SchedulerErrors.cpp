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


#include "SchedulerErrors.h"


ErrMsgBase:: ErrMsgsInfo SchedulerErrorsStr = {

	// Times
	SCH_TIMES_INIT_FAILED,
		"The init method of Times class failed",
	SCH_TIMES_FINISH_FAILED,
		"The finish method of Times class failed",
	SCH_TIMES_START_FAILED,
		"The start method of Times class failed",
	SCH_TIMES_STOP_FAILED,
		"The stop method of Times class failed",
	SCH_TIMES_UPDATENEXTPERIODICEXPIRATIONDATETIME_FAILED,
		"The updateNextPeriodicExpirationDateTime method of Times class failed",
	SCH_TIMES_UPDATENEXTCALENDAREXPIRATIONDATETIME_FAILED,
		"The updateNextCalendarExpirationDateTime method of Times class failed",
	SCH_TIMES_UPDATENEXTEXPIRATIONDATETIME_FAILED,
		"The updateNextExpirationDateTime method of Times class failed",
	SCH_TIMES_ADDSECONDSTODATETIME_FAILED,
		"The addSecondsToDateTime method of Times class failed",
	SCH_TIMES_ISSTARTED_FAILED,
		"The isStarted method of Times class failed",
	SCH_TIMES_GETCLASSNAME_FAILED,
		"The getClassName method of Times class failed",
	SCH_TIMES_HANDLETIMEOUT_FAILED,
		"The handleTimeOut method of Times class failed",
	SCH_TIMES_ISEXPIREDTIME_FAILED,
		"The isExpiredTime method of Times class failed",
	SCH_TIMES_NOTFOUND,
		"Times not found",
	SCH_LOCALTIME_R_FAILED,
		"The localtime_r function failed",
	SCH_SSCANF_FAILED,
		"The sscanf function failed",
	SCH_CLASSNAME_TOOLONG,
		"Class name too long",
	SCH_REACHED_LASTTIMEOUT,
		"Reached the last timeout",
	SCH_CALENDARSCHEDULE_WRONG,
		"The calendar schedule is wrong",
	SCH_SCHEDULEFIELDRANGE_WRONG,
		"The value %ld is wrong for the %s field",

	// Scheduler
	SCH_SCHEDULER_INIT_FAILED,
		"The init method of Scheduler class failed",
	SCH_SCHEDULER_FINISH_FAILED,
		"The finish method of Scheduler class failed",
	SCH_SCHEDULER_START_FAILED,
		"The start method of Scheduler class failed",
	SCH_SCHEDULER_SUSPEND_FAILED,
		"The suspend method of Scheduler class failed",
	SCH_SCHEDULER_RESUME_FAILED,
		"The resume method of Scheduler class failed",
	SCH_SCHEDULER_CANCEL_FAILED,
		"The cancel method of Scheduler class failed",
	SCH_SCHEDULER_HANDLETIMES_FAILED,
		"The handleTimes method of Scheduler class failed",
	SCH_SCHEDULER_DEACTIVETIMES_FAILED,
		"The deactiveTimes method of Scheduler class failed",
	SCH_SCHEDULER_ACTIVETIMES_FAILED,
		"The activeTimes method of Scheduler class failed",
	SCH_SCHEDULER_GETTIMESPOINTERINDEX_FAILED,
		"The getTimesPointerIndex method of Times class failed",
	SCH_SCHEDULER_GETTIMESNUMBER_FAILED,
		"The getTimesNumber method of Times class failed",
	SCH_SCHEDULER_GETTIMES_FAILED,
		"The getTimes method of Times class failed",
	SCH_SCHEDULER_TIMESALREADYACTIVE,
		"The times is alread active",

	// common
	SCH_NEW_FAILED,
		"new failed",
	SCH_ACTIVATION_WRONG,
		"The activation of the method is wrong",
	SCH_OPERATION_NOTALLOWED,
		"Operation not allowed (Current object status: %ld)"

	// Insert here other errors...

} ;

