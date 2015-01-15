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

#include "SNMP.h"
#ifdef WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#endif


int main (int iArgc, char *pArgv [])

{

	SNMP_t						tTrap;
	Error_t						errSNMP;
	// const char					*pPathName;


	/*
	if (iArgc != 2)
	{
		std:: cerr << "Usage: " << pArgv [0] << " <pathname>"
			<< std:: endl;

		return 1;
	}

	pPathName			= pArgv [1];
	*/

	std:: cout << "tTrap. init" << std:: endl;
	if ((errSNMP = tTrap. init ("./SNMP.cfg")) != errNoError)
	{
		std:: cerr << (const char *) errSNMP << std:: endl;

		Error err = SNMPErrors (__FILE__, __LINE__,
			SNMP_SNMP_INIT_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << "tTrap. sendTrap" << std:: endl;
	if ((errSNMP = tTrap. sendTrap (1, time(NULL), 1234, "MyMessage")) !=
		errNoError)
	{
		std:: cerr << (const char *) errSNMP << std:: endl;

		Error err = SNMPErrors (__FILE__, __LINE__,
			SNMP_SNMP_SENDTRAP_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout << "tTrap. finish" << std:: endl;
	if ((errSNMP = tTrap. finish ()) != errNoError)
	{
		std:: cerr << (const char *) errSNMP << std:: endl;

		Error err = SNMPErrors (__FILE__, __LINE__,
			SNMP_SNMP_FINISH_FAILED);
		std:: cerr << (const char *) err << std:: endl;

		return 1;
	}

	std:: cout. flush ();


	return 0;
}

