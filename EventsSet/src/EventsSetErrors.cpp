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

#include "EventsSetErrors.h"


ErrMsgBase:: ErrMsgsInfo EventsSetErrorsStr = {


	// Event
	EVSET_EVENT_INIT_FAILED,
		"The init method of Event class failed",
	EVSET_EVENT_FINISH_FAILED,
		"The finish method of Event class failed",
	EVSET_EVENT_GETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED,
		"The getExpirationLocalDateTimeInMilliSecs method of Event class failed",
	EVSET_EVENT_SETEXPIRATIONLOCALDATETIMEINMILLISECS_FAILED,
		"The setExpirationLocalDateTimeInMilliSecs method of Event class failed",
	EVSET_EVENT_GETTYPEIDENTIFIER_FAILED,
		"The getTypeIdentifier method of Event class failed",
	EVSET_EVENT_GETDESTINATION_FAILED,
		"The getDestination method of Event class failed",

	// EventsSet
	EVSET_EVENTSSET_INIT_FAILED,
		"The init method of EventsSet class failed",
	EVSET_EVENTSSET_FINISH_FAILED,
		"The finish method of EventsSet class failed",
	EVSET_EVENTSSET_GETFREEEVENT_FAILED,
		"The getFreeEvent method of EventsSet class failed",
	EVSET_EVENTSSET_RELEASEEVENT_FAILED,
		"The releaseEvent method of EventsSet class failed",
	EVSET_EVENTSSET_ALLOCATEMOREFREEUSEREVENTS_FAILED,
		"The allocateMoreFreeUserEvents method of EventsSet class failed",
	EVSET_EVENTSSET_DELETEALLOCATEDEVENTS_FAILED,
		"The deleteAllocatedEvents method of EventsSet class failed",
	EVSET_EVENTSSET_ADDEVENT_FAILED,
		"The addEvent method of EventsSet class failed",
	EVSET_EVENTSSET_ADDDESTINATION_FAILED,
		"The addDestination method of EventsSet class failed",
	EVSET_EVENTSSET_DELETEEVENT_FAILED,
		"The deleteEvent method of EventsSet class failed",
	EVSET_EVENTSSET_GETFIRSTEVENT_FAILED,
		"The getFirstEvent method of EventsSet class failed",
	EVSET_EVENTSSET_GETANDREMOVEFIRSTEVENT_FAILED,
		"The getAndRemoveFirstEvent method of EventsSet class failed",
	EVSET_EVENTSSET_DESTINATIONNOTFOUND,
		"Destination (%s) not found",
	EVSET_EVENTSSET_DESTINATIONALREADYADDED,
		"Destination (%s) already added",
	EVSET_EVENTSSET_EVENTNOTFOUND,
		"Event (%llu) not found",
	EVSET_EVENTSSET_NOEVENTSFOUND,
		"No events found",
	EVSET_EVENTSSET_KEYDUPLICATED,
		"The key (%lld) has already been inserted",
	EVSET_EVENTSSET_NOEVENTFREEFOUND,
		"No events free available",
	EVSET_EVENTSSET_EVENTNOTEXPIREDYET,
		"Event not expired",
	EVSET_EVENTSSET_NOFREEEVENTSALLOCATED,
		"No free events allocated",
	EVSET_EVENTSSET_EVENTTYPEOVERCOMENUMBEROFDIFFERENTEVENTTYPES,
		"Event type index (%lu) overcome the number of different event types",
	EVSET_EVENTSSET_UNKNOWNEVENTTYPE,
		"Unknown event type index (%lu)",
	EVSET_EVENTSSET_REACHEDMAXIMUMEVENTSTOALLOCATE,
		"Reached the maximum numbers of events to allocate (%lu)",

	// common
	EVSET_NEW_FAILED,
		"The new function failed",
	EVSET_OPERATION_NOTALLOWED,
		"The operation is not allowed. Status is: %ld",
	EVSET_ACTIVATION_WRONG,
		"Activation wrong",
	EVSET_LOCALTIME_R_FAILED,
		"The localtime_r function failed"

	// Insert here other errors...

} ;

