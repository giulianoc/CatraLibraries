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

#ifndef EventsSetErrors_h
	#define EventsSetErrors_h

	#include "Error.h"
	#include <iostream>

	/**
		Click <a href="EventsSetErrors.C#EventsSetErrors" target=classContent>here</a> for the errors strings.
	*/
	enum EventsSetErrorsCodes {

		// Event
		EVSET_EVENT_INIT_FAILED,
		EVSET_EVENT_FINISH_FAILED,
		EVSET_EVENT_GETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED,
		EVSET_EVENT_SETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED,
		EVSET_EVENT_GETTYPEIDENTIFIER_FAILED,
		EVSET_EVENT_GETDESTINATION_FAILED,

		// EventsSet
		EVSET_EVENTSSET_INIT_FAILED,
		EVSET_EVENTSSET_FINISH_FAILED,
		EVSET_EVENTSSET_GETFREEEVENT_FAILED,
		EVSET_EVENTSSET_RELEASEEVENT_FAILED,
		EVSET_EVENTSSET_ALLOCATEMOREFREEUSEREVENTS_FAILED,
		EVSET_EVENTSSET_DELETEALLOCATEDEVENTS_FAILED,
		EVSET_EVENTSSET_ADDEVENT_FAILED,
		EVSET_EVENTSSET_ADDDESTINATION_FAILED,
		EVSET_EVENTSSET_DELETEEVENT_FAILED,
		EVSET_EVENTSSET_GETFIRSTEVENT_FAILED,
		EVSET_EVENTSSET_GETANDREMOVEFIRSTEVENT_FAILED,
		EVSET_EVENTSSET_DESTINATIONNOTFOUND,
		EVSET_EVENTSSET_DESTINATIONALREADYADDED,
		EVSET_EVENTSSET_EVENTNOTFOUND,
		EVSET_EVENTSSET_NOEVENTSFOUND,
		EVSET_EVENTSSET_KEYDUPLICATED,
		EVSET_EVENTSSET_NOEVENTFREEFOUND,
		EVSET_EVENTSSET_EVENTNOTEXPIREDYET,
		EVSET_EVENTSSET_NOFREEEVENTSALLOCATED,
		EVSET_EVENTSSET_EVENTTYPEOVERCOMENUMBEROFDIFFERENTEVENTTYPES,
		EVSET_EVENTSSET_UNKNOWNEVENTTYPE,
		EVSET_EVENTSSET_REACHEDMAXIMUMEVENTSTOALLOCATE,

		// common
		EVSET_NEW_FAILED,
		EVSET_OPERATION_NOTALLOWED,
		EVSET_ACTIVATION_WRONG,
		EVSET_LOCALTIME_R_FAILED,

		// Insert here other errors...

		EVSET_MAXERRORS

	} ;

	// declaration of class error
	dclCustomErrorClass (EventsSetErrors, EVSET_MAXERRORS)
   
#endif
