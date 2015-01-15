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


#include "ToolsErrors.h"


ErrMsgBase:: ErrMsgsInfo ToolsErrorsStr = {

	// Buffer
	{ TOOLS_BUFFER_INIT_FAILED,
		"The init function of the Buffer class failed" },
	{ TOOLS_BUFFER_FINISH_FAILED,
		"The finish function of the Buffer class failed" },
	{ TOOLS_BUFFER_GETBUFFER_FAILED,
		"The getBuffer function of the Buffer class failed" },
	{ TOOLS_BUFFER_GETBUFFERLENGTH_FAILED,
		"The getBufferLength function of the Buffer class failed" },
	{ TOOLS_BUFFER_SETBUFFER_FAILED,
		"The setBuffer function of the Buffer class failed" },
	{ TOOLS_BUFFER_READBUFFERFROMFILE_FAILED,
		"The readBufferFromFile function of the Buffer class failed, PathName: %s" },
	{ TOOLS_BUFFER_WRITEBUFFERONFILE_FAILED,
		"The writeBufferOnFile function of the Buffer class failed" },
	{ TOOLS_BUFFER_STARTWITH_FAILED,
		"The startWith function of the Buffer class failed" },
	{ TOOLS_BUFFER_ISEQUALWITH_FAILED,
		"The isEqualWith function of the Buffer class failed" },
	{ TOOLS_BUFFER_APPEND_FAILED,
		"The append function of the Buffer class failed" },
	{ TOOLS_BUFFER_INSERTAT_FAILED,
		"The insertAt function of the Buffer class failed" },
	{ TOOLS_BUFFER_ALLOCMEMORYBLOCKFORBUFFERIFNECESSARY_FAILED,
		"The allocMemoryBlockForBufferIfNecessary function failed" },
	{ TOOLS_BUFFER_SUBSTITUTE_FAILED,
		"The substitute function of the Buffer class failed" },
	{ TOOLS_BUFFER_REMOVECTRLCHARACTERS_FAILED,
		"The removeCTRLCharacters function of the Buffer class failed" },
	{ TOOLS_BUFFER_REMOVECHAR_FAILED,
		"The removeChar function of the Buffer class failed" },
	{ TOOLS_BUFFER_TRUNCATESTARTINGFROM_FAILED,
		"The truncateStartingFrom function of the Buffer class failed" },
	{ TOOLS_BUFFER_TRUNCATEIFBIGGER_FAILED,
		"The truncateIfBigger function of the Buffer class failed" },
	{ TOOLS_BUFFER_SETCHAR_FAILED,
		"The setChar function of the Buffer class failed" },
	{ TOOLS_BUFFER_STRIP_FAILED,
		"The strip function of the Buffer class failed" },
	{ TOOLS_BUFFER_GETCHAR_FAILED,
		"The getChar function of the Buffer class failed" },
	{ TOOLS_BUFFER_CONVERSIONFROMCHARTOWCHAR_FAILED,
		"The conversionFromCharToWCHAR function of the Buffer class failed" },
	{ TOOLS_BUFFER_INDEXOUTOFRANGE,
		"The index is out of range" },
	{ TOOLS_BUFFER_CHARNOTFOUND,
		"The char is not found. Char: %c, Buffer: %s" },

	// DateTime
	{ TOOLS_DATETIME_NOWUTCINMILLISECS_FAILED,
		"The nowUTCInMilliSecs function of the DateTime class failed" },
	{ TOOLS_DATETIME_NOWLOCALINMILLISECS_FAILED,
		"The nowLocalInMilliSecs function of the DateTime class failed" },
	{ TOOLS_DATETIME_NOWLOCALTIME_FAILED,
		"The nowLocalTime function of the DateTime class failed" },
	{ TOOLS_DATETIME_GETTIMEZONEINFORMATION_FAILED,
		"The getTimeZoneInformation function of the DateTime class failed" },
	{ TOOLS_DATETIME_GET_TM_LOCALTIME_FAILED,
		"The get_tm_localtime function of the DateTime class failed" },
	{ TOOLS_DATETIME_CONVERTFROMLOCALTOUTC_FAILED,
		"The convertFromLocalToUTC function of the DateTime class failed" },
	{ TOOLS_DATETIME_CONVERTFROMUTCTOLOCAL_FAILED,
		"The convertFromUTCToLocal function of the DateTime class failed" },
	{ TOOLS_DATETIME_CONVERTFROMLOCALDATETIMETOLOCALINSECS_FAILED,
		"The convertFromLocalDateTimeToLocalInSecs function of the DateTime class failed" },
	{ TOOLS_DATETIME_ADDSECONDS_FAILED,
		"The addSeconds function of the DateTime class failed" },
	{ TOOLS_DATETIME_ISLEAPYEAR_FAILED,
		"The isLeapYear function of the DateTime class failed" },
	{ TOOLS_DATETIME_GETLASTDAYOFMONTH_FAILED,
		"The getLastDayOfMonth function of the DateTime class failed" },
	{ TOOLS_DATETIME_CONVERTFROMUTCINSECONDSTOBREAKDOWNUTC_FAILED,
		"The convertFromUTCInSecondsToBreakDownUTC function of the DateTime class failed" },

	// StringTokenizer
	{ TOOLS_STRINGTOKENIZER_INIT_FAILED,
		"The init function of the StringTokenizer class failed" },
	{ TOOLS_STRINGTOKENIZER_FINISH_FAILED,
		"The finish function of the StringTokenizer class failed" },
	{ TOOLS_STRINGTOKENIZER_NEXTTOKEN_FAILED,
		"The nextToken function of the StringTokenizer class failed. Buffer: %s" },
	{ TOOLS_STRINGTOKENIZER_COUNTTOKENS_FAILED,
		"The countTokens function of the StringTokenizer class failed" },
	{ TOOLS_STRINGTOKENIZER_NOMORETOKEN,
		"No more token" },

	// FileIO
	{ TOOLS_FILEIO_GETFILENAMEFROMPATHFILENAME_FAILED,
		"The getFileFromPathFileName method of the FileIO class failed." },
	{ TOOLS_FILEIO_REPLACEUNSAFEFILENAMECHARS_FAILED,
		"The replaceUnsafeFileNameChars method of the FileIO class failed." },
	{ TOOLS_FILEIO_OPENDIRECTORY_FAILED,
		"The openDirectory method of the FileIO class failed." },
	{ TOOLS_FILEIO_READDIRECTORY_FAILED,
		"The readDirectory method of the FileIO class failed" },
	{ TOOLS_FILEIO_CLOSEDIRECTORY_FAILED,
		"The closeDirectory method of the FileIO class failed" },
	{ TOOLS_FILEIO_GETWORKINGDIRECTORY_FAILED,
		"The getWorkingDirectory method of the FileIO class failed" },
	{ TOOLS_FILEIO_GZIP_FAILED,
		"The gzip method of the FileIO class failed" },
	{ TOOLS_FILEIO_GETDIRECTORYENTRYTYPE_FAILED,
		"The getDirectoryEntryType method of the FileIO class failed" },
	{ TOOLS_FILEIO_CHANGEPERMISSION_FAILED,
		"The changePermission method of the FileIO class failed" },
	{ TOOLS_FILEIO_READLINK_FAILED,
		"The readLink method of the FileIO class failed" },
	{ TOOLS_FILEIO_CREATEDIRECTORY_FAILED,
		"The createDirectory method of the FileIO class failed. (Directory name: %s)" },
	{ TOOLS_FILEIO_REMOVEDIRECTORY_FAILED,
		"The removeDirectory method of the FileIO class failed. Dir path name: %s" },
	{ TOOLS_FILEIO_GETFILETIME_FAILED,
		"The getFileTime method of the FileIO class failed. (File name: %s)" },
	{ TOOLS_FILEIO_GETFILESIZEINBYTES_FAILED,
		"The getFileSizeInBytes method of the FileIO class failed. (File name: %s)" },
	{ TOOLS_FILEIO_ISFILEEXISTING_FAILED,
		"The isFileExisting method of the FileIO class failed. (File name: %s)" },
	{ TOOLS_FILEIO_ISDIRECTORYEXISTING_FAILED,
		"The isDirectoryExisting method of the FileIO class failed. (Directory path: %s)" },
	{ TOOLS_FILEIO_GETFILESYSTEMINFO_FAILED,
		"The getFileSystemInfo function of the FileIO class failed" },
	{ TOOLS_FILEIO_GETDIRECTORYUSAGE_FAILED,
		"The getDirectoryUsage function of the FileIO class failed. (Directory path: %s)" },
	{ TOOLS_FILEIO_GETDIRECTORYSIZEINBYTES_FAILED,
		"The getDirectorySizeInBytes function of the FileIO class failed. (Directory path: %s)" },
	{ TOOLS_FILEIO_COPYFILE_FAILED,
		"The copyFile method of the FileIO class failed. Src: %s, Dst: %s" },
	{ TOOLS_FILEIO_MOVEFILE_FAILED,
		"The moveFile method of the FileIO class failed. Src: %s, Dst: %s" },
	{ TOOLS_FILEIO_MOVELINK_FAILED,
		"The moveLink method of the FileIO class failed. Src: %s, Dst: %s" },
	{ TOOLS_FILEIO_MOVEDIRECTORY_FAILED,
		"The moveDirectory method of the FileIO class failed. Src: %s, Dst: %s" },
	{ TOOLS_FILEIO_COPYDIRECTORY_FAILED,
		"The copyDirectory method of the FileIO class failed. Src: %s, Dst: %s" },
	{ TOOLS_FILEIO_CREATELINK_FAILED,
		"The createLink method of the FileIO class failed. Src: %s, Dst: %s" },
	{ TOOLS_FILEIO_OPEN_FAILED,
		"The open method of the FileIO class failed. (File name: %s)" },
	{ TOOLS_FILEIO_CLOSE_FAILED,
		"The close method of the FileIO class failed" },
	{ TOOLS_FILEIO_TOUCHFILE_FAILED,
		"The touchFile method of the FileIO class failed. (File name: %s)" },
	{ TOOLS_FILEIO_REMOVE_FAILED,
		"The remove method of the FileIO class failed. (File name: %s)" },
	{ TOOLS_FILEIO_SEEK_FAILED,
		"The seek method of the FileIO class failed" },
	{ TOOLS_FILEIO_READCHARS_FAILED,
		"The readChars method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITECHARS_FAILED,
		"The writeChars method of the FileIO class failed" },
	{ TOOLS_FILEIO_READBYTES_FAILED,
		"The readBytes method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITEBYTES_FAILED,
		"The writeBytes method of the FileIO class failed" },
	{ TOOLS_FILEIO_READNETUNSIGNEDINT8BIT_FAILED,
		"The readNetUnsignedInt8Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_READNETUNSIGNEDINT16BIT_FAILED,
		"The readNetUnsignedInt16Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_READNETUNSIGNEDINT24BIT_FAILED,
		"The readNetUnsignedInt24Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_READNETUNSIGNEDINT32BIT_FAILED,
		"The readNetUnsignedInt32Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_READNETUNSIGNEDINT64BIT_FAILED,
		"The readNetUnsignedInt64Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_READNETFLOAT16BIT_FAILED,
		"The readNetFloat16Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_READNETFLOAT32BIT_FAILED,
		"The readNetFloat32Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITENETUNSIGNEDINT8BIT_FAILED,
		"The writeNetUnsignedInt8Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITENETUNSIGNEDINT16BIT_FAILED,
		"The writeNetUnsignedInt16Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITENETUNSIGNEDINT24BIT_FAILED,
		"The writeNetUnsignedInt24Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITENETUNSIGNEDINT32BIT_FAILED,
		"The writeNetUnsignedInt32Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITENETUNSIGNEDINT64BIT_FAILED,
		"The writeNetUnsignedInt64Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITENETFLOAT16BIT_FAILED,
		"The writeNetFloat16Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITENETFLOAT32BIT_FAILED,
		"The writeNetFloat32Bit method of the FileIO class failed" },
	{ TOOLS_FILEIO_READMP4DESCRIPTORSIZE_FAILED,
		"The readMP4DescriptorSize method of the FileIO class failed" },
	{ TOOLS_FILEIO_WRITEMP4DESCRIPTORSIZE_FAILED,
		"The writeMP4DescriptorSize method of the FileIO class failed" },
	{ TOOLS_FILEIO_LOCKFILE_FAILED,
		"The lockFile method of the FileIO class failed. Lock pathname: %s." },
	{ TOOLS_FILEIO_UNLOCKFILE_FAILED,
		"The unLockFile method of the FileIO class failed. Lock pathname: %s." },
	{ TOOLS_FILEIO_APPENDBUFFER_FAILED,
		"The appendBuffer method of the FileIO class failed." },
	{ TOOLS_FILEIO_DIRECTORYFILESFINISHED,
		"Finished the files of the directory" },
	{ TOOLS_FILEIO_FILENOTRELEASED,
		"File not released. Pathname: %s" },
	{ TOOLS_FILEIO_BUFFERTOOSMALL,
		"Buffer too small" },
	{ TOOLS_FILEIO_WRONGSOURCEFILE,
		"Wrong source file. Source file: %s" },
	{ TOOLS_FILEIO_DIRECTORYNOTEXISTING,
		"The %s directory is not existing" },
	{ TOOLS_FILEIO_UNKNOWNFILETYPE,
		"Unknown type of the %s%s file" },

	// File
	{ TOOLS_FILEREADER_INIT_FAILED,
		"The init method of the FileReader class failed. PathName: %s" },
	{ TOOLS_FILEREADER_FINISH_FAILED,
		"The finish method of the FileReader class failed" },
	{ TOOLS_FILEREADER_WRITE_FAILED,
		"The write method of the FileReader class failed" },
	{ TOOLS_FILEREADER_SEEK_FAILED,
		"The seek method of the FileReader class failed" },
	{ TOOLS_FILEREADER_SEEKBYSEARCH_FAILED,
		"The seekBySearch method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READLINE_FAILED,
		"The readLine method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READCHARS_FAILED,
		"The readChars method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READBYTES_FAILED,
		"The readBytes method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READNETUNSIGNEDINT8BIT_FAILED,
		"The readNetUnsignedInt8Bit method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READNETUNSIGNEDINT16BIT_FAILED,
		"The readNetUnsignedInt16Bit method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READNETUNSIGNEDINT24BIT_FAILED,
		"The readNetUnsignedInt24Bit method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READNETUNSIGNEDINT32BIT_FAILED,
		"The readNetUnsignedInt32Bit method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READNETUNSIGNEDINT64BIT_FAILED,
		"The readNetUnsignedInt64Bit method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READNETFLOAT16BIT_FAILED,
		"The readNetFloat16Bit method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READNETFLOAT32BIT_FAILED,
		"The readNetFloat32Bit method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READMP4DESCRIPTORSIZE_FAILED,
		"The readMP4DescriptorSize method of the FileReader class failed" },
	{ TOOLS_FILEREADER_REFRESHCACHEIFNECESSARY_FAILED,
		"The refreshCacheIfNecessary method of the FileReader class failed" },
	{ TOOLS_FILEREADER_READMORETHANCACHE_FAILED,
		"The readMoreThanCache method of the FileReader class failed" },
	{ TOOLS_FILEREADER_CACHESIZETOOLOW,
		"The cache size (%lu bytes) too low" },
	{ TOOLS_FILEREADER_BUFFERTOOSMALL,
		"The buffer is too small" },
	{ TOOLS_FILEREADER_REACHEDENDOFFILE,
		"Reached the end of the file" },
	{ TOOLS_FILEREADER_BUFFERNOTFOUND,
		"Buffer not found" },

	// MergeSort
	{ TOOLS_MERGESORT_MERGESORTASCENDING_FAILED,
		"The mergeSortAscending function of the MergeSort class failed" },
	{ TOOLS_MERGESORT_MERGESORTDESCENDING_FAILED,
		"The mergeSortDescending function of the MergeSort class failed" },

	// Convert
	{ TOOLS_CONVERT_BINARYTOBASE16_FAILED,
		"The binaryToBase16 function of the Convert class failed" },
	{ TOOLS_CONVERT_BASE16TOBINARY_FAILED,
		"The base16ToBinary function of the Convert class failed" },
	{ TOOLS_CONVERT_STRINGTOBASE16_FAILED,
		"The stringToBase16 function of the Convert class failed. SrcString: %s" },
	{ TOOLS_CONVERT_BASE16TOSTRING_FAILED,
		"The base16ToString function of the Convert class failed" },
	
	// Encrypt
	{ TOOLS_ENCRYPT_ENCRYPT_FAILED,
		"The encrypt function of the Encrypt class failed" },
	{ TOOLS_ENCRYPT_DECRYPT_FAILED,
		"The decrypt function of the Encrypt class failed" },
	{ TOOLS_ENCRYPT_GETCRYPTEDBUFFERLENGTH_FAILED,
		"The getCryptedBufferLength function of the Encrypt class failed" },
	{ TOOLS_ENCRYPT_GETDECRYPTEDBUFFERLENGTH_FAILED,
		"The getDecryptedBufferLength function of the Encrypt class failed" },

	// System
	{ TOOLS_SYSTEM_GETHOSTNAME_FAILED,
		"The getHostName function of the System class failed" },
	{ TOOLS_SYSTEM_GETHOMEDIRECTORY_FAILED,
		"The getHomeDirectory function of the System class failed" },
	{ TOOLS_SYSTEM_BUFFERTOOSMALL,
		"The buffer is too small" },
	{ TOOLS_SYSTEM_ENVIRONMENTVARIABLENOTDEFINED,
		"The '%s' is not defined" },

	// ProcessUtility
	{ TOOLS_PROCESSUTILITY_GETCURRENTPROCESSIDENTIFIER_FAILED,
		"The getCurrentProcessIdentifier function of the ProcessUtility class failed" },
	{ TOOLS_PROCESSUTILITY_EXECUTE_FAILED,
		"The execute function of the ProcessUtility class failed" },
	{ TOOLS_PROCESSUTILITY_SETUSERANDGROUPID_FAILED,
		"The setUserAndGroupID function of the ProcessUtility class failed" },
	{ TOOLS_PROCESSUTILITY_PROCESSDOESNOTEXITNORMALLY,
	"The process created to execute the '%s' command does not exit normally" },
	{ TOOLS_PROCESSUTILITY_USERNAMENOTFOUND,
		"The '%s' user name was not found" },

	// Network
	{ TOOLS_NETWORK_INIT_FAILED,
		"The init function of the Network class failed" },
	{ TOOLS_NETWORK_FINISH_FAILED,
		"The finish function of the Network class failed" },
	{ TOOLS_NETWORK_PARSEIPADDRESS_FAILED,
		"The parseIPAddress function of the Network class failed" },
	{ TOOLS_NETWORK_ISIPINNETWORK_FAILED,
		"The isIPInNetwork function of the Network class failed" },
	{ TOOLS_NETWORK_GETIPADDRESSFROMHOSTNAME,
		"The getIPAddressFromHostName function of the Network class failed. HostNameOrIPAddress: %s. Error: %s" },
	{ TOOLS_NETWORK_IPADDRESSNOTRETRIEVED,
		"IP Address not retrieved. HostNameOrIPAddress: %s" },

	// Service
	{ TOOLS_SERVICE_LAUNCHUNIXDAEMON_FAILED,
		"The launchUnixDaemon function of the Service class failed" },
	{ TOOLS_SERVICE_INIT_FAILED,
		"The init function of the Service class failed" },
	{ TOOLS_SERVICE_FINISH_FAILED,
		"The finish function of the Service class failed" },
	{ TOOLS_SERVICE_START_FAILED,
		"The start function of the Service class failed" },
	{ TOOLS_SERVICE_PARSEARGUMENTS_FAILED,
		"The parseArguments function of the Service class failed" },
	{ TOOLS_SERVICE_ISINSTALLED_FAILED,
		"The isInstalled function of the Service class failed" },
	{ TOOLS_SERVICE_INSTALL_FAILED,
		"The install function of the Service class failed" },
	{ TOOLS_SERVICE_SETSTATUS_FAILED,
		"The method setStatus of the Service class failed" },
	{ TOOLS_SERVICE_UNINSTALL_FAILED,
		"The unInstall function of the Service class failed" },
	{ TOOLS_SERVICE_ONINSTALL_FAILED,
		"The onInstall function of the Service class failed" },
	{ TOOLS_SERVICE_ONUNINSTALL_FAILED,
		"The onUninstall function of the Service class failed" },
	{ TOOLS_SERVICE_ONINIT_FAILED,
		"The onInit function of the Service class failed" },
	{ TOOLS_SERVICE_ONSTART_FAILED,
		"The onStart function of the Service class failed" },
	{ TOOLS_SERVICE_ONSTOP_FAILED,
		"The onStop function of the Service class failed" },
	{ TOOLS_SERVICE_ONCONTINUE_FAILED,
		"The method onContinue of the Service class failed" },
	{ TOOLS_SERVICE_ONDEVICEEVENT_FAILED,
		"The method onDeviceEvent of the Service class failed" },
	{ TOOLS_SERVICE_ONHARDWAREPROFILECHANGE_FAILED,
		"The method onHardwareProfileChange of the Service class failed" },
	{ TOOLS_SERVICE_ONINTERROGATE_FAILED,
		"The method onInterrogate of the Service class failed" },
	{ TOOLS_SERVICE_ONNETBINDADD_FAILED,
		"The method onNetBindAdd of the Service class failed" },
	{ TOOLS_SERVICE_ONNETBINDDISABLE_FAILED,
		"The method onNetBindDisable of the Service class failed" },
	{ TOOLS_SERVICE_ONNETBINDENABLE_FAILED,
		"The method onNetBindEnable of the Service class failed" },
	{ TOOLS_SERVICE_ONNETBINDREMOVE_FAILED,
		"The method onNetBindRemove of the Service class failed" },
	{ TOOLS_SERVICE_ONPARAMCHANGE_FAILED,
		"The method onParamChange of the Service class failed" },
	{ TOOLS_SERVICE_ONPAUSE_FAILED,
		"The method onPause of the Service class failed" },
	{ TOOLS_SERVICE_ONPOWEREVENT_FAILED,
		"The method onPowerEvent of the Service class failed" },
	{ TOOLS_SERVICE_ONSESSIONCHANGE_FAILED,
		"The method onSessionChange of the Service class failed" },
	{ TOOLS_SERVICE_ONSHUTDOWN_FAILED,
		"The method onShutdown of the Service class failed" },
	{ TOOLS_SERVICE_APPENDSTARTSCRIPTCOMMAND_FAILED,
		"The appendStartScriptCommand function of the Service class failed" },
	{ TOOLS_SERVICE_APPENDSTOPSCRIPTCOMMAND_FAILED,
		"The appendStopScriptCommand function of the Service class failed" },
	{ TOOLS_SERVICE_APPENDSTATUSSCRIPTCOMMAND_FAILED,
		"The appendStatusScriptCommand function of the Service class failed" },
	{ TOOLS_SERVICE_SERVICENOTINSTALLED,
		"Service not installed. Service description: %s" },
	{ TOOLS_SERVICE_SERVICEALREADYINSTALLED,
		"Service already installed. Service description: %s" },
	{ TOOLS_SERVICE_CURRENTRUNLEVELNOTFOUND,
		"Current runlevel not found" },
	{ TOOLS_SERVICE_INSTALLSERVICEEXECUTECOMMANDFAILED,
		"The command (%s) to install the service failed. Status returned: %ld. Install the service manually with the right command (chkconfig?)" },
	{ TOOLS_SERVICE_REMOVESERVICEEXECUTECOMMANDFAILED,
		"The command to remove the service failed. Status returned: %ld" },
	{ TOOLS_SERVICE_STARTSERVICEEXECUTECOMMANDFAILED,
		"The command to start the service failed. Status returned: %ld" },
	{ TOOLS_SERVICE_STOPSERVICEEXECUTECOMMANDFAILED,
		"The command to stop the service failed. Status returned: %ld" },
	{ TOOLS_SERVICE_RUNLEVELDIRECTORYLENGTHTOOSMALL,
		"Runlevel directory length too small" },
	{ TOOLS_SERVICE_SERVICEALREADYINPAUSE,
		"Service already in pause" },
	{ TOOLS_SERVICE_SERVICEALREADYSTOPPED,
		"Service already stopped" },
	{ TOOLS_SERVICE_SERVICENOTINPAUSE,
		"Service not in pause" },
	{ TOOLS_SERVICE_STOPFAILED,
		"The stopping of the service failed. Current state: %ld" },

	// common
	{ TOOLS_NEW_FAILED,
		"The new function failed" },
	{ TOOLS_SYSTEM_FAILED,
		"The system function failed" },
	{ TOOLS_FINDFIRST_FAILED,
		"The _findfirst function failed" },
	{ TOOLS_COCREATEINSTANCE_FAILED,
		"The CoCreateInstance function failed" },
	{ TOOLS_SHELLLINK_QUERYINTERFACE_FAILED,
		"The ShellLink::QueryInterface function failed" },
	{ TOOLS_SHELLLINK_GETPATH_FAILED,
		"The ShellLink::GetPath function failed" },
	{ TOOLS_PERSISTFILE_LOAD_FAILED,
		"The PersistFile::Load function failed" },
	{ TOOLS_GETCWD_FAILED,
		"The getcwd function failed. Errno: %ld" },
	{ TOOLS_GETADDRINFO_FAILED,
		"The getaddrinfo function failed. Errno: %ld, HostNameOrIPAddress: %s" },
	{ TOOLS_GETNAMEINFO_FAILED,
		"The getnameinfo function failed. Errno: %ld" },
	{ TOOLS_STAT_FAILED,
		"_stat failed. Pathname: %s, errno: %ld" },
	{ TOOLS_STATFS_FAILED,
		"statfs failed. Pathname: %s, errno: %ld" },
	{ TOOLS_SPRINTF_FAILED,
		"The sprintf function failed" },
	{ TOOLS_LSTAT_FAILED,
		"lstat failed. Pathname: %s, errno: %ld" },
	{ TOOLS_FSTAT_FAILED,
		"fstat failed. Pathname: %s, errno: %ld" },
	{ TOOLS_FCNTL_FAILED,
		"The fcntl function failed. Errno: %ld" },
	{ TOOLS_MBSTOWCS_FAILED,
		"The mbstowcs function failed" },
	{ TOOLS_WCSRTOMBS_FAILED,
		"The wcsrtombs function failed" },
	{ TOOLS_OPEN_FAILED,
		"The open function failed. Pathname: %s, errno: %ld" },
	{ TOOLS_LSEEK_FAILED,
		"The lseek function failed. Errno: %ld" },
	{ TOOLS_READ_FAILED,
		"The read function failed. Errno: %ld" },
	{ TOOLS_READLINK_FAILED,
		"The readLink function failed. Pathname: %s, Errno: %ld" },
	{ TOOLS_CHMOD_FAILED,
		"The chmod function failed. Pathname: %s, Errno: %ld" },
	{ TOOLS_WRITE_FAILED,
		"The write function failed. Errno: %ld" },
	{ TOOLS_CLOSE_FAILED,
		"The close function failed. Errno: %ld" },
	{ TOOLS_OPENDIR_FAILED,
		"The openDir function failed. Dir: %s, Errno: %ld" },
	{ TOOLS_CLOSEDIR_FAILED,
		"The closeDir function failed. Errno: %ld" },
	{ TOOLS_UNLINK_FAILED,
		"The unlink function failed. PathName: %s, Errno: %ld" },
	{ TOOLS_SYMLINK_FAILED,
		"The symlink function failed. OriPathName: %s, LinkPathName: %s, Errno: %ld" },
	{ TOOLS_MKDIR_FAILED,
		"The mkdir function failed. PathName: %s, Errno: %ld" },
	{ TOOLS_RMDIR_FAILED,
		"The rmdir function failed. PathName: %s, Errno: %ld" },
	{ TOOLS_SETGID_FAILED,
		"The setgid function failed. Errno: %ld, GroupID: %ld" },
	{ TOOLS_SETUID_FAILED,
		"The setuid function failed. Errno: %ld, UserID: %ld" },
	{ TOOLS_GZOPEN_FAILED,
		"The gzopen function failed" },
	{ TOOLS_GZWRITE_FAILED,
		"The gzwrite function failed" },
	{ TOOLS_GETTIMEZONEINFORMATION_FAILED,
		"The GetTimeZoneInformation function failed" },
	{ TOOLS_GETTIMEOFDAY_FAILED,
		"The gettimeofday function failed" },
	{ TOOLS_LOCALTIME_R_FAILED,
		"The localtime_r function failed" },
	{ TOOLS_LOCALTIME_FAILED,
		"The localtime function failed" },
	{ TOOLS_UNAME_FAILED,
		"The uname function failed" },
	{ TOOLS_MOVEFILE_FAILED,
		"Movefile failed (Error: %ld)" },
	{ TOOLS_WSASTARTUP_FAILED,
		"The WSAStartup function failed" },
	{ TOOLS_WSACLEANUP_FAILED,
		"The WSACleanup function failed" },
	{ TOOLS_WIDECHARTOMULTIBYTE_FAILED,
		"The WideCharToMultiByte function failed (Error: %ld)" },
	{ TOOLS_OPENPROCESSTOKEN_FAILED,
		"The OpenProcessToken function failed (Error: %ld)" },
	{ TOOLS_GETUSERPROFILEDIRECTORY_FAILED,
		"The GetUserProfileDirectory function failed (Error: %ld)" },
	{ TOOLS_MULTIBYTETOWIDECHAR_FAILED,
		"The MultiByteToWideChar function failed" },
	{ TOOLS_OPENSCMANAGER_FAILED,
		"OpenSCManager failed (Error: %ld)" },
	{ TOOLS_CLOSESERVICEHANDLE_FAILED,
		"CloseServiceHandle failed (Error: %ld)" },
	{ TOOLS_CREATESERVICE_FAILED,
		"The CreateService function failed. Error: %ld" },
	{ TOOLS_GETDISKFREESPACE_FAILED,
		"The GetDiskFreeSpace function failed. Error: %ld" },
	{ TOOLS_GETMODULEFILENAME_FAILED,
		"The GetModuleFileName function failed. Error: %ld" },
	{ TOOLS_OPENSERVICE_FAILED,
		"The OpenService function failed. Error: %ld" },
	{ TOOLS_DELETESERVICE_FAILED,
		"The DeleteService function failed. Error: %ld" },
	{ TOOLS_SETSERVICESTATUS_FAILED,
		"The SetServiceStatus function failed. Error: %ld" },
	{ TOOLS_STARTSERVICECTRLDISPATCHER_FAILED,
		"The StartServiceCtrlDispatcher function failed. Error: %ld" },
	{ TOOLS_REGISTERSERVICECTRLHANDLEREX_FAILED,
		"The RegisterServiceCtrlHandlerEx function failed. Error: %ld" },
	{ TOOLS_CONTROLSERVICE_FAILED,
		"The ControlService function failed. Error: %ld" },
	{ TOOLS_STARTSERVICE_FAILED,
		"The StartService function failed. Error: %ld" },
	{ TOOLS_ACTIVATION_WRONG,
		"Activation wrong" },
	{ TOOLS_OPERATION_NOTALLOWED,
		"Operation not allowed (Current object status: %ld)" }

	// Insert here other errors...

} ;
