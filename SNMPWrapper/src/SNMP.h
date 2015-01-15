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


#ifndef SNMP_h
	#define SNMP_h


	#include "ConfigurationFile.h"
	#include "SNMPErrors.h"

	#define SNMP_OIDLENGTH						7

	#define SNMP_MAXIPADDRESSANDPORTLENGTH		64
	#define SNMP_MAXCFGITEMLENGTH				128


	typedef class SNMP

	{
		private:
			Boolean_t			_bEnabled;
			char				_pManagerIPAddressAndPort [
				SNMP_MAXIPADDRESSANDPORTLENGTH];


		protected:


		public:
			SNMP (void);

			SNMP (const SNMP &);

			~SNMP (void);

			Error init (const char *pConfigurationPathName);

			Error init (ConfigurationFile_p pcfConfiguration);

			Error finish (void);

			/*
			 * iVersion: 1 or 2
			 */
			Error sendTrap (int iVersion, unsigned long ulUpTime,
				long lSpecificType, const char *pTrapMessage);

	} SNMP_t, *SNMP_p;

#endif

