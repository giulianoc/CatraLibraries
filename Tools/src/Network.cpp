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


#ifdef WIN32
	#include <io.h>
	#include <winsock2.h>
	#include <windows.h>
	#include <ws2spi.h>
	#include <ws2tcpip.h>
	#include <Iphlpapi.h>
#else
	#include <sys/socket.h>
	#include <netdb.h>
#endif
#include <sys/types.h>
#include "Network.h"
#include "StringTokenizer.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>


Network:: Network (void)

{

}


Network:: ~Network (void)

{

}



Network:: Network (const Network &)

{

	assert (1==0);

	// to do

}


Network &Network:: operator = (const Network &)

{

	assert (1==0);

	// to do

	return *this;

}


Error Network:: init (const char *pIPAddress,
	const char *pMaskAddress)

{

	StringTokenizer_t			stStringTokenizer;
	Error						errNextToken;


	if (strlen (pIPAddress) >= NET_MAXIPADDRESSLENGTH ||
		strlen (pMaskAddress) >= NET_MAXIPADDRESSLENGTH)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	strcpy (_niNetworkIP. _pIPAddress, pIPAddress);
	strcpy (_niNetworkIP. _pMaskAddress, pMaskAddress);

	if (parseIPAddress (_niNetworkIP. _pIPAddress,
		&_ulIPFirst,
		&_ulIPSecond,
		&_ulIPThird,
		&_ulIPFourth) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NETWORK_PARSEIPADDRESS_FAILED);

		return err;
	}

	if (parseIPAddress (_niNetworkIP. _pMaskAddress,
		&_ulMaskFirst,
		&_ulMaskSecond,
		&_ulMaskThird,
		&_ulMaskFourth) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NETWORK_PARSEIPADDRESS_FAILED);

		return err;
	}

	_ulSubnetIPFirst		= _ulIPFirst & _ulMaskFirst;
	_ulSubnetIPSecond		= _ulIPSecond & _ulMaskSecond;
	_ulSubnetIPThird		= _ulIPThird & _ulMaskThird;
	_ulSubnetIPFourth		= _ulIPFourth & _ulMaskFourth;


	return errNoError;
}


/*
Error Network:: finish (void)

{

	return errNoError;
}
*/


Error Network:: isIPInNetwork (const char *pIPAddress,
	Boolean_p pbIsIPInNetwork)

{

	unsigned long			ulLocalIPFirst;
	unsigned long			ulLocalIPSecond;
	unsigned long			ulLocalIPThird;
	unsigned long			ulLocalIPFourth;


	if (parseIPAddress (pIPAddress,
		&ulLocalIPFirst,
		&ulLocalIPSecond,
		&ulLocalIPThird,
		&ulLocalIPFourth) != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NETWORK_PARSEIPADDRESS_FAILED);

		return err;
	}

	if (((ulLocalIPFirst & _ulMaskFirst) == _ulSubnetIPFirst) &&
		((ulLocalIPSecond & _ulMaskSecond) == _ulSubnetIPSecond) &&
		((ulLocalIPThird & _ulMaskThird) == _ulSubnetIPThird) &&
		((ulLocalIPFourth & _ulMaskFourth) == _ulSubnetIPFourth))
		*pbIsIPInNetwork		= true;
	else
		*pbIsIPInNetwork		= false;


	return errNoError;
}


Error Network:: parseIPAddress (const char *pIPAddress,
	unsigned long *pulIPFirst,
	unsigned long *pulIPSecond,
	unsigned long *pulIPThird,
	unsigned long *pulIPFourth)

{

	StringTokenizer_t			stStringTokenizer;
	Error						errNextToken;
	const char					*pToken;


	if (strlen (pIPAddress) >= NET_MAXIPADDRESSLENGTH)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (stStringTokenizer. init (pIPAddress, -1, ".") !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_INIT_FAILED);

		return err;
	}

	if ((errNextToken = stStringTokenizer. nextToken (&pToken)) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

		if (stStringTokenizer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_FINISH_FAILED);
		}

		return err;
	}

	*pulIPFirst			= atol (pToken);

	if ((errNextToken = stStringTokenizer. nextToken (&pToken)) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

		if (stStringTokenizer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_FINISH_FAILED);
		}

		return err;
	}

	*pulIPSecond			= atol (pToken);

	if ((errNextToken = stStringTokenizer. nextToken (&pToken)) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

		if (stStringTokenizer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_FINISH_FAILED);
		}

		return err;
	}

	*pulIPThird			= atol (pToken);

	if ((errNextToken = stStringTokenizer. nextToken (&pToken)) !=
		errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

		if (stStringTokenizer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_FINISH_FAILED);
		}

		return err;
	}

	*pulIPFourth			= atol (pToken);

	if (stStringTokenizer. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_FINISH_FAILED);

		return err;
	}


	return errNoError;
}


Error Network:: getIPAddressFromHostName (
	const char *pHostNameOrIPAddress,
	char *pIPAddress,
	unsigned long ulIPAddressBufferLength)

{

	struct addrinfo			*paiAddrInfo;
	long					lReturn;
	struct addrinfo			*paiLocalAddrInfo;


	if (ulIPAddressBufferLength == 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if ((lReturn = getaddrinfo (pHostNameOrIPAddress, (const char *) NULL,
		(const struct addrinfo *) NULL, &paiAddrInfo)) != 0)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_GETADDRINFO_FAILED,
			2, lReturn, pHostNameOrIPAddress);

		return err;
	}

	for (paiLocalAddrInfo = paiAddrInfo;
		paiLocalAddrInfo != (struct addrinfo *) NULL;
		paiLocalAddrInfo = paiLocalAddrInfo -> ai_next)
	{
		pIPAddress [0]		= '\0';

		if ((lReturn = getnameinfo (paiLocalAddrInfo -> ai_addr,
			paiLocalAddrInfo -> ai_addrlen, pIPAddress, ulIPAddressBufferLength,
			(char *) NULL, 0, NI_NUMERICHOST)) != 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETNAMEINFO_FAILED,
				1, lReturn);

			freeaddrinfo (paiAddrInfo);

			return err;
		}

		if (pIPAddress [0] != '\0')
			break;
	}

	freeaddrinfo (paiAddrInfo);

	if (paiLocalAddrInfo == (struct addrinfo *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NETWORK_IPADDRESSNOTRETRIEVED,
			1, pHostNameOrIPAddress);

		return err;
	}


	return errNoError;
}

