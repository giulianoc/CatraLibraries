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


#ifndef SchedulerErrors_h
	#define SchedulerErrors_h

	#include "Error.h"
	#include <iostream>

	/**
		Click <a href="SchedulerErrors.C#SchedulerErrors" target=classContent>here</a> for the errors strings.
	*/
	enum SchedulerErrorsCodes {

		// Times
		SCH_TIMES_INIT_FAILED,
		SCH_TIMES_FINISH_FAILED,
		SCH_TIMES_START_FAILED,
		SCH_TIMES_STOP_FAILED,
		SCH_TIMES_UPDATENEXTPERIODICEXPIRATIONDATETIME_FAILED,
		SCH_TIMES_UPDATENEXTCALENDAREXPIRATIONDATETIME_FAILED,
		SCH_TIMES_UPDATENEXTEXPIRATIONDATETIME_FAILED,
		SCH_TIMES_ADDSECONDSTODATETIME_FAILED,
		SCH_TIMES_ISSTARTED_FAILED,
		SCH_TIMES_GETCLASSNAME_FAILED,
		SCH_TIMES_HANDLETIMEOUT_FAILED,
		SCH_TIMES_ISEXPIREDTIME_FAILED,
		SCH_TIMES_NOTFOUND,
		SCH_LOCALTIME_R_FAILED,
		SCH_SSCANF_FAILED,
		SCH_CLASSNAME_TOOLONG,
		SCH_REACHED_LASTTIMEOUT,
		SCH_CALENDARSCHEDULE_WRONG,
		SCH_SCHEDULEFIELDRANGE_WRONG,

		// Scheduler
		SCH_SCHEDULER_INIT_FAILED,
		SCH_SCHEDULER_FINISH_FAILED,
		SCH_SCHEDULER_START_FAILED,
		SCH_SCHEDULER_SUSPEND_FAILED,
		SCH_SCHEDULER_RESUME_FAILED,
		SCH_SCHEDULER_CANCEL_FAILED,
		SCH_SCHEDULER_HANDLETIMES_FAILED,
		SCH_SCHEDULER_DEACTIVETIMES_FAILED,
		SCH_SCHEDULER_ACTIVETIMES_FAILED,
		SCH_SCHEDULER_GETTIMESPOINTERINDEX_FAILED,
		SCH_SCHEDULER_GETTIMESNUMBER_FAILED,
		SCH_SCHEDULER_GETTIMES_FAILED,
		SCH_SCHEDULER_TIMESALREADYACTIVE,

		// common
		SCH_NEW_FAILED,
		SCH_ACTIVATION_WRONG,
		SCH_OPERATION_NOTALLOWED,

		// Insert here other errors...

		SCH_MAXERRORS
	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long', possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomErrorClass (SchedulerErrors, SCH_MAXERRORS)
   
#endif

