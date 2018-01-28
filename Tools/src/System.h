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


#ifndef System_h
#define System_h

#include <string>
#include "ToolsErrors.h"

using namespace std;

/**
    The System class is a collection of static methods just
    to hide the differences to retrieve information between
    different operative system.
*/
typedef class System
{

private:
    System (const System &);

    System &operator = (const System &);

public:
    /**
            Costruttore.
    */
    System ();

    /**
            Distruttore.
    */
    ~System ();

/*
metodi completi ma non ancora pubblicati
BOOL GetOSVersion(char *pOS)
char *GetCPUInfo()
int GetProcessorNum()
BOOL GetLocalHostName(char *name)
*/

    /**
            Return the host name of the machine.
    */
    static Error getHostName (
            char *pHostName,
            unsigned long ulHostNameBufferLength);

    static string getHostName ();

    static Error getHomeDirectory (
            char *pHomeDirectory,
            unsigned long ulBufferLength);

} System_t, *System_p;

#endif

