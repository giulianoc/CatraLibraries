
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
	BaseProcessor.cpp
	CheckServerSocketTimes.cpp
	CheckSocketsPoolTimes.cpp
	CheckUnusedSocketsTimes.cpp
	CMEventsSet.cpp
	CMProcessor.cpp
	CMSocketsPool.cpp
	ConnectionEvent.cpp
	ConnectionsManagerErrors.cpp
	ConnectionsManagerMessages.cpp
	ConnectionsManagerService.cpp
	HTTPProxyThread.cpp
	Session.cpp
)
SET (HEADERS
	BaseProcessor.h
	CheckServerSocketTimes.h
	CheckSocketsPoolTimes.h
	CheckUnusedSocketsTimes.h
	CMEventsSet.h
	CMProcessor.h
	CMSocketsPool.h
	ConnectionEvent.h
	ConnectionsManagerErrors.h
	ConnectionsManagerMessages.h
	ConnectionsManagerService.h
	HTTPProxyThread.h
	Session.h
)

include_directories ("${PROJECT_SOURCE_DIR}/ErrMsg/src")
include_directories ("${PROJECT_SOURCE_DIR}/Tools/src")
include_directories ("${PROJECT_SOURCE_DIR}/Socket/src")
include_directories ("${PROJECT_SOURCE_DIR}/PThread/src")
include_directories ("${PROJECT_SOURCE_DIR}/Tracer/src")
include_directories ("${PROJECT_SOURCE_DIR}/my_hash_map")
include_directories ("${PROJECT_SOURCE_DIR}/WebTools/src")
include_directories ("${PROJECT_SOURCE_DIR}/EventsSet/src")
include_directories ("${PROJECT_SOURCE_DIR}/Scheduler/src")
include_directories ("${PROJECT_SOURCE_DIR}/LoadBalancer/src")
include_directories ("${PROJECT_SOURCE_DIR}/Configuration/src")
include_directories ("${PROJECT_SOURCE_DIR}/ConfigurationFile/src")
add_compile_options(-D_REENTRANT)

add_library (ConnectionsManager SHARED ${SOURCES} ${HEADERS})

if (APPLE)
	#target_link_libraries (ConnectionsManager ErrMsg)
	#target_link_libraries (ConnectionsManager Tools)
	#target_link_libraries (ConnectionsManager PThread)
	#target_link_libraries (ConnectionsManager Socket)
	target_link_libraries (ConnectionsManager Configuration)
	target_link_libraries (ConnectionsManager ConfigurationFile)
	target_link_libraries (ConnectionsManager EventsSet)
	target_link_libraries (ConnectionsManager LoadBalancer)
	target_link_libraries (ConnectionsManager Scheduler)
	#if (ZLIB_FOUND)
		#target_link_libraries(ConnectionsManager ${ZLIB_LIBRARIES})
	#endif()
endif()

install (TARGETS ConnectionsManager DESTINATION lib)
install (FILES ${HEADERS} DESTINATION ${INSTALL_INCLUDE_DIRECTORY})

