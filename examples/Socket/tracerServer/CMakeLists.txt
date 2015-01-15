
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

SET (SOURCES
	tracerServer.cpp
	TracerService.cpp
	TracerSocketsPool.cpp
	TracerServerErrors.cpp
	TracerServerMessages.cpp
)

SET (HEADERS
	TracerService.h
	TracerSocketsPool.h
	TracerServerErrors.h
	TracerServerMessages.h
)

include_directories ("${PROJECT_SOURCE_DIR}/ErrMsg/src")
include_directories ("${PROJECT_SOURCE_DIR}/Tools/src")
include_directories ("${PROJECT_SOURCE_DIR}/PThread/src")
include_directories ("${PROJECT_SOURCE_DIR}/Socket/src")
include_directories ("${PROJECT_SOURCE_DIR}/Tracer/src")
include_directories ("${PROJECT_SOURCE_DIR}/Configuration/src")
include_directories ("${PROJECT_SOURCE_DIR}/ConfigurationFile/src")
include_directories ("${PROJECT_SOURCE_DIR}/my_hash_map")
if(CRYPTO_FOUND)
	include_directories(${CRYPTO_INCLUDE_DIR})
endif()
if (ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
endif()

add_compile_options(-D_REENTRANT)

add_executable(tracerServer ${SOURCES} ${HEADERS})

target_link_libraries (tracerServer ConfigurationFile)
target_link_libraries (tracerServer Configuration)
target_link_libraries (tracerServer Tracer)
target_link_libraries (tracerServer Socket)
target_link_libraries (tracerServer PThread)
target_link_libraries (tracerServer Tools)
target_link_libraries (tracerServer ErrMsg)
if(CRYPTO_FOUND)
	if (APPLE)
		target_link_libraries (tracerServer ${CRYPTO_LIBRARIES})
	else()
		target_link_libraries (tracerServer crypt)
	endif()
endif()
if (ZLIB_FOUND)
	target_link_libraries(tracerServer ${ZLIB_LIBRARIES})
endif()
target_link_libraries (tracerServer pthread)

