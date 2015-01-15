
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

find_package(BISON)
find_package(FLEX)

#if run from MAC use y.tab.c and lex.yy.c
#From Makefile.am: AM_YFLAGS = -d -v -l
BISON_TARGET(MyParser configurationParser.yy ${CMAKE_CURRENT_BINARY_DIR}/configurationParser.cc)
FLEX_TARGET(MyScanner configurationScanner.ll  ${CMAKE_CURRENT_BINARY_DIR}/configurationScanner.cc)
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

SET (SOURCES
	${BISON_MyParser_OUTPUTS}
	${FLEX_MyScanner_OUTPUTS}
	ConfigurationFile.cpp
	ConfigurationFileErrors.cpp
)
SET (HEADERS
	configurationParser.h
	ConfigurationFileErrors.h
	ConfigurationFile.h
)

include_directories ("${PROJECT_SOURCE_DIR}/ErrMsg/src")
include_directories ("${PROJECT_SOURCE_DIR}/Tools/src")
include_directories ("${PROJECT_SOURCE_DIR}/Configuration/src")
include_directories ("${PROJECT_SOURCE_DIR}/PThread/src")
add_compile_options(-D_REENTRANT -D_FLEX)
#add_compile_options(-D_REENTRANT -D_FLEX -DYACC_DEBUG -DLEX_DEBUG)

add_library (ConfigurationFile SHARED ${SOURCES} ${HEADERS})

if (APPLE)
	#target_link_libraries (ConfigurationFile ErrMsg)
	#target_link_libraries (ConfigurationFile Tools)
	target_link_libraries (ConfigurationFile Configuration)
	#if (ZLIB_FOUND)
		#target_link_libraries(ConfigurationFile ${ZLIB_LIBRARIES})
	#endif()
endif()

install (TARGETS ConfigurationFile DESTINATION lib)
install (FILES ${HEADERS} DESTINATION include)

