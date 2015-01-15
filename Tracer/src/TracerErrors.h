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


#ifndef TracerErrors_h
	#define TracerErrors_h

	#include "Error.h"
	#include <iostream>

	/**
		Click <a href="TracerErrors.C#TracerErrors" target=classContent>here</a> for the errors strings.
	*/
	enum TracerErrorsCodes {

		// Tracer
		TRACER_TRACER_INIT_FAILED,
		TRACER_TRACER_FINISH_FAILED,
		TRACER_TRACER_CANCEL_FAILED,
		TRACER_TRACER_TRACEFUNCTIONBEGIN_FAILED,
		TRACER_TRACER_TRACEFUNCTIONEND_FAILED,
		TRACER_TRACER_TRACE_FAILED,
		TRACER_TRACER_SETTRACERSHUTDOWN_FAILED,
		TRACER_TRACER_GETTRACERSHUTDOWN_FAILED,
		TRACER_TRACER_PERFORMTRACE_FAILED,
		TRACER_TRACER_ADDTRACE_FAILED,
		TRACER_TRACER_GETTRACEONFILE_FAILED,
		TRACER_TRACER_GETTRACEONTTY_FAILED,
		TRACER_TRACER_GETTRACELEVEL_FAILED,
		TRACER_TRACER_GETCOMPLETETRACELENGTH_FAILED,
		TRACER_TRACER_POPULATETRACEFILECACHE_FAILED,
		TRACER_TRACER_FLUSHOFTRACES_FAILED,
		TRACER_TRACER_FLUSHTRACEFILECACHE_FAILED,
		TRACER_TRACER_FILLCOMPLETETRACEMESSAGE_FAILED,
		TRACER_TRACER_GZIPANDDELETECURRENTTRACEFILE_FAILED,
		TRACER_TRACER_FILLCLOSEDFILESREPOSITORY_FAILED,
		TRACER_TRACER_CHECKFILESYSTEMSIZE_FAILED,
		TRACER_TRACER_SETTRACEONFILE_FAILED,
		TRACER_TRACER_SETTRACEONTTY_FAILED,
		TRACER_TRACER_SETTRACELEVEL_FAILED,
		TRACER_TRACER_SETTRACEFILESNUMBERTOMAINTAIN_FAILED,
		TRACER_TRACER_SETCOMPRESSEDTRACEFILE_FAILED,
		TRACER_TRACER_SETMAXTRACEFILESIZE_FAILED,
		TRACER_TRACER_GETCURRENTTRACEFILENUMBER_FAILED,
		TRACER_TRACER_GETNEXTTRACEFILENUMBER_FAILED,
		TRACER_TRACER_TRACEFILECLOSED_FAILED,
		TRACER_TRACER_REMOVEOLDFILE_FAILED,
		TRACER_TRACER_GETNEWTRACEPATHFILENAME_FAILED,
		TRACER_TRACER_NOSPACEAVAILABLE,
		TRACER_TRACETOOLONG,
		TRACER_COMMANDWRONG,
		TRACER_TRACER_TRACEFILEPATHNAMENOTAVAILABLE,

		// common
		TRACER_OPERATION_NOTALLOWED,
		TRACER_ACTIVATION_WRONG,
		TRACER_NEW_FAILED,
		TRACER_SYSTEM_FAILED,
		TRACER_LOCALTIME_R_FAILED,
		TRACER_GZOPEN_FAILED,
		TRACER_GZWRITE_FAILED,

		// Insert here other errors...

		TRACER_MAXERRORS

	} ;

	#ifdef WIN32
		// warning C4267: 'argument' : conversion from 'size_t' to 'long', possible loss of data

		#pragma warning (disable : 4267)
	#endif

	// declaration of class error
	dclCustomErrorClass (TracerErrors, TRACER_MAXERRORS)
   
#endif
