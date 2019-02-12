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

#include "ProcessUtility.h"
#include "stdlib.h"
#include <vector>

using namespace std;

int main (int iArgc, char *pArgv [])

{

	int		iReturnedStatus;

	try
	{
		vector<string> argList;
		argList.push_back("script.csh");
		argList.push_back("param1=value1");
		argList.push_back("param2=value2");

		string programPath = "/app/7/DevelopmentWorkingArea/CatraLibraries/examples/Tools/forkAndExec/script.csh";

		pid_t	childPid;

		ProcessUtility::forkAndExec (programPath, argList, &childPid, &iReturnedStatus);

		cout << "Returned status: " << iReturnedStatus << endl;
	}
	catch(runtime_error e)
	{
		cerr << "Runtime_error: " << e.what() << endl;
	}



	return 0;
}

