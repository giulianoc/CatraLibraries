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

#include "ConfigurationFile.h"
#include "MyLoadBalancer.h"
#include "PosixThread.h"
#include <time.h>



int main (int argc, char **argv)

{

	ConfigurationFile_t			cfConfiguration;
	long						lIndex;
	char						*pConfigFilePathName;
	Error_t						errParseError;
	Error_t						errInit;
	Error_t						errGetAvailModule;
	MyLoadBalancer_t			lbLoadBalancer;
	char						pIPAddress [SCK_MAXIPADDRESSLENGTH];
	unsigned long				ulPort;
	Tracer_t					tTracer;
	time_t						tNow;


	if (argc != 2)
	{
		std:: cout << "Usage: " << argv [0] << " <pConfigFilePathName>" << std:: endl;

		return 1;
	}

	pConfigFilePathName			= argv [1];

	std:: cout << "Args: " << pConfigFilePathName << std:: endl;

	Error_t								errJoin;


	if (tTracer. init (
		"demoTracer",					// pName
		-1,								// lCacheSizeOfTraceFile K-byte
		"./logs",						// pTraceDirectory
		"traceFile",					// pTraceFileName
		5000,							// lMaxTraceFileSize K-byte
		150000,							// lTraceFilePeriodInSecs
		false,							// bCompressedTraceFile
		false,							// _bClosedFileToBeCopied
		(const char *) NULL,			// _pClosedFilesRepository
		100,							// lTraceFilesNumberToMaintain
		true,							// bTraceOnFile
		true,							// bTraceOnTTY
		0,								// lTraceLevel
		7,								// lSecondsBetweenTwoCheckTraceToManage
		3000,							// lMaxTracesNumber
		-1,								// lListenPort
		10000,							// lTracesNumberAllocatedOnOverflow
		1000) != errNoError)			// lSizeOfEachBlockToGzip K-byte
	{

		std:: cerr << "Tracer. init failed" << std:: endl;

		return 1;
	}

	if (tTracer. start () != errNoError)
	{
		std:: cerr << "Tracer. start failed" << std:: endl;

		tTracer. finish (true);

		return 1;
	}

	for (lIndex = 0; lIndex < 1000; lIndex++)
	{
		if ((errParseError = cfConfiguration. init (pConfigFilePathName,
			"", 20, 5, 32000)) != errNoError)
		{
			std:: cout << (const char *) errParseError << std:: endl;
			Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				CFGFILE_CONFIGURATIONFILE_INIT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if ((errInit = lbLoadBalancer. init (&cfConfiguration,
			"LoadBalancer", &tTracer)) != errNoError)
		{
			std:: cout << (const char *) errInit << std:: endl;
			Error err = LoadBalancerErrors (__FILE__, __LINE__,
				LB_LOADBALANCER_INIT_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		tNow			= time(NULL);

		while (time(NULL) - tNow <= 5 * 60)
		{
			PosixThread:: getSleep (1, 0);

			std:: cout << ".";
			std:: cout. flush ();

			// std:: cout << "before lbLoadBalancer. checkAllDeliveryModules"
			// 	<< std:: endl;
			if (lbLoadBalancer. checkAllDeliveryModules () != errNoError)
			{
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_CHECKALLDELIVERYMODULES_FAILED);
				std:: cout << (const char *) err << std:: endl;

				return 1;
			}
			// std:: cout << "later lbLoadBalancer. checkAllDeliveryModules"
			// 	<< std:: endl;

			//if ((errGetAvailModule = lbLoadBalancer. getAvailableModule (
			//	"Streamer", pIPAddress, SCK_MAXIPADDRESSLENGTH, &ulPort)) !=
			//	errNoError)
			//{
			//	std:: cout << (const char *) errGetAvailModule << std:: endl;
				/*
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_GETAVAILABLEMODULE_FAILED,
					1, "Streamer");
				std:: cout << (const char *) err << std:: endl;

				return 1;
				*/
			//}
			//else
			//{
				// std:: cout << "Streamer AvailableModule. IPAddress: "
				// 	<< pIPAddress << ", Port: " << ulPort << std:: endl;
			//}

			if ((errGetAvailModule = lbLoadBalancer. getAvailableModule (
				"Download", pIPAddress, SCK_MAXIPADDRESSLENGTH, &ulPort)) !=
				errNoError)
			{
				std:: cout << (const char *) errGetAvailModule << std:: endl;
				/*
				Error err = LoadBalancerErrors (__FILE__, __LINE__,
					LB_LOADBALANCER_GETAVAILABLEMODULE_FAILED,
					1, "Download");
				std:: cout << (const char *) err << std:: endl;

				return 1;
				*/
			}
			else
			{
				// std:: cout << "Download AvailableModule. IPAddress: "
				// 	<< pIPAddress << ", Port: " << ulPort << std:: endl;
			}
		}

		if (lbLoadBalancer. finish () != errNoError)
		{
			Error err = LoadBalancerErrors (__FILE__, __LINE__,
				LB_LOADBALANCER_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}

		if (cfConfiguration. finish () != errNoError)
		{
			Error err = ConfigurationFileErrors (__FILE__, __LINE__,
				CFGFILE_CONFIGURATIONFILE_FINISH_FAILED);
			std:: cout << (const char *) err << std:: endl;

			return 1;
		}
	}

	if (tTracer. cancel () != errNoError)
	{
		std:: cout << "Tracer. cancel failed" << std:: endl;

		return 1;
	}

	if (tTracer. finish (true) != errNoError)
	{
		std:: cout << "Tracer. finish failed" << std:: endl;

		return 1;
	}


	return 0;
}

