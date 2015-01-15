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


#include "CustomSocketsPool.h"



CustomSocketsPool:: CustomSocketsPool (void): SocketsPool ()

{

}


CustomSocketsPool:: ~CustomSocketsPool (void)

{

}


Error CustomSocketsPool:: updateSocketStatus (
	Socket_p pSocket, long lSocketIdentifier,
	void *pvSocketData, unsigned short usSocketStatusType)

{

	// REMEMBER, you cannot call deleteSocket from thi method if
	//	bAllowDeletionSocketFromUpdateMethod (parameter in inti)
	//	is false
	if (usSocketStatusType & SOCKETSTATUS_EXCEPTION)
	{
		std:: cerr << (const char *) pvSocketData << " - Error on socket"
			<< std:: endl;

		return errNoError;
	}

	if (usSocketStatusType & SOCKETSTATUS_WRITE)
	{
		std:: cerr << (const char *) pvSocketData
			<< " - Write on socket. Strange!!!!!" << std:: endl;

		return errNoError;
	}

	if (!(usSocketStatusType & SOCKETSTATUS_READ))
	{
		std:: cerr << (const char *) pvSocketData
			<< " - Not read on socket. Strange!!!!!" << std:: endl;

		return errNoError;
	}

	// usSocketStatusType | SOCKETSTATUS_READ
	std:: cout << (const char *) pvSocketData
		<< " - client connection received" << std:: endl;


	return errNoError;
}


