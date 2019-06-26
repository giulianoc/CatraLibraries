
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
	connectorCppAPI.cpp
)

SET (HEADERS
)

find_program(LSB_RELEASE_EXEC lsb_release)
execute_process(COMMAND ${LSB_RELEASE_EXEC} -is
    OUTPUT_VARIABLE LSB_RELEASE_ID_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("-- LSB_RELEASE_ID_SHORT: ${LSB_RELEASE_ID_SHORT}")

#if(LSB_RELEASE_ID_SHORT STREQUAL "CentOS")
	include_directories ("/usr/include/mysql-cppconn/jdbc")
#endif()

#link_directories ("${PROJECT_SOURCE_DIR}/../mysql-connector-c++-8.0.16-linux-glibc2.12-x86-64bit/lib64")

add_executable(connectorCppAPI ${SOURCES} ${HEADERS})

target_link_libraries (connectorCppAPI mysqlcppconn)

#install (TARGETS connectorCppAPI DESTINATION bin)

