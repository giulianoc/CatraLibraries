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

bin_PROGRAMS = httpRequest

##These files are added to the distribution by default.
httpRequest_SOURCES =	\
	httpRequest.cpp

httpRequest_CPPFLAGS =	\
	-I${top_srcdir}/Socket/src	\
	-I${top_srcdir}/ErrMsg/src	\
	-I${top_srcdir}/Tools/src	\
	-I${top_srcdir}/PThread/src	\
	-I${top_srcdir}/WebTools/src

httpRequest_LDADD =	\
	${top_srcdir}/ErrMsg/src/libErrMsg.la	\
	${top_srcdir}/Socket/src/libSocket.la	\
	${top_srcdir}/PThread/src/libPThread.la	\
	${top_srcdir}/Tools/src/libTools.la	\
	-lz	\
	${top_srcdir}/WebTools/src/libWebTools.la	\
	@GZIP_LIBRARY@	\
	@CRYPT_LIBRARY@	\
	-lpthread

EXTRA_DIST =	\
	example_Socket_httpRequest.vcproj	\
	headerChangeClipSBRequest.txt	\
	bodyChangeClipSBRequest.txt	\
	headerChangeClipStreamerWrong.txt	\
	bodyChangeClipStreamerWrong.txt	\
	headerGetSessionStatusSBRequest.txt	\
	bodyGetSessionStatusSBRequest.txt	\
	headerGetSessionStatusStreamer.txt	\
	bodyGetSessionStatusStreamer.txt	\
	headerTerminateSessionStreamer.txt	\
	bodyTerminateSessionStreamer.txt

AM_CXXFLAGS = -D_REENTRANT

