
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
	CollectionTimes.cpp
	dataProcessing.cpp
	DistributionTimes.cpp
)

SET (HEADERS
	CollectionTimes.h
	DistributionTimes.h
)

include_directories ("${PROJECT_SOURCE_DIR}/ErrMsg/src")
include_directories ("${PROJECT_SOURCE_DIR}/Tools/src")
include_directories ("${PROJECT_SOURCE_DIR}/PThread/src")
include_directories ("${PROJECT_SOURCE_DIR}/Scheduler/src")
if(CRYPTO_FOUND)
	include_directories(${CRYPTO_INCLUDE_DIR})
endif()
if (ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
endif()

add_compile_options(-D_REENTRANT)

add_executable(dataProcessing_Calendar ${SOURCES} ${HEADERS})

target_link_libraries (dataProcessing_Calendar Scheduler)
target_link_libraries (dataProcessing_Calendar PThread)
target_link_libraries (dataProcessing_Calendar Tools)
target_link_libraries (dataProcessing_Calendar ErrMsg)
if(CRYPTO_FOUND)
	if (APPLE)
		target_link_libraries (dataProcessing_Calendar ${CRYPTO_LIBRARIES})
	else()
		target_link_libraries (dataProcessing_Calendar crypt)
	endif()
endif()
if (ZLIB_FOUND)
	target_link_libraries(dataProcessing_Calendar ${ZLIB_LIBRARIES})
endif()
if (LDAP_FOUND)
	target_link_libraries(dataProcessing_Calendar ${LDAP_LIBRARIES})
endif()
target_link_libraries (dataProcessing_Calendar pthread)
target_link_libraries (dataProcessing_Calendar crypto)

