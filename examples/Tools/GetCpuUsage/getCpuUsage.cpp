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

#include <iostream>
#include <string.h>
#ifdef WIN32
#else
	#include <unistd.h>
#endif
#include "GetCpuUsage.h"


int main (int iArgc, char **pArgv)

{

	GetCpuUsage_t			gcuGetCpuUsage;


	while (1)
	{
		std:: cout << gcuGetCpuUsage. getCpuUsage () << std:: endl;

		#ifdef WIN32
			Sleep (1000);
		#else
			sleep (1);
		#endif
	}

	return 0;
}

