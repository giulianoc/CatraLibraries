
THIS FILE IS OBSOLETE AND HAS TO BE UPDATED

CatraLibraries
==============


CatraLibraries is a general-purpose C++ framework.
It is platform independent (Unix, MAC, Windows, ...) and provides classes to manage POSIX threads, Sockets, Sockets Connection Manager, Tracer, Scheduler,
EventsSet, Configuration files, HTTP Cache Manager, Software Load Balancer,
FTP, Tools (Buffers/Strings, DateTime, FileIO, FileReader, CPU Usage, Networks,
Processes, Services, ...), WebTools (HTTP GET, HTTP POST, HTTP Server, ...),
Database access (Oracle, Sybase), , ...

CatraLibraries is free software.   For copyright  information please  see the
file COPYING, which contains the GNU Public Library License.


Prerequisits - Unix
===================

Before trying to compile the CatraLibraries make sure you have installed the
following software packages:

   o gnu make version 3.7 or newer (required)
   o gcc compiler and library (required). We recommend gcc 3.4.2 or newer
   o flex 2.5.2 or newer (optional)
   o bison 1.22 or newer (optional)

flex and bison are only necessary if you change their input files
(files having the suffix .ll and .yy).


Prerequisits - Win32 (Windows 95/98/ME/NT/2000/XP)
==================================================

	o Microsoft's Visual C++ compiler 7.0
	o pthreads-dll-2005-03-08 library
		download the pthreads-dll-2005-01-25 directory
			from ftp://sources.redhat.com/pub/pthreads-win32.
			(the site is http://sources.redhat.com/pthreads-win32)
		Create the link pthreads-dll (must be brother
			of the catralibraries directory) to pthreads-dll-2005-01-25
	o flex 2.5.2 or newer (optional)
	o bison 1.22 or newer (optional)

flex and bison are only necessary if you change their input files
(files having the suffix .ll and .yy).

Installation - Unix
===================

For installation perform the following steps:
	- verify the "Prerequisits - Unix"
	- ./configure
	- make
	- make install (you must be logged as root to execute this command)
For detailed instructions please see the file INSTALL.


Installation - Win32 (Windows 95/98/ME/NT/2000/XP)
==================================================

For installation perform the following steps:
	- verify the "Prerequisits - Win32 (Windows 95/98/ME/NT/2000/XP)"
	- explode the catralibraries package as brother
		of the pthreads-dll directory
	- open the CatraLibraries.sln project into
		the catralibraries directory with 
		Microsoft's Visual C++ compiler 7.0
	- rebuild all the project ('Build -> Rebuild Solution' menù item)
	- all the libraries and executables (examples) will be into the
		relative 'Debug' directories

Documentation
=============

	Please refer the documents inside the doc subdirectory.

Examples
========

	Please refer the examples inside the examples subdirectory.

