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


#ifndef Network_h
	#define Network_h

	#include "ToolsErrors.h"
	#ifdef WIN32
	#else
	#endif

    #define NET_MAXIPADDRESSLENGTH			(15 + 1)


	typedef class Network

	{
		public:
			typedef struct NetworkIP {
				char			_pIPAddress [NET_MAXIPADDRESSLENGTH];
				char			_pMaskAddress [NET_MAXIPADDRESSLENGTH];
			} NetworkIP_t, *NetworkIP_p;

		private:
			NetworkIP_t			_niNetworkIP;
			unsigned long		_ulIPFirst;
			unsigned long		_ulIPSecond;
			unsigned long		_ulIPThird;
			unsigned long		_ulIPFourth;
			unsigned long		_ulMaskFirst;
			unsigned long		_ulMaskSecond;
			unsigned long		_ulMaskThird;
			unsigned long		_ulMaskFourth;
			unsigned long		_ulSubnetIPFirst;
			unsigned long		_ulSubnetIPSecond;
			unsigned long		_ulSubnetIPThird;
			unsigned long		_ulSubnetIPFourth;


			Network (const Network &);

			Network &operator = (const Network &);

		public:
			/**
				Costruttore.
			*/
			Network ();

			/**
				Distruttore.
			*/
			~Network ();

			/**
			*/
			Error init (const char *pIPAddress,
				const char *pMaskAddress);

			/**
			*/
			// Error finish (void);

			/**
			*/
			Error isIPInNetwork (const char *pIPAddress,
				Boolean_p pbIsIPInNetwork);

			static Error parseIPAddress (const char *pIPAddress,
				unsigned long *pulIPFirst,
				unsigned long *pulIPSecond,
				unsigned long *pulIPThird,
				unsigned long *pulIPFourth);

			/*
			 * this method receives an hostname or ip address
			 * and returns the IP Address
			 */
			static Error getIPAddressFromHostName (
				const char *pHostNameOrIPAddress,
				char *pIPAddress,
				unsigned long ulIPAddressBufferLength);

	} Network_t, *Network_p;

#endif

