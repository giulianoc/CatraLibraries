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


#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdexcept>
#ifdef WIN32
	#include <process.h>
	#include <Winsock2.h>
#else
	#include <unistd.h>
	#include <sys/vfs.h>
	#include <sys/utsname.h>
#endif
#include "StringTokenizer.h"
#include "ProcessUtility.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
// #ifdef __QTCOMPILER__
	#include <sys/wait.h>
	#include <signal.h>
// #endif




ProcessUtility:: ProcessUtility (void)

{

}


ProcessUtility:: ~ProcessUtility (void)

{

}



ProcessUtility:: ProcessUtility (const ProcessUtility &)

{

	assert (1==0);

	// to do

}


ProcessUtility &ProcessUtility:: operator = (const ProcessUtility &)

{

	assert (1==0);

	// to do

	return *this;

}


Error ProcessUtility:: getCurrentProcessIdentifier (long *plProcessIdentifier)

{

	if (plProcessIdentifier == (long *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	#ifdef WIN32
		*plProcessIdentifier			= _getpid ();
	#else
		*plProcessIdentifier			= getpid ();
	#endif

	return errNoError;
}

long ProcessUtility:: getCurrentProcessIdentifier ()
{
	#ifdef WIN32
		return _getpid ();
	#else
		return getpid ();
	#endif    
}

Error ProcessUtility:: execute (const char *pCommand,
	int *piReturnedStatus)

{

	int				iLocalStatus;


	if (pCommand == (const char *) NULL ||
		piReturnedStatus == (int *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if ((iLocalStatus = system (pCommand)) == -1)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SYSTEM_FAILED);

		return err;
	}

	#ifdef WIN32
		*piReturnedStatus			= iLocalStatus;
	#else
		if (!WIFEXITED(iLocalStatus))
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_PROCESSUTILITY_PROCESSDOESNOTEXITNORMALLY,
				1, pCommand);

			return err;
		}

		*piReturnedStatus			= WEXITSTATUS(iLocalStatus);
	#endif


	return errNoError;
}

int ProcessUtility:: execute (string command)
{
    int returnedStatus;
    
    Error errProcess;
    
    if ((errProcess = ProcessUtility::execute (command.c_str(), &returnedStatus)) != errNoError)
    {
        throw runtime_error(string("ProcessUtility::execute failed: ")
                + (const char *) errProcess);
    }
    
    return returnedStatus;
}

void ProcessUtility::forkAndExec (
		string programPath,
		// first string is the program name, than we have the params
		vector<string>& argList,
		string redirectionPathName,
		bool redirectionStdOutput,
		bool redirectionStdError,
		pid_t* pPid,
		int *piReturnedStatus
		)
{

	/* Duplicate this process. */
	pid_t childPid = fork();
	if (childPid == -1)
	{
		// fork failed
		string errorMessage = string("Fork failed. errno: ") + to_string(errno);

		throw runtime_error(errorMessage);
	}

	if (childPid != 0)
	{
		// parent process
		// Status information about the child reported by wait is more than just the exit status of the child, it also includes
		// - normal/abnormal termination
		//		WIFEXITED(status): child exited normally
		//		WEXITSTATUS(status): return code when child exits
		// - termination cause
		//		WIFSIGNALED(status): child exited because a signal was not caught
		//		WTERMSIG(status): gives the number of the terminating signal
		// - exit status
		//		WIFSTOPPED(status): child is stopped
		//		WSTOPSIG(status): gives the number of the stop signal
		// if we want to prints information about a signal
		//	void psignal(unsigned sig, const char *s);

		*pPid = childPid;


		int wstatus;
		// pid_t childPid = wait(piReturnedStatus);
		wait(&wstatus);

		if (WIFEXITED(wstatus))
			*piReturnedStatus = WEXITSTATUS(wstatus);
		else if (WIFSIGNALED(wstatus))
		{
			string errorMessage = string("Child has exit abnormally. Terminating signal: ") + to_string(WTERMSIG(wstatus));

			throw runtime_error(errorMessage);
		}
		else if (WIFSTOPPED(wstatus))
		{
			string errorMessage = string("Child has exit abnormally. Stop signal: ") + to_string(WSTOPSIG(wstatus));

			throw runtime_error(errorMessage);
		}
	}
	else
	{
		/*
		char** argListParam = new char*[argList.size() + 1];
		for (int paramIndex = 0; paramIndex < argList.size(); paramIndex++)
		{
			// cout << argList[paramIndex] << endl;

			argListParam[paramIndex] = new char [argList[paramIndex].size() + 1];
			strcpy (argListParam[paramIndex], argList[paramIndex].c_str());
		}
		argListParam[argList.size()] = NULL;
		*/

		vector<char*> commandVector;
		for (int paramIndex = 0; paramIndex < argList.size(); paramIndex++)
		{
			commandVector.push_back(const_cast<char*>(argList[paramIndex].c_str()));
		}
		commandVector.push_back(NULL);

		if (redirectionPathName != "" &&
				(redirectionStdOutput || redirectionStdError)
				)
		{
			int fd = open(redirectionPathName.c_str(),
					O_WRONLY | O_TRUNC | O_CREAT,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if (fd == -1)
			{
				string errorMessage = string("Redirection file Open failed: ") + redirectionPathName;

				throw runtime_error(errorMessage);
			}

			// redirect out, copy the file descriptor fd into standard output/error
			if (redirectionStdOutput)
			{
				close(STDOUT_FILENO);
				dup2(fd, STDOUT_FILENO); 
			}
			if (redirectionStdError)
			{
				close(STDERR_FILENO);
				dup2(fd, STDERR_FILENO); 
			}

			// close (fd); // close the file descriptor as we don't need it more
		}

		// child process: execute the command
		execv(programPath.c_str(),  &commandVector[0]);
		// execv(programPath.c_str(),  argListParam);

		/*
		for (int paramIndex = 0; paramIndex < argList.size(); paramIndex++)
			delete argListParam[paramIndex];
		delete [] argListParam;
		*/

		/* The execv  function returns only if an error occurs.  */
		string errorMessage = string("An error occurred in execv. errno: ") + to_string(errno);

		throw runtime_error(errorMessage);
	}
}


void ProcessUtility::killProcess (pid_t pid)
{
	if(kill(pid, SIGTERM) == -1)
	{
		string errorMessage = string("kill failed. errno: ") + to_string(errno);

		throw runtime_error(errorMessage);
	}
}


#ifdef WIN32
#else
Error ProcessUtility:: setUserAndGroupID (const char *pUserName)

{

	char						*pUserNameToSearch;
	Buffer_t					bPasswdFile;
	StringTokenizer_t			stPasswdTokenizer;
	StringTokenizer_t			stUserNameTokenizer;
	Error						errRead;
	Error						errNextToken;
	const char					*pToken;
	unsigned long				ulUserNameLength;
	uid_t						uUserID;
	gid_t						gGroupID;
	Boolean_t					bUserNameFound;


	if (pUserName == (const char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	if (bPasswdFile. init () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_INIT_FAILED);

		return err;
	}

	if ((errRead = bPasswdFile. readBufferFromFile ("/etc/passwd")) !=
		errNoError)
	{
		// Error err = ToolsErrors (__FILE__, __LINE__,
		// 	TOOLS_BUFFER_READBUFFERFROMFILE_FAILED);

		if (bPasswdFile. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return errRead;
	}

	if (stPasswdTokenizer. init ((const char *) bPasswdFile, -1,
		"\n") != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_INIT_FAILED);

		if (bPasswdFile. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	ulUserNameLength			= strlen (pUserName);

	if ((pUserNameToSearch = new char [ulUserNameLength + 1 + 1]) ==
		(char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_NEW_FAILED);

		if (stPasswdTokenizer. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_STRINGTOKENIZER_FINISH_FAILED);
		}

		if (bPasswdFile. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	strcpy (pUserNameToSearch, pUserName);
	strcat (pUserNameToSearch, ":");

	uUserID					= 0;
	gGroupID				= 0;
	bUserNameFound			= false;

	do
	{
		if ((errNextToken = stPasswdTokenizer. nextToken (&pToken)) !=
			errNoError)
		{
			if ((long) errNextToken == TOOLS_STRINGTOKENIZER_NOMORETOKEN)
				continue;
			else
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

				delete [] pUserNameToSearch;

				if (stPasswdTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				if (bPasswdFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}
		}

		if (strlen (pToken) < ulUserNameLength + 1)
			continue;

		if (!strncmp (pToken, pUserNameToSearch, ulUserNameLength + 1))
		{
			if (stUserNameTokenizer. init (pToken, -1, ":") != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_INIT_FAILED);

				delete [] pUserNameToSearch;

				if (stPasswdTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				if (bPasswdFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}

			// user name
			if (stUserNameTokenizer. nextToken (&pToken) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

				if (stUserNameTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				delete [] pUserNameToSearch;

				if (stPasswdTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				if (bPasswdFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}

			// ???
			if (stUserNameTokenizer. nextToken (&pToken) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

				if (stUserNameTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				delete [] pUserNameToSearch;

				if (stPasswdTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				if (bPasswdFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}

			// user ID
			if (stUserNameTokenizer. nextToken (&pToken) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

				if (stUserNameTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				delete [] pUserNameToSearch;

				if (stPasswdTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				if (bPasswdFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}

			uUserID			= atol (pToken);

			// group ID
			if (stUserNameTokenizer. nextToken (&pToken) != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED);

				if (stUserNameTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				delete [] pUserNameToSearch;

				if (stPasswdTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				if (bPasswdFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}

			gGroupID			= atol (pToken);

			if (stUserNameTokenizer. finish () != errNoError)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_STRINGTOKENIZER_FINISH_FAILED);

				delete [] pUserNameToSearch;

				if (stPasswdTokenizer. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_STRINGTOKENIZER_FINISH_FAILED);
				}

				if (bPasswdFile. finish () != errNoError)
				{
					Error err = ToolsErrors (__FILE__, __LINE__,
						TOOLS_BUFFER_FINISH_FAILED);
				}

				return err;
			}

			bUserNameFound			= true;

			break;
		}
	}
	while (errNextToken == errNoError);

	delete [] pUserNameToSearch;

	if (stPasswdTokenizer. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_STRINGTOKENIZER_FINISH_FAILED);

		if (bPasswdFile. finish () != errNoError)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_BUFFER_FINISH_FAILED);
		}

		return err;
	}

	if (bPasswdFile. finish () != errNoError)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_BUFFER_FINISH_FAILED);

		return err;
	}

	if (!bUserNameFound)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_PROCESSUTILITY_USERNAMENOTFOUND,
			1, pUserName);

		return err;
	}

	// it is important the order, first setgid and after setuid
	if (setgid (gGroupID) == -1)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SETGID_FAILED,
			2, errno, (long) gGroupID);

		return err;
	}

	if (setuid (uUserID) == -1)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_SETUID_FAILED,
			2, errno, (long) uUserID);

		return err;
	}


	return errNoError;
}
#endif


/*
Error ProcessUtility:: getProcessorsNumber (unsigned long *pulProcessorsNumber)

{

	#ifdef WIN32
		SYSTEM_INFO				theSystemInfo;
		::GetSystemInfo (&theSystemInfo);

		*pulProcessorsNumber		=
			(unsigned long) theSystemInfo. dwNumberOfProcessors;

		return errNoError;
	#endif

	#if (__MacOSX__ || __FreeBSD__)
    	int numCPUs = 1;
    	size_t len = sizeof(numCPUs);
		int mib[2];
    	mib[0] = CTL_HW;
    	mib[1] = HW_NCPU;
    	(void) ::sysctl(mib,2,&numCPUs,&len,NULL,0);
    	if (numCPUs < 1) 
        	numCPUs = 1;
    	*pulProcessorsNumber			=
			(unsigned long) numCPUs;

    	return errNoError;
	#endif

	#if(__linux__ || __linuxppc__)
    	char cpuBuffer[8192] = "";
    	StrPtrLen cpuInfoBuf(cpuBuffer, sizeof(cpuBuffer));
    	FILE    *cpuFile = ::fopen( "/proc/cpuinfo", "r" );
    	if (cpuFile)
    	{
			cpuInfoBuf.Len = ::fread(cpuInfoBuf.Ptr, sizeof(char),
				cpuInfoBuf.Len, cpuFile);
			::fclose(cpuFile);
    	}

		StringParser cpuInfoFileParser(&cpuInfoBuf);
		StrPtrLen line;
		StrPtrLen word;
		UInt32 numCPUs = 0;
    
    	while (cpuInfoFileParser. GetDataRemaining() != 0) 
    	{
			cpuInfoFileParser.GetThruEOL(&line);    // Read each line   
			StringParser lineParser(&line);
			lineParser.ConsumeWhitespace();         //skip over leading whitespace

			if (lineParser.GetDataRemaining() == 0) // must be an empty line
				continue;

			lineParser.ConsumeUntilWhitespace(&word);

			if ( word.Equal("processor") ) // found a processor as first word in line
        	{
				numCPUs ++; 
			}
		}

		if (numCPUs == 0)
			numCPUs = 1;

    	*pulProcessorsNumber			= numCPUs;

    	return errNoError;
	#endif

	#if(__solaris__)
	{
    	UInt32 numCPUs = 0;
    	char linebuff[512] = "";
    	StrPtrLen line(linebuff, sizeof(linebuff));
    	StrPtrLen word;

    	FILE *p = ::popen("uname -X","r");
    	while((::fgets(linebuff, sizeof(linebuff -1), p)) > 0)
    	{
        	StringParser lineParser(&line);
        	lineParser.ConsumeWhitespace(); //skip over leading whitespace

        	if (lineParser.GetDataRemaining() == 0) // must be an empty line
            	continue;

        	lineParser.ConsumeUntilWhitespace(&word);

        	if ( word.Equal("NumCPU")) // found a tag as first word in line
        	{
            	lineParser.GetThru(NULL,'=');
            	lineParser.ConsumeWhitespace();  //skip over leading whitespace
            	lineParser.ConsumeUntilWhitespace(&word); //read the number of cpus
            	if (word.Len > 0)
                	::sscanf(word.Ptr, "%lu", &numCPUs);

            	break;
        	}
    	}
    	if (numCPUs == 0)
        	numCPUs = 1;

		::pclose(p);

    	*pulProcessorsNumber			= numCPUs;

    	return errNoError;
	}
	#endif

	#if(__sgi__) 
    	UInt32 numCPUs = 0;

    	numCPUs = sysconf(_SC_NPROC_ONLN);
	
    	*pulProcessorsNumber			= numCPUs;

    	return errNoError;
	#endif		


    return errNoError;
}

*/

