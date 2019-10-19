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


#include "TracerErrors.h"


ErrMsgBase:: ErrMsgsInfo TracerErrorsStr = {


	// Tracer
	{ TRACER_TRACER_INIT_FAILED,
		"The init method of the Tracer class failed" },
	{ TRACER_TRACER_FINISH_FAILED,
		"The finish method of the Tracer class failed" },
	{ TRACER_TRACER_CANCEL_FAILED,
		"The cancel method of the Tracer class failed" },
	{ TRACER_TRACER_TRACEFUNCTIONBEGIN_FAILED,
		"The traceFunctionBegin method of the Tracer class failed" },
	{ TRACER_TRACER_TRACEFUNCTIONEND_FAILED,
		"The traceFunctionEnd method of the Tracer class failed" },
	{ TRACER_TRACER_TRACE_FAILED,
		"The trace method of the Tracer class failed" },
	{ TRACER_TRACER_SETTRACERSHUTDOWN_FAILED,
		"The setTracerShutdown method of the Tracer class failed" },
	{ TRACER_TRACER_GETTRACERSHUTDOWN_FAILED,
		"The getTracerShutdown method of the Tracer class failed" },
	{ TRACER_TRACER_PERFORMTRACE_FAILED,
		"The performTrace method of the Tracer class failed" },
	{ TRACER_TRACER_ADDTRACE_FAILED,
	"The addTrace method of the Tracer class failed. Trace message: %s" },
	{ TRACER_TRACER_GETTRACEONFILE_FAILED,
		"The getTraceOnFile method of the Tracer class failed" },
	{ TRACER_TRACER_GETTRACEONTTY_FAILED,
		"The getTraceOnTTY method of the Tracer class failed" },
	{ TRACER_TRACER_GETTRACELEVEL_FAILED,
		"The getTraceLevel method of the Tracer class failed" },
	{ TRACER_TRACER_GETCOMPLETETRACELENGTH_FAILED,
		"The getCompleteTraceLength method of the Tracer class failed" },
	{ TRACER_TRACER_POPULATETRACEFILECACHE_FAILED,
		"The populateTraceFileCache method of the Tracer class failed" },
	{ TRACER_TRACER_FLUSHOFTRACES_FAILED,
		"The flushOfTraces method of the Tracer class failed" },
	{ TRACER_TRACER_FLUSHTRACEFILECACHE_FAILED,
		"The flushTraceFileCache method of the Tracer class failed" },
	{ TRACER_TRACER_FILLCOMPLETETRACEMESSAGE_FAILED,
		"The fillCompleteTraceMessage method of the Tracer class failed" },
	{ TRACER_TRACER_GZIPANDDELETECURRENTTRACEFILE_FAILED,
		"The gzipAndDeleteCurrentTraceFile method of the Tracer class failed" },
	{ TRACER_TRACER_FILLCLOSEDFILESREPOSITORY_FAILED,
		"The fillClosedFilesRepository method of the Tracer class failed" },
	{ TRACER_TRACER_CHECKFILESYSTEMSIZE_FAILED,
		"The checkFileSystemSize method of the Tracer class failed" },
	{ TRACER_TRACER_SETTRACEONFILE_FAILED,
		"The setTraceOnFile method of the Tracer class failed" },
	{ TRACER_TRACER_SETTRACEONTTY_FAILED,
		"The setTraceOnTTY method of the Tracer class failed" },
	{ TRACER_TRACER_SETTRACELEVEL_FAILED,
		"The setTraceLevel method of the Tracer class failed" },
	{ TRACER_TRACER_SETTRACEFILESNUMBERTOMAINTAIN_FAILED,
		"The setTraceFilesNumberToMaintain method of the Tracer class failed" },
	{ TRACER_TRACER_SETCOMPRESSEDTRACEFILE_FAILED,
		"The setCompressedTraceFile method of the Tracer class failed" },
	{ TRACER_TRACER_SETMAXTRACEFILESIZE_FAILED,
		"The setMaxTraceFileSize method of the Tracer class failed" },
	{ TRACER_TRACER_GETCURRENTTRACEFILENUMBER_FAILED,
		"The getCurrentTraceFileNumber method of the Tracer class failed" },
	{ TRACER_TRACER_GETNEXTTRACEFILENUMBER_FAILED,
		"The getNextTraceFileNumber method of the Tracer class failed" },
	{ TRACER_TRACER_TRACEFILECLOSED_FAILED,
		"The traceFileClosed method of the Tracer class failed" },
	{ TRACER_TRACER_REMOVEOLDFILE_FAILED,
		"The removeOldFile method of the Tracer class failed" },
	{ TRACER_TRACER_GETNEWTRACEPATHFILENAME_FAILED,
		"The getNewTracePathFileName method of the Tracer class failed" },
	{ TRACER_TRACER_NOSPACEAVAILABLE,
		"No space available on File System (%s). Available Bytes: %llu" },
	{ TRACER_TRACETOOLONG,
		"The trace message too long (%lu), increase the trace file cache (%lu)" },
	{ TRACER_COMMANDWRONG,
		"The command (%s) is wrong" },
	{ TRACER_TRACER_TRACEFILEPATHNAMENOTAVAILABLE,
		"The trace file name is not available" },

	// common
	{ TRACER_OPERATION_NOTALLOWED,
		"Operation not allowed. Status: %ld" },
	{ TRACER_ACTIVATION_WRONG,
		"Activation wrong" },
	{ TRACER_NEW_FAILED,
		"The new function failed" },
	{ TRACER_SYSTEM_FAILED,
		"The system function failed (command: %s)" },
	{ TRACER_LOCALTIME_R_FAILED,
		"The localtime_r function failed" },
	{ TRACER_GZOPEN_FAILED,
		"The gzopen function failed" },
	{ TRACER_GZWRITE_FAILED,
		"The gzwrite function failed" }

	// Insert here other errors...

} ;

