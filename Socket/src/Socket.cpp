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


#include "Socket.h"
#include <assert.h>



Socket:: Socket (void)

{

	_ulIdentifier						= 0;

}


Socket:: ~Socket (void)

{

}


Socket:: Socket (const Socket &t)

{

	assert (1 == 0);

}


Error Socket:: init (unsigned long ulIdentifier)

{

	if ((_pSocketImpl = new SocketImpl_t ()) == (SocketImpl_p) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_NEW_FAILED);

		return err;
	}

	_ulIdentifier						= ulIdentifier;


	return errNoError;
}


Error Socket:: finish (void)

{

	delete _pSocketImpl;
	_pSocketImpl				= (SocketImpl_p) NULL;


	return errNoError;
}


Error Socket:: getSocketImpl (SocketImpl_p *pSocketImpl)

{

	if (pSocketImpl == (SocketImpl_p *) NULL)
	{
		Error err = SocketErrors (__FILE__, __LINE__,
			SCK_ACTIVATION_WRONG);

		return err;
	}

	*pSocketImpl		= _pSocketImpl;


	return errNoError;
}


Error Socket:: setIdentifier (unsigned long ulIdentifier)

{
	_ulIdentifier				= ulIdentifier;


	return errNoError;
}


unsigned long Socket:: getIdentifier (void)

{

	return _ulIdentifier;
}


