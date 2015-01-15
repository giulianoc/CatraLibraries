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


#ifndef Tracer_h
	#define Tracer_h

	#ifdef WIN32
		#include "WinThread.h"
	#else
		#include "PosixThread.h"
	#endif
	#include "PMutex.h"
	#include "TracerErrors.h"
	#include "Buffer.h"
    #ifdef __QTCOMPILER__
        #include <QtCore/qglobal.h>

        #if defined(TRACER_LIBRARY)
            #define TRACERSHARED_EXPORT Q_DECL_EXPORT
        #else
            #define TRACERSHARED_EXPORT Q_DECL_IMPORT
        #endif
    #else
        #define TRACERSHARED_EXPORT
    #endif


	#define TRACER_MAXDATELENGTH						(10 + 1) // YYYY-MM-DD

	#define TRACER_MAXTIMELENGTH						(12 + 1) // HH:MM:SS:MIL

	#define TRACER_MAXTRACEFILENUMBERLENGTH				(6 + 1) // _YYYY_

	#define TRACER_MAXTRACELEVES						50

	#define TRACER_MAXDEFAULTTRACELEVELSNUMBER			11

	#define TRACER_DEFAULTCACHESIZEOFTRACEFILE			50

	#define TRACER_MAXCONNECTIONBUFFER					(1024 + 1)

	#define TRACER_MAXPATHFILENAMELENGTH				(512 + 1)

	#define TRACER_FLUSHCOMMAND							"flushTraceFileCache"

	#define TRACER_SETMAXTRACEFILESIZECOMMAND			"setMaxTraceFileSize"

	#define TRACER_SETCOMPRESSEDTRACEFILECOMMAND		"setCompressedTraceFile"

	#define TRACER_SETTRACEFILESNUMBERTOMAINTAINCOMMAND	"setTraceFilesNumberToMaintain"

	#define TRACER_SETTRACEONFILECOMMAND				"setTraceOnFile"

	#define TRACER_SETTRACEONTTYCOMMAND					"setTraceOnTTY"

	#define TRACER_SETTRACELEVELCOMMAND					"setTraceLevel"


	/**
		La libreria Tracer gestisce il trace di una applicazione
		compromettendo in modo minimale le performance della stessa.
		L'oggetto Tracer e' un thread il cui unico compito e' quello
		di gestire il trace.

		Ecco le caratteristiche principali:

		* cache configurable
		* output of the trace configurable
		* support of multiple output files configurable according time and size
		* configurabilita' del numero di files di trace da mantenere
		* support to compress the generated trace files
		* 11 levels of trace
		* possibility to customize the levels of trace
		* possibility to change the configuration in real time
		* different processes can also use the same configuration file
			(that means the same trace files). In that case, the access to
			pBaseTraceFileName is managed through a lock on the file.
		* e' possibile influire sulle performance dell'applicazione
			specificando una dimensione massima del buffer di trace.
			Minori sono le dimensioni del buffer di trace, maggiori sono
			le performance dell'applicazione pero' maggiore e' la probabilita'
			che messaggi di trace vengono persi. In questo caso si mantiene
			l'informazione di quanti messaggi di trace sono stati persi.

		* This implementation is based on two differents pools of traces.
		* The first pool (called MAIN POLL) is filled by the client
		* applications and the other one (called SECONDARY POOL)
		* is used to build the traces and added them to the local cache
		* in memory. When the cache is full, it will be flushed on trace file.
		* All the activity on the SECONDARY POOL and on the cache is made
		* in background and the application client can continue to fill the
		* MAIN POOL.
		* Each lSecondsBetweenTwoCheckTraceToManage seconds the two polls
		* are exchanged in order to
		*	free the pool to receive the new traces by the client applications
		*	to feed the background process that will move the traces on file

		I passi per l'uso della libreria sono i seguenti:

		* create a Tracer object (Tracer_t tTracer)
		* initialize the Tracer (tTracer. init (...))
		* start the thread (tTracer. start ())
		* ......use the trace method to trace a message/error (tTracer. trace (...))....
		* at the end
		*	cancel the Tracer thread (tTracer. cancel ())
		*	finish the Tracer thread (tTracer. finish (...))

		The Tracer class is thread safe and could be declared as a global variable and used by all the application.

		Gli esempi che si trovano nella directory examples chiariranno
		l'uso di questa libreria.
	*/
	#ifdef WIN32
        typedef class TRACERSHARED_EXPORT Tracer: public WinThread
	#else
        typedef class TRACERSHARED_EXPORT Tracer: public PosixThread
	#endif
	{
		protected:
			typedef enum TracerStatus {
				TRACER_BUILDED,
				TRACER_INITIALIZED
			} TracerStatus_t, *TracerStatus_p;

		public:
			typedef enum DefaultTraceLevel
			{
				TRACER_LDBG1,
				TRACER_LDBG2,
				TRACER_LDBG3,
				TRACER_LDBG4,
				TRACER_LDBG5,
				TRACER_LDBG6,
				TRACER_LINFO,
				TRACER_LMESG,
				TRACER_LWRNG,
				TRACER_LERRR,
				TRACER_LFTAL
			} DefaultTraceLevel_t, *DefaultTraceLevel_p;

			typedef struct TraceInfo
			{
				tm					_tmDateTime;
				unsigned long		_ulMilliSecs;
				long				_lTraceLevel;
				Buffer_t			_bTraceMessage;
				Buffer_t			_bFileName;
				long				_lFileLine;
				unsigned long		_ulThreadId;
				long				_lUpdateStackDeep;
			} TraceInfo_t, *TraceInfo_p;

		private:
			TracerStatus_t		_tsTracerStatus;

			// Per proteggere la condivisione di _ptiTraces tra Tracer::run
			// e Tracer:: trace, Tracer:: traceFunctionBegin,
			// Tracer::traceFunctionEnd. Praticamente questo mutex protegge
			// le variabili del pool di traces (MAIN POOL) usato
			// dalle applicazioni clients per aggiungere trace
			PMutex_t				_mtMutexForTraces;

			// Struttura dati dove vengono inseriti i trace dei clients
			TraceInfo_p				_ptiTraces;
			long					_lTracesNumber;
			long					_lAllocatedTracesNumber;
			long					_lTracesNumberAllocatedOnOverflow;

			// Struttura dati di appoggio dove vengono copiati i trace
			// dei clients per gestirli
			TraceInfo_p				_ptiTracesToManage;
			long					_lTracesNumberToManage;
			long					_lAllocatedTracesNumberToManage;
			long					_lTracesNumberToManageAllocatedOnOverflow;

			Boolean_t				_bClosedFileToBeCopied;
			char					*_pClosedFilesRepository;

			long					_lMaxTracesNumber;
			long					_lLostTracesNumber;

			long					_lSecondsBetweenTwoCheckTraceToManage;

			Boolean_t				_bTracerShutdown;

			char					*_pName;
			long					_lNameLength;

			Buffer_t				_bLockPathName;
			unsigned long long	_ullMaxTraceFileSize;
			long					_lTraceFilesNumberToMaintain;
			long					_lCurrentTraceFileNumber;
			Boolean_t				_bHasFileNumberRound;
			int						_iFileDescriptor;
			Buffer_t				_bLastTraceFilePathName;
			Buffer_t				_bTraceFilePathInfoName;
			Boolean_t				_bAreSubstitutionToDo;
			#ifdef WIN32
				__int64					_ullCurrentTraceFileSize;
			#else
				unsigned long long		_ullCurrentTraceFileSize;
			#endif

			long					_lSizeOfEachBlockToGzip;

			Boolean_t				_bTraceOnFile;

			long					_lTraceLevel;
			char					*_pTraceLevelLabel [TRACER_MAXTRACELEVES];

			long					_lListenPort;

		protected:
			// Per proteggere la condivisione delle variabili di Tracer tra
			// Tracer:: performTrace e Tracer:: get..., Tracer:: set...
			// Praticamente questo mutex protegge tutto tranne
			// le variabili del pool di traces usato dalle applicazioni clients
			// per aggiungere trace (SECONDARY POOL + other variables).
			// E' complementare al mutex precedente.
			PMutex_t				_mtMutexForTracerVariable;

			Boolean_t				_bCompressedTraceFile;

			long					_lFunctionsStackDeep;
			Boolean_t				_bTraceOnTTY;
			time_t					_tStartTraceFile;
			long					_lTraceFilePeriodInSecs;

			Buffer_t				_bTraceDirectory;
			Buffer_t				_bTraceFileName;

			Buffer_t				_bLinkToTheLastTraceFileName;

			Boolean_t				_bTraceFileCacheActive;
			long					_lCacheSizeOfTraceFile;
			long					_lCurrentCacheBusyInBytes;
			char					*_pTraceFileCache;

		private:
			Error addTrace (long lTraceLevel, const char *pTraceMessage,
				const char *pFileName, long lFileLine, unsigned long ulThreadId,
				long lUpdateStackDeep);

			Error getTracerShutdown (Boolean_p pbTracerShutdown);

			Error setTracerShutdown (Boolean_t bTracerShutdown);

			Error getCompleteTraceLength (long lTraceLevel,
				const char *pTraceMessage, const char *pFileName,
				long lFileLine, unsigned long ulThreadId,
				long *plTraceLevelLabelLength,
				long *plFileNameLength, char *pFileLine,
				long *plFileLineLength,
				char *pThreadId, long *plThreadIdLength,
				long *plTraceMessageLength,
				long *plCompleteTraceMessageLength);

			Error fillCompleteTraceMessage (char *pCompleteTraceMessageToFill,
				char *pCurrentDate, char *pCurrentTime,
				long lTraceLevel, long lTraceLevelLabelLength,
				const char *pFileName, long lFileNameLength,
				const char *pFileLine, long lFileLineLength,
				const char *pThreadId, long lThreadIdLength,
				const char *pTraceMessage, long lTraceMessageLength);

			#ifdef _USEGZIPLIB
				Error gzipAndDeleteCurrentTraceFile (void);
			#endif

			Error populateTraceFileCache (void);

			Error getCurrentTraceFileNumber (
				long *plCurrentTraceFileNumber,
				Boolean_p pbHasFileNumberRound);

			Error getNextTraceFileNumber (
				long *plCurrentTraceFileNumber,
				Boolean_p pbHasFileNumberRound);

			Error checkFileSystemSize (const char *pDirectoryPathName);

			Error removeOldFile (long lTraceFileNumber);

			Error getNewTraceFilePathName (
				long lTraceFileNumber, Buffer_p pbNewTraceFileName);

		protected:
			virtual Error performTrace (
				tm *ptmDateTime, unsigned long *pulMilliSecs,
				long lTraceLevel, const char *pTraceMessage,
				const char *pFileName, long lFileLine,
				unsigned long ulThreadId, long lUpdateStackDeep);

			Error flushTraceFileCache (void);

			/**
				This method is called every time the Tracer close
				a trace file.
				It could be redefined to process the file just closed. 
			*/
			virtual Error traceFileClosed (const char *pTraceFilePathName);

			virtual Error fillClosedFilesRepository (
				Buffer_p pbClosedFilesRepository);

			virtual Error run (void);

		public:
			Tracer (void);

			~Tracer (void);

			Error init (const char *pName,

				// If lCacheSizeOfTraceFile is initialized to -1, the cache will
				//	not be used and the traces will be flushed as they are
				//	added to the Tracer
				long lCacheSizeOfTraceFile,				// K-byte
				const char *pTraceDirectory,

				// The TraceFileName could contain the following patterns
				// that will be substitute appropriately
				// when the file name is created:
				//	- @YYYY@: year having 4 digits
				//	- @MM@: month having 2 digits
				//	- @DD@: day having 2 digits
				//	- @HI24@: hour having 2 digits
				//	- @MI@: minutes having 2 digits
				//	- @SS@: seconds having 2 digits
				//	- @MSS@: milliseconds having 3 digits
				const char *pTraceFileName,

				// the next two parameters determine the creation
				//	of the next trace file (the condition for the
				//	two parameters is OR)
				#ifdef WIN32
					__int64 ullMaxTraceFileSize = 1000,		// K-byte
				#else
					unsigned long long ullMaxTraceFileSize = 1000,	// K-byte
				#endif
				long lTraceFilePeriodInSecs = 60 * 10,	// 10 minutes

				Boolean_t bCompressedTraceFile = true,

				// With the next two parameters, it is possible to set
				// a repository where the a trace file will be copied
				// when it is closed
				// bClosedFileToBeCopied enable or disable this functionality
				// pClosedFilesRepository is the directory path name
				// 	where to copy the trace files
				Boolean_t bClosedFileToBeCopied = false,
				const char *pClosedFilesRepository = (const char *) NULL,

				// -1 se si vogliono mantenere tutti i files
				long lTraceFilesNumberToMaintain = 5,
				Boolean_t bTraceOnFile = true,
				Boolean_t bTraceOnTTY = true,
				long lTraceLevel = 0,
				long lSecondsBetweenTwoCheckTraceToManage = 15,

				// se -1 buffer di trace illimitato
				long lMaxTracesNumber = 5000,

				// if -1 the Tracer does not implement a server
				// to listen runtime commands
				long lListenPort = 7531,
				long lTracesNumberAllocatedOnOverflow = 10000,
				long lSizeOfEachBlockToGzip = 1000);	// K-byte

			Error finish (Boolean_t bFlushOfTraces);

			Error cancel (void);

			#ifdef WIN32
				Error traceFunctionBegin (long lTraceLevel,
					const char *pFunctionName, const char *pFileName,
					long lFileLine,
					unsigned long ulThreadId =
					WinThread:: getCurrentThreadIdentifier ());
			#else
				Error traceFunctionBegin (long lTraceLevel,
					const char *pFunctionName, const char *pFileName,
					long lFileLine,
					unsigned long ulThreadId =
					PosixThread:: getCurrentThreadIdentifier ());
			#endif

			#ifdef WIN32
				Error traceFunctionEnd (long lTraceLevel, const char *pFunctionName,
					const char *pFileName, long lFileLine,
					unsigned long ulThreadId =
					WinThread:: getCurrentThreadIdentifier ());
			#else
				Error traceFunctionEnd (long lTraceLevel, const char *pFunctionName,
					const char *pFileName, long lFileLine,
					unsigned long ulThreadId =
					PosixThread:: getCurrentThreadIdentifier ());
			#endif

			#ifdef WIN32
				Error trace (long lTraceLevel, const char *pTraceMessage,
					const char *pFileName, long lFileLine,
					unsigned long ulThreadId =
					WinThread:: getCurrentThreadIdentifier ());
			#else
				Error trace (long lTraceLevel, const char *pTraceMessage,
					const char *pFileName, long lFileLine,
					unsigned long ulThreadId =
					PosixThread:: getCurrentThreadIdentifier ());
			#endif

			Error getTraceOnFile (Boolean_p pbTraceOnFile);

			Error getTraceOnTTY (Boolean_p pbTraceOnTTY);

			Error getName (char *pName);

			// Error getBaseTraceFileName (char *pBaseTraceFileName);

			#ifdef WIN32
				Error getMaxTraceFileSize (
					__int64 *pullMaxTraceFileSize);
			#else
				Error getMaxTraceFileSize (
					unsigned long long *pullMaxTraceFileSize);
			#endif

			#ifdef WIN32
				Error setMaxTraceFileSize (
					__int64 ullMaxTraceFileSize);
			#else
				Error setMaxTraceFileSize (
					unsigned long long ullMaxTraceFileSize);
			#endif

			Error setCompressedTraceFile (Boolean_t bCompressedTraceFile);

			Error getTraceFilesNumberToMaintain (
				long *plTraceFilesNumberToMaintain);

			Error setTraceFilesNumberToMaintain (
				long lTraceFilesNumberToMaintain);

			Error setTraceOnFile (Boolean_t bTraceOnFile);

			Error setTraceOnTTY (Boolean_t bTraceOnTTY);

			Error getTraceLevel (long *plTraceLevel);

			Error setTraceLevel (long lTraceLevel);

			Error resetTraceLevels (void);

			Error addTraceLevel (const char *pTraceLevelLabel);

			Error flushOfTraces (void);

	} Tracer_t, *Tracer_p;

    #define FUNCTION_TRACER(lTracerLevel) \
		struct FunctionTracer { \
			FunctionTracer()	\
			{ \
				pgtDebugTracer-> traceFunctionBegin (   \
					lTracerLevel, __FUNCTION__,  \
					__FILE__, __LINE__); \
			} \
			~FunctionTracer()	\
			{ \
				pgtDebugTracer-> traceFunctionEnd ( \
					lTracerLevel, __FUNCTION__,    \
					__FILE__, __LINE__); \
			} \
		} FunctionTracerInstance;

/*
    #define FUNCTION_TRACER(lTracerLevel, pFunctionName) \
		struct FunctionTracer { \
			FunctionTracer()	\
			{ \
				pgtDebugTracer-> traceFunctionBegin (   \
					lTracerLevel, #pFunctionName ,  \
					__FILE__, __LINE__); \
			} \
			~FunctionTracer()	\
			{ \
				pgtDebugTracer-> traceFunctionEnd ( \
					lTracerLevel, #pFunctionName ,    \
					__FILE__, __LINE__); \
			} \
		} FunctionTracerInstance;
*/

#endif

