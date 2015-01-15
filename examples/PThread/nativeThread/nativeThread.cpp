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
#include <pthread.h>
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif

void *runFunction (void *pvPThread)

{

	std:: cout << "Sono il thread" << std:: endl;

	#if defined(WIN32)
		Sleep (300);
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		usleep (300000);
	#endif

	// pthread_exit (NULL);

	return (void *) NULL;

}

int main ()

{
	void				*pvStatus;
	pthread_t			_pThread;
	pthread_attr_t		_pThreadAttribute;
 

	for (long lIndex = 0; lIndex < 1; lIndex++)
	{
		std:: cout << "Index: " << lIndex << std:: endl;

		std:: cout << pthread_attr_init (&_pThreadAttribute) << std:: endl;

		std:: cout << pthread_attr_setdetachstate (&_pThreadAttribute, PTHREAD_CREATE_DETACHED)
			<< std:: endl;

		std:: cout << pthread_create (&_pThread, &_pThreadAttribute, runFunction, NULL) << std:: endl;
		// std:: cout << pthread_create (&_pThread, NULL, runFunction, NULL) << std:: endl;

		// std:: cout << pthread_detach (_pThread) << std:: endl;

		// std:: cout << pthread_join (_pThread, &pvStatus) << std:: endl;

		std:: cout << pthread_attr_destroy (&_pThreadAttribute) << std:: endl;

		#if defined(WIN32)
			Sleep (300);
		#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
			usleep (300000);
		#endif
	}

	#if defined(WIN32)
		Sleep (1000 * 1000);
	#else	// POSIX.1-1996 standard (SunOs, HPUX 11)
		sleep (1000);
	#endif

}

