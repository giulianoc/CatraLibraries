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


#ifndef ProcessUtility_h
	#define ProcessUtility_h

        #include <string>
	#include "ToolsErrors.h"
	#ifdef WIN32
	#else
	#endif

        using namespace std;

	/**
		The ProcessUtility class is a collection of static methods just
		to hide the differences to retrieve information between
		different operative system.
	*/
	typedef class ProcessUtility

	{

		private:
			ProcessUtility (const ProcessUtility &);

			ProcessUtility &operator = (const ProcessUtility &);

		public:
			/**
				Costruttore.
			*/
			ProcessUtility ();

			/**
				Distruttore.
			*/
			~ProcessUtility ();

			/**
				Return the current process identifier.
			*/
			static Error getCurrentProcessIdentifier (
				long *plProcessIdentifier);

			/**
				Executes a command specified in pCommand and
				returns after the command has been completed.

				piReturnedStatus contains the return code
				of the child which terminated, which may have been set as the
				argument to a call to exit() or _exit() or as the argument for a
				return statement in the main program.
			*/
			static Error execute (const char *pCommand,
				int *piReturnedStatus);

                        static int execute (string command);

                        /**
				Set the user and group ID of the current process.
			*/
			#ifdef WIN32
			#else
				static Error setUserAndGroupID (const char *pUserName);
			#endif

			/*
			static Error getProcessorsNumber (
				unsigned long *pulProcessorsNumber);
			*/

	} ProcessUtility_t, *ProcessUtility_p;

#endif

