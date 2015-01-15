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


#include "System.h"
#ifdef WIN32
	// #include <process.h>
	#include <Winsock2.h>
	#include <UserEnv.h>
#else
	#include <unistd.h>
	#include <sys/utsname.h>
#endif
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>


System:: System (void)

{

}


System:: ~System (void)

{

}



System:: System (const System &)

{

	assert (1==0);

	// to do

}


System &System:: operator = (const System &)

{

	assert (1==0);

	// to do

	return *this;

}


/*
Copiati da Xenon
BOOL GetOSVersion(char *pOS)
{
#ifdef WIN32
	char *platform=NULL, *type=NULL,ver[80];
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;
	ver[0] = 0;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	//
	// If that fails, try using the OSVERSIONINFO structure.
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( (bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) != 0 )
	{
	  // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.

	  osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	  if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
	  {
		  return mlmGetOSVersionOld(pOS);
	  }
	}

	switch (osvi.dwPlatformId)
	{
	  case VER_PLATFORM_WIN32_NT:

	  // Test for the product.

		 if ( osvi.dwMajorVersion <= 4 )
			 platform = "Windows NT";
		 else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
			 platform = "Windows 2000";
		 else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
			 platform = "Windows XP";
		 else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
			 platform = "Windows 2003";
		 else 
			 platform = "Windows 2003 Future";

	  // Test for product type.
#ifndef _WIN32_WCE
		 if( bOsVersionInfoEx )
		 {
			if ( osvi.wProductType == VER_NT_WORKSTATION )
			{
	// For Wisler Edition
//			   if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
//				  type = "Personal ";
//			   else
				  type = "Professional ";
			}
			else if ( osvi.wProductType == VER_NT_SERVER )
			{
			   if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
				  type = "DataCenter";
			   else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
				  type = "Enterprise";
			   else
				  type = "Server ";
			}
			else if ( osvi.wProductType == VER_NT_DOMAIN_CONTROLLER )
			{
				type = "Domain Controller";
			}
		 }
		 else
#endif //_WIN32_WCE
		 {
			HKEY hKey;
			char szProductType[80];
			DWORD dwBufLen;

			RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			   "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
			   0, KEY_QUERY_VALUE, &hKey );
			RegQueryValueEx( hKey, "ProductType", NULL, NULL,
			   (LPBYTE) szProductType, &dwBufLen);
			RegCloseKey( hKey );
			if ( lstrcmpi( "WINNT", szProductType) == 0 )
			   type = "Workstation ";
			if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
			   type = "Server ";
		 }

	  // Display version, service pack (if any), and build number.

		 if ( osvi.dwMajorVersion <= 4 )
		 {
			sprintf (ver,"version %d.%d %s Build %d",
			   osvi.dwMajorVersion,
			   osvi.dwMinorVersion,
			   osvi.szCSDVersion,
			   osvi.dwBuildNumber & 0xFFFF);
		 }
		 else
		 { 
			sprintf (ver, "%s Build %d",
			   osvi.szCSDVersion,
			   osvi.dwBuildNumber & 0xFFFF);
		 }

		 //sprintf(retstr,"%s%s%s",platform,type,ver);
		 sprintf(pOS,"%s %s",platform,type);
		 //sprintf(retstr,"%s",platform);
		 break;

	  case VER_PLATFORM_WIN32_WINDOWS:

		 if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		 {
			 platform = "Microsoft Windows 95";
			 if ( osvi.szCSDVersion[1] == 'C' )
				type = "OSR2 ";
		 } 

		 if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		 {
			 platform = "Microsoft Windows 98";
			 if ( osvi.szCSDVersion[1] == 'A' )
				type = "SE ";
		 } 

		 if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		 {
			 platform = "Microsoft Windows Me";
			 type = "";
		 } 

		 //sprintf(retstr,"%s%s",platform,type);
		 sprintf(pOS,"%s",platform);
		 break;

	  case VER_PLATFORM_WIN32s:

		 platform = "Microsoft Win32s";
		 sprintf(pOS,"%s",platform);
		 break;
#ifdef _WIN32_WCE
	  case VER_PLATFORM_WIN32_CE:
		 platform = "Microsoft WinCE";
	  	  // Display version, service pack (if any), and build number.
		 sprintf (ver,"version %d.%d %s (Build %d)",
			   osvi.dwMajorVersion,
			   osvi.dwMinorVersion,
			   osvi.szCSDVersion,
			   osvi.dwBuildNumber & 0xFFFF);

		 sprintf(pOS,"%s%s",platform,ver);
		 break;
#endif // _WIN32_WCE
	  default:
		sprintf(pOS,"Unknown Platform");
	}

#else //Solaris & Linux all
	struct utsname sname;
	if(uname(&sname) < 0)
	{
		sprintf(pOS,"Unknown Platform");
		return FALSE;
	}
	
	// printf("sysname - (%s)\n", sname.sysname);
	// printf("nodename - (%s)\n", sname.nodename);
	// printf("release - (%s)\n", sname.release);
	// printf("version - (%s)\n", sname.version);
	// printf("machine - (%s)\n", sname.machine);
	
	sprintf(pOS,"%s %s %s", sname.sysname,sname.release, sname.machine);
#endif
	return TRUE;

}


BOOL GetOSVersionOld(char *pOS)
{
#ifdef WIN32
	DWORD dwWindowsMajorVersion, dwWindowsMinorVersion;
	DWORD dwBuild, dwVersion;

	dwVersion = GetVersion();
 
	// Get the Windows version.

	dwWindowsMajorVersion =  (DWORD)(LOBYTE(LOWORD(dwVersion)));
	dwWindowsMinorVersion =  (DWORD)(HIBYTE(LOWORD(dwVersion)));

	// Get the build number.

	if (dwVersion < 0x80000000)              // Windows NT/2000, Whistler
	{
		if (dwWindowsMajorVersion < 2)      // Error
		{
			dwBuild = (DWORD)(HIWORD(dwVersion));
			strcpy(pOS,"Windows_NT_3.51_below");
		}
		else if (dwWindowsMajorVersion == 3)      // Windows NT 3.51
		{
			dwBuild = (DWORD)(HIWORD(dwVersion));
			strcpy(pOS,"Windows_NT_3.51");
		}
		else if (dwWindowsMajorVersion == 4)      // Windows NT
		{
			dwBuild = (DWORD)(HIWORD(dwVersion));
			strcpy(pOS,"Windows_NT_4.0");
		}
		else if (dwWindowsMajorVersion == 5)      // Windows 2000/Whisler
		{
			dwBuild = (DWORD)(HIWORD(dwVersion));
			if (dwWindowsMinorVersion == 0 )
				strcpy(pOS,"Windows_2000");
			else if (dwWindowsMinorVersion == 1 )
				strcpy(pOS,"Windows_XP");
			else if (dwWindowsMinorVersion == 2 )
				strcpy(pOS,"Windows_2003");
			else
				strcpy(pOS,"Windows_2003_Future_Version");
		}
		else
		{
			dwBuild = (DWORD)(HIWORD(dwVersion));
			strcpy(pOS,"Windows_2000_Future");
		}
	}
	else 
	{
		if (dwWindowsMajorVersion < 4)      // Win32s
		{
			dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
			strcpy(pOS,"Windows_3.1_Win32s");
		}
		else                                     // Windows 95/98/Me
		{
			dwBuild =  0;
			strcpy(pOS,"Windows_95_98_Me");
		}
	}
#endif
	return TRUE;
}

char *GetCPUInfo()
{
	char *retstr;
#ifdef WIN32

	SYSTEM_INFO si;

	GetSystemInfo(&si);

	switch (si.dwProcessorType)
	{
	case PROCESSOR_INTEL_386 :
		retstr = "INTEL_386";
		break;
	case PROCESSOR_INTEL_486 :
		retstr = "INTEL_486";
		break;
	case PROCESSOR_INTEL_PENTIUM :
		retstr = "INTEL_PENTIUM";
		break;
#ifdef _WIN32_WCE
//	case PROCESSOR_MIPS_R3000 :
//		retstr = "MIPS_R3000";
//		break;
	case PROCESSOR_MIPS_R4000 :
		retstr = "MIPS_R4000";
		break;
	case PROCESSOR_HITACHI_SH3 :
		retstr = "HITACHI_SH3";
		break;
	case PROCESSOR_HITACHI_SH4 :
		retstr = "HITACHI_SH4";
		break;
	case PROCESSOR_PPC_403 :
		retstr = "PPC_403";
		break;
//	case PROCESSOR_PPC_821 :
//		retstr = "PPC_821";
//		break;
	case PROCESSOR_STRONGARM :
		retstr = "STRONGARM";
		break;
	case PROCESSOR_ARM720 :
		retstr = "ARM720";
		break;
#endif //_WIN32_WCE
	default:
		retstr = "UNKNOWN";
	}
#elif defined(SOLARIS)
	char genericBuffer[1024] ;
	memset( genericBuffer, 0, sizeof(genericBuffer) );
	
	if(sysinfo(SI_ARCHITECTURE, genericBuffer, 1024) < 0)
	{
		retstr = "ERROR";
	}

	retstr = genericBuffer;

#elif defined(LINUX)
    struct utsname sname;
    if(uname(&sname) < 0)
    {
        sprintf(retstr,"Unknown Platform");
        return FALSE;
    }

    // printf("sysname - (%s)\n", sname.sysname);
    // printf("nodename - (%s)\n", sname.nodename);
    // printf("release - (%s)\n", sname.release);
    // printf("version - (%s)\n", sname.version);
    // printf("machine - (%s)\n", sname.machine);

    retstr = sname.machine;

#endif
	return retstr;
}

int GetProcessorNum()
{
#ifdef WIN32
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);
	return SystemInfo.dwNumberOfProcessors;
#elif LINUX
	return get_nprocs();
#elif defined(SOLARIS)
	long nProcessorOnline 		= sysconf(_SC_NPROCESSORS_ONLN);
	return nProcessorOnline;
#else
	return 0;
#endif
}

BOOL GetLocalHostName(char *name)
{
	bool bRet = TRUE;
#ifdef WIN32
	DWORD dwLevel = 102;
	LPWKSTA_INFO_102 pBuf = NULL;
	NET_API_STATUS nStatus;
//	LPTSTR pszServerName = NULL;
//	char tmp[256];
	// Call the NetWkstaGetInfo function, specifying level 102.
	//
	nStatus = NetWkstaGetInfo(NULL,
							 dwLevel,
							 (LPBYTE *)&pBuf);
	//
	// If the call is successful,
	//  print the workstation data.
	//
	if (nStatus == NERR_Success)
	{
	//      TRACE(_T("\n\tPlatform: %d\n"), pBuf->wki102_platform_id);
	  //wctomb(tmp,pBuf->wki102_computername);
	 // mbstowcs(tmp,(char*)pBuf->wki102_computername,256);
	//	  TRACE(_T("\tName:     %ls\n"), pBuf->wki102_computername);
	  sprintf(name,_T("%s"), pBuf->wki102_computername);
	//      TRACE(_T("\tVersion:  %d.%d\n"), pBuf->wki102_ver_major,
	//                                  pBuf->wki102_ver_minor);
	//      TRACE(_T("\tDomain:   %ls\n"), pBuf->wki102_langroup);
	//      TRACE(_T("\tLan Root: %ls\n"), pBuf->wki102_lanroot);
	//      TRACE(_T("\t# Logged On Users: %d\n"), pBuf->wki102_logged_on_users);
	}
	//
	// Otherwise, indicate the system error.
	//
	else
	{
		//fprintf(stderr, "A system error has occurred: %d\n", nStatus);
		sprintf(name, "UNKNOWN(A system error has occurred: %d)\n", nStatus);
		//strcpy(name,"UNKNOWN(");
		bRet = FALSE;
	}

	//
	// Free the allocated memory.
	//
	if (pBuf != NULL)
		NetApiBufferFree(pBuf);
#else
	struct utsname sname;
	if(uname(&sname) < 0)
	{
		sprintf(name,"Unknown hostname");
		return FALSE;
	}
	
	sprintf(name,"%s", sname.nodename);
#endif

	return bRet;
}
*/

Error System:: getHostName (
	char *pHostName,
	unsigned long ulHostNameBufferLength)

{

	if (pHostName == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	// host name initialization
	#ifdef WIN32
		WSADATA			wsaData;

		if (WSAStartup (MAKEWORD (2, 2), &wsaData))
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_WSASTARTUP_FAILED);

			return err;
		}

		if (gethostname (pHostName, ulHostNameBufferLength) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETHOSTNAME_FAILED);

			if (WSACleanup ())
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_WSACLEANUP_FAILED);
			}

			return err;
		}

		if (WSACleanup ())
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_WSACLEANUP_FAILED);

			return err;
		}
	#else
		struct utsname		unUtsname;

		if (uname (&unUtsname) == -1)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_UNAME_FAILED);

			return err;
		}
		if (strlen (unUtsname. nodename) >= ulHostNameBufferLength)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SYSTEM_BUFFERTOOSMALL);

			return err;
		}

		strcpy (pHostName, unUtsname. nodename);
	#endif


	return errNoError;
}


Error System:: getHomeDirectory (
	char *pHomeDirectory,
	unsigned long ulBufferLength)

{

	if (pHomeDirectory == (char *) NULL)
	{
		Error err = ToolsErrors (__FILE__, __LINE__,
			TOOLS_ACTIVATION_WRONG);

		return err;
	}

	// host name initialization
	#ifdef WIN32
	{
		HANDLE			hToken;
		TCHAR			szHomeDirBuf [MAX_PATH] = { 0 };
		DWORD			dwBufSize;

		hToken          = 0;
		dwBufSize       = MAX_PATH;

		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == 0)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_OPENPROCESSTOKEN_FAILED,
				1, (long) GetLastError ());

			return err;
		}

		if (GetUserProfileDirectory(hToken, szHomeDirBuf, &dwBufSize) == FALSE)
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_GETUSERPROFILEDIRECTORY_FAILED,
				1, (long) GetLastError ());

			return err;
		}

		CloseHandle (hToken);

		// TCHAR is a Microsoft-specific typedef for either
		// char or wchar_t (a wide character).
		// Conversion to char depends on which of these it actually is.
		// If TCHAR is actually a char, then you can do a simple cast,
		// but if it is truly a wchar_t, you'll need a routine
		// to convert between character sets.
		if (sizeof(TCHAR) != sizeof(wchar_t))
		{
			strcpy (pHomeDirectory, (const char *) szHomeDirBuf);
		}
		else
		{
			if (WideCharToMultiByte (CP_ACP, 0, (WCHAR *) szHomeDirBuf, -1,
				pHomeDirectory, ulBufferLength, NULL, NULL) ==
				0)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_WIDECHARTOMULTIBYTE_FAILED,
					1, (long) GetLastError ());

				return err;
			}
		}
	}
	#else
	{
		const char			*pHome;

		pHome			= getenv ("HOME");

		if (pHome != (const char *) NULL)
		{
			if (strlen (pHome) >= ulBufferLength)
			{
				Error err = ToolsErrors (__FILE__, __LINE__,
					TOOLS_SYSTEM_BUFFERTOOSMALL);

				return err;
			}

			strcpy (pHomeDirectory, pHome);
		}
		else
		{
			Error err = ToolsErrors (__FILE__, __LINE__,
				TOOLS_SYSTEM_ENVIRONMENTVARIABLENOTDEFINED,
				1, "HOME");

			return err;
		}
	}
	#endif


	return errNoError;
}

