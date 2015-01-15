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


#ifdef SNMPENABLED
	#include <net-snmp/net-snmp-config.h>
	#include <net-snmp/session_api.h>
#endif
#include <stdlib.h>
#if defined(WIN32)
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif
#include "SNMP.h"


#ifdef SNMPENABLED
	int mySNMPCallback (int operation, netsnmp_session *session,
		int reqid, netsnmp_pdu *pdu, void *magic)
	{
		return 1;
	}
#endif


SNMP:: SNMP (void)

{

}


SNMP:: SNMP (const SNMP &)

{

}


SNMP:: ~SNMP (void)

{

}


Error SNMP:: init (const char *pConfigurationPathName)

{

	ConfigurationFile_t		cfConfiguration;
	Error_t					errInit;


	#ifdef SNMPENABLED
	{
		Error								errParseError;

		if ((errParseError = cfConfiguration. init (
			pConfigurationPathName, "SNMP")) != errNoError)
		{
			// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			// 	CFGFILE_CONFIGURATIONFILE_INIT_FAILED);

			return errParseError;
		}
	}

	if ((errInit = init (&cfConfiguration)) != errNoError)
	{
		Error err = SNMPErrors (__FILE__, __LINE__,
			SNMP_SNMP_INIT_FAILED);

		if (cfConfiguration. finish () != errNoError)
		{
			// Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			// 	CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
			// std:: cerr << (const char *) err << std:: endl;
		}

		return errInit;
	}

	if (cfConfiguration. finish () != errNoError)
	{
		Error err = ConfigurationFileErrors (__FILE__, __LINE__,
			CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);

		return err;
	}
	#endif


	return errNoError;
}


Error SNMP:: init (ConfigurationFile_p pcfConfiguration)

{

	#ifdef SNMPENABLED
	Error_t					errGetItemValue;
	char					pConfigurationBuffer [
		SNMP_MAXCFGITEMLENGTH];


	if ((errGetItemValue = pcfConfiguration -> getItemValue ("SNMP",
		"Enabled", pConfigurationBuffer, SNMP_MAXCFGITEMLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
			CFG_CONFIG_GETITEMVALUE_FAILED,
			2, "SNMP", "Enabled");

		return err;
	}
	#ifdef WIN32
		if (!stricmp (pConfigurationBuffer, "true"))
	#else
		if (!strcasecmp (pConfigurationBuffer, "true"))
	#endif
		_bEnabled						= true;
	else
		_bEnabled						= false;

	if ((errGetItemValue = pcfConfiguration -> getItemValue (
		"SNMP", "ManagerIPAddressAndPort", _pManagerIPAddressAndPort,
		SNMP_MAXIPADDRESSANDPORTLENGTH)) != errNoError)
	{
		Error err = ConfigurationErrors (__FILE__, __LINE__,
	 		CFG_CONFIG_GETITEMVALUE_FAILED,
	 		2, "SNMP", "ManagerIPAddressAndPort");

		return err;
	}
	#endif


	return errNoError;
}


Error SNMP:: finish (void)

{


	return errNoError;
}


Error SNMP:: sendTrap (int iVersion, unsigned long ulUpTime,
	long lSpecificType, const char *pTrapMessage)

{
	#ifdef SNMPENABLED

	netsnmp_session		ssSNMPSession;
	netsnmp_session		*pssSNMPSessionHandle;
	netsnmp_pdu			*pnspNetSNMPPdu;
	netsnmp_pdu			*pnspNetSNMPPduClone;


	if (!_bEnabled)
	{
		Error err = SNMPErrors (__FILE__, __LINE__,
			SNMP_SNMP_NOTENABLED);

		return err;
	}

	if (iVersion != 1 && iVersion != 2)
	{
		Error err = SNMPErrors (__FILE__, __LINE__,
			SNMP_ACTIVATION_WRONG);

		return err;
	}

	if (iVersion == 1)
	{
		if ((pnspNetSNMPPdu = snmp_pdu_create (SNMP_MSG_TRAP)) ==
			(netsnmp_pdu *) NULL)
		{
			Error err = SNMPErrors (__FILE__, __LINE__,
				SNMP_SNMP_PDU_CREATE_FAILED);

			return err;
		}

		/*
		oid		oOIDCatraSoft []	= { 1, 3, 6, 1, 4, 1, 28285 };

		pnspNetSNMPPdu -> enterprise			=
			(oid *) malloc (sizeof (oOIDCatraSoft));
		memcpy (pnspNetSNMPPdu -> enterprise, oOIDCatraSoft,
			sizeof (oOIDCatraSoft));
		pnspNetSNMPPdu -> enterprise_length		= SNMP_OIDLENGTH;
		*/

		// setting pdu -> agent_addr
		/*
		in_addr_t *piaAgentAddress	=
			(in_addr_t *) pnspNetSNMPPdu -> agent_addr;
		*piaAgentAddress			= inet_addr (pAgentIPAddress);
		*/

		pnspNetSNMPPdu -> trap_type			= SNMP_TRAP_ENTERPRISESPECIFIC;
		pnspNetSNMPPdu -> specific_type		= lSpecificType;
		pnspNetSNMPPdu -> time				= ulUpTime;

		// add variable to the pdu
		{
			oid		oOIDIdentifier [MAX_OID_LEN] =
				{ 1, 3, 6, 1, 4, 1, 18285, 123 };
			size_t	sOIDLength		= 8;


			// int snmp_add_var (netsnmp_pdu *pdu, const oid *name,
			//		size_t name_length, char type, const char *value) 

			// for an integer value
			// snmp_add_var(pnspNetSNMPPdu, oOIDIdentifier, sOIDLength,
			// 	'i', "100");

			// for a string
			snmp_add_var(pnspNetSNMPPdu, oOIDIdentifier, sOIDLength,
				's', pTrapMessage);
		}
	}
	else	// if (iVersion == 2)
	{
		oid		objid_sysuptime[] = { 1, 3, 6, 1, 2, 1, 1, 3, 0 };

		if ((pnspNetSNMPPdu = snmp_pdu_create (SNMP_MSG_TRAP2)) ==
			(netsnmp_pdu *) NULL)
		{
			Error err = SNMPErrors (__FILE__, __LINE__,
				SNMP_SNMP_PDU_CREATE_FAILED);

			return err;
		}

		// set OID uptime
		char		sysuptime[20];

		memset (sysuptime, 0, sizeof (char) * 20);
		sprintf (sysuptime, "%lu", ulUpTime);

		snmp_add_var (pnspNetSNMPPdu, objid_sysuptime,
			sizeof(objid_sysuptime) / sizeof(oid), 't', sysuptime);


		// .....
		// vidiator_trap_root[10] = (oid)pData->trapType;

		// snmp_add_var (pnspNetSNMPPdu, objid_snmptrap,
		// 	sizeof(objid_snmptrap) / sizeof(oid),
		// 	'o', MakeOidStr(vidiator_trap_root, 11) );

		// add variable to the pdu
		{
			oid		oOIDIdentifier [MAX_OID_LEN];
			size_t	sOIDLength;


			// int snmp_add_var (netsnmp_pdu *pdu, const oid *name,
			//		size_t name_length, char type, const char *value) 

			// for an integer value
			// snmp_add_var(pnspNetSNMPPdu, oOIDIdentifier,
			// 	sOIDLength, 'i', "100");

			// for a string
			snmp_add_var(pnspNetSNMPPdu, oOIDIdentifier,
				sOIDLength, 's', pTrapMessage);
		}
	}

	// for each manager
	{
		snmp_sess_init (&ssSNMPSession);

		if (iVersion == 1)
			ssSNMPSession. version				= SNMP_VERSION_1;
		else	// if (iVersion == 2)
			ssSNMPSession. version				= SNMP_VERSION_2c;

		// ManagerIPAddress:ManagerPort (Port by default is 162)
		ssSNMPSession. peername				=
			(char *) _pManagerIPAddressAndPort;
		// ssSNMPSession. callback				= mySNMPCallback;
		ssSNMPSession. callback_magic		= NULL;

		// ssSNMPSession. community			= (u_char *) pCommunity;
		// ssSNMPSession. community_len		= strlen (pCommunity);

		pssSNMPSessionHandle		= snmp_open (&ssSNMPSession);

		pnspNetSNMPPduClone		= snmp_clone_pdu (pnspNetSNMPPdu);

		snmp_send (pssSNMPSessionHandle, pnspNetSNMPPduClone);

		snmp_close (pssSNMPSessionHandle);
	}

	#endif


	return errNoError;
}


