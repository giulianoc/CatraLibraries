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

#ifndef LoopThread_h
	#define LoopThread_h

	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "Tracer.h"
	#include "PMutex.h"


	#ifdef WIN32
		typedef class LoopThread: public WinThread
	#else
		typedef class LoopThread: public PosixThread
	#endif
	{
		private:
			Tracer_p			_ptTracer1;
			Tracer_p			_ptTracer2;

		protected:
			virtual Error run (void);

		public:
			LoopThread (void);

			~LoopThread (void);

			Error init (Tracer_p ptTracer1, Tracer_p ptTracer2);

			virtual Error cancel (void);

	} LoopThread_t, *LoopThread_p;

#endif

