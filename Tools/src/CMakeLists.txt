
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
	Buffer.cpp
	Convert.cpp
	DateTime.cpp
	Encrypt.cpp
	FileIO.cpp
	FileReader.cpp
	GetCpuUsage.cpp
	Network.cpp
	Service.cpp
	StringTokenizer.cpp
	System.cpp
	ProcessUtility.cpp
	ToolsErrors.cpp
)
SET (HEADERS
	Buffer.h
	Convert.h
	DateTime.h
	Encrypt.h
	FileIO.h
	FileReader.h
	GetCpuUsage.h
	MergeSort.h
	Network.h
	Service.h
	StringTokenizer.h
	System.h
	ProcessUtility.h
	ToolsErrors.h
)

include_directories ("${PROJECT_SOURCE_DIR}/ErrMsg/src")
add_compile_options(-D_REENTRANT -DSERVICEDEBUG -DUSEGZIPLIB -D_FILE_OFFSET_BITS=64)

add_library (Tools SHARED ${SOURCES} ${HEADERS})

if (APPLE)
	target_link_libraries (Tools ErrMsg)
	if (ZLIB_FOUND)
		target_link_libraries(Tools ${ZLIB_LIBRARIES})
	endif()
endif()

install (TARGETS Tools DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include)

