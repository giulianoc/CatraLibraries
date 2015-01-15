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

#ifndef TimesForMeeting1_h
	#define TimesForMeeting1_h


	#include "Times.h"


	typedef class TimesForMeeting1: public Times

	{
		private:

		protected:
//			TimesForMeeting1 (const TimesForMeeting1 &m);

		public:
			TimesForMeeting1 (void);

			~TimesForMeeting1 (void);

			virtual Error handleTimeOut (void);

	} TimesForMeeting1_t, *TimesForMeeting1_p;

#endif

