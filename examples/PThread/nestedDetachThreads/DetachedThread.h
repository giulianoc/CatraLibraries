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

#ifndef DetachedThread_h
	#define DetachedThread_h

	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif


	#ifdef WIN32
		typedef class DetachedThread: public WinThread
	#else
		typedef class DetachedThread: public PosixThread
	#endif
	{
		protected:
			virtual Error run (void);

		public:
			DetachedThread (void);

			~DetachedThread (void);

			Error init (void);

			virtual Error cancel (void);

	} DetachedThread_t, *DetachedThread_p;

#endif

