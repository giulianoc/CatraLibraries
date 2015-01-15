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

#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "SocketImpl.h"


int main (int iArgc, char **pArgv)

{

	std:: vector<SocketImpl:: IPAddress_t>		vIPAddresses;
	SocketImpl:: IPAddress_t					iaIPAddress;
	std:: vector<SocketImpl:: IPAddress_t>:: const_iterator
		it;


	if (SocketImpl:: getIPAddressesList (&vIPAddresses) != errNoError)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETIPADDRESSESLIST_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	for (it  = vIPAddresses. begin (); it != vIPAddresses. end ();
		it++)
	{
		iaIPAddress				= *it;

		std:: cout << iaIPAddress. pIPAddress << std:: endl;
		std:: cout << iaIPAddress. pHostName << std:: endl << std:: endl;
	}


	return 0;
}


