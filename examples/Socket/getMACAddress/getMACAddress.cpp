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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "SocketImpl.h"


int main (int iArgc, char **pArgv)

{

	char			pNetworkName [1024];
	unsigned char	pucMACAddress [6];
	Error_t			errGetMAC;


	#ifdef WIN32
		strcpy (pNetworkName, "0");
	#else
		strcpy (pNetworkName, "eth0");
	#endif

	if ((errGetMAC = SocketImpl:: getMACAddress (pNetworkName,
		pucMACAddress)) != errNoError)
	{
		std:: cout << (const char *) errGetMAC << std:: endl;
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_SOCKETIMPL_GETMACADDRESS_FAILED);
		std:: cout << (const char *) err << std:: endl;

		return 1;
	}

	{
		char		pBuffer [512 + 1];

		sprintf (pBuffer, "%02X:%02X:%02X:%02X:%02X:%02X",
			pucMACAddress [0],
			pucMACAddress [1],
			pucMACAddress [2],
			pucMACAddress [3],
			pucMACAddress [4],
			pucMACAddress [5]);
		std:: cout << "MAC address = " << pBuffer << std:: endl;
	}


	return 0;
}


