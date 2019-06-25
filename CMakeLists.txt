
# Copyright (C) Giuliano Catrambone (giuliano.catrambone@catrasoftware.it)

# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation; either 
# version 2 of the License, or (at your option) any later 
# version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

# Commercial use other than under the terms of the GNU General Public
# License is allowed only after express negotiation of conditions
# with the authors.

cmake_minimum_required(
	VERSION 2.8.12
)

project(
	CatraLibraries
)

SET(CMAKE_CXX_FLAGS "-std=c++14")

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Release)

set(CatraLibraries_VERSION_MAJOR 1)
set(CatraLibraries_VERSION_MINOR 0)

INCLUDE (CheckFunctionExists)
CHECK_FUNCTION_EXISTS(poll HAVE_POLL)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
	"${PROJECT_SOURCE_DIR}/CatraLibrariesConfig.h.in"
	"${PROJECT_BINARY_DIR}/CatraLibrariesConfig.h"
)


# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(LibCrypto)
find_package(ZLIB)
find_package(Ldap)

set(INSTALL_INCLUDE_DIRECTORY include/catralibraries)

add_subdirectory(my_hash_map)
add_subdirectory(ErrMsg/src)
#add_subdirectory(doc)
add_subdirectory(Tools/src)

add_subdirectory(md5/src)
add_subdirectory(DBConnectionPool/src)

add_subdirectory(PThread/src)
add_subdirectory(Socket/src)
add_subdirectory(WebTools/src)
add_subdirectory(HTTPCacheManager/src)
add_subdirectory(Configuration/src)
add_subdirectory(ConfigurationFile/src)
add_subdirectory(ConnectionsManager/src)
add_subdirectory(EventsSet/src)
add_subdirectory(MultiEventsSet/src)
add_subdirectory(MyFTP/src)
add_subdirectory(SNMPWrapper/src)
add_subdirectory(Tracer/src)
add_subdirectory(Scheduler/src)
add_subdirectory(LoadBalancer/src)

add_subdirectory(examples/DBConnectionPool/dbConnectionPool)

add_subdirectory(examples/Configuration/configuration)
add_subdirectory(examples/Configuration/cryptedItem)

add_subdirectory(examples/ConfigurationFile/appendConfigurationItemValue)
add_subdirectory(examples/ConfigurationFile/removeConfigurationSection)
add_subdirectory(examples/ConfigurationFile/configurationFile)
add_subdirectory(examples/ConfigurationFile/getConfigurationItemValue)
add_subdirectory(examples/ConfigurationFile/saveCryptedItem)
add_subdirectory(examples/ConfigurationFile/setConfigurationItemValue)

add_subdirectory(examples/ErrMsg/checkErrors)
add_subdirectory(examples/ErrMsg/messages)

add_subdirectory(examples/EventsSet/printerSpooler)

add_subdirectory(examples/MultiEventsSet/producerConsumer)

add_subdirectory(examples/LoadBalancer/loadBalancer)

add_subdirectory(examples/MyFTP/ftpPut)

add_subdirectory(examples/PThread/conditionVariable)
add_subdirectory(examples/PThread/coreOfAThread)
add_subdirectory(examples/PThread/detachThread)
add_subdirectory(examples/PThread/nativeThread)
add_subdirectory(examples/PThread/nestedDetachThreads)
add_subdirectory(examples/PThread/threadsAndMutex)

add_subdirectory(examples/SNMPWrapper/sendTrap)

add_subdirectory(examples/Scheduler/dataProcessing_Calendar)
add_subdirectory(examples/Scheduler/dataProcessing_Periodic)
add_subdirectory(examples/Scheduler/meetings)

add_subdirectory(examples/Socket/SocketsPool/client)
add_subdirectory(examples/Socket/SocketsPool/server)
add_subdirectory(examples/Socket/demoDGRAMSocket/client)
add_subdirectory(examples/Socket/demoDGRAMSocket/server)
add_subdirectory(examples/Socket/demoSTREAMSocket/client)
add_subdirectory(examples/Socket/demoSTREAMSocket/server)
add_subdirectory(examples/Socket/demoSTREAMSocket_NewProcess/client)
add_subdirectory(examples/Socket/demoSTREAMSocket_NewProcess/server)
add_subdirectory(examples/Socket/getInterfaces)
add_subdirectory(examples/Socket/getMACAddress)
add_subdirectory(examples/Socket/httpRequest)
add_subdirectory(examples/Socket/telnetClient)
add_subdirectory(examples/Socket/tracerServer)
add_subdirectory(examples/Tracer/demoTracer)
add_subdirectory(examples/Tracer/tracerClient)
add_subdirectory(examples/Tracer/usingTwoTracer)

add_subdirectory(examples/WebTools/decodeURL)
add_subdirectory(examples/WebTools/encodeURL)
add_subdirectory(examples/WebTools/getURLParameterValue)
add_subdirectory(examples/WebTools/httpGetThread)
add_subdirectory(examples/WebTools/httpPostThread)
add_subdirectory(examples/WebTools/httpServer)
add_subdirectory(examples/WebTools/parseURL)

add_subdirectory(examples/Tools/GetCpuUsage)
add_subdirectory(examples/Tools/appendBuffer)
add_subdirectory(examples/Tools/base64)
add_subdirectory(examples/Tools/buffer)
add_subdirectory(examples/Tools/copyFile)
add_subdirectory(examples/Tools/createDirectory)
add_subdirectory(examples/Tools/createLink)
add_subdirectory(examples/Tools/decrypt)
add_subdirectory(examples/Tools/dateTime)
add_subdirectory(examples/Tools/dirCmp)
add_subdirectory(examples/Tools/directoryList)
add_subdirectory(examples/Tools/encrypt)
add_subdirectory(examples/Tools/execute)
add_subdirectory(examples/Tools/forkAndExec)
add_subdirectory(examples/Tools/fileReader)
add_subdirectory(examples/Tools/getDirectoryUsage)
add_subdirectory(examples/Tools/getFileSizeInBytes)
add_subdirectory(examples/Tools/getFileSystemInfo)
add_subdirectory(examples/Tools/getFileTime)
add_subdirectory(examples/Tools/getHostName)
add_subdirectory(examples/Tools/isFileExisting)
if (LDAP_FOUND)
	add_subdirectory(examples/Tools/ldapWrapper)
endif()
add_subdirectory(examples/Tools/mergeSort)
add_subdirectory(examples/Tools/moveDirectory)
add_subdirectory(examples/Tools/moveFile)
add_subdirectory(examples/Tools/moveLink)
add_subdirectory(examples/Tools/readLinesFromFile)
add_subdirectory(examples/Tools/removeFile)
add_subdirectory(examples/Tools/service)
add_subdirectory(examples/Tools/stringTokenizer)

