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

#include <iostream>
#include "Event.h"

ostream& operator << (ostream& os, const Event& event)
{
    time_t tExpirationTimePoint = chrono::system_clock::to_time_t(event._expirationTimePoint);
    time_t tStartProcessingTime = chrono::system_clock::to_time_t(event._startProcessingTime);
    
    cout
        << "Event. "
        << ", type identifier: " << event._eventKey.first
        << "identifier: " << event._eventKey.second
        << ", source: " << event._source
        << ", destination: " << event._destination
        << ", expiration time point: " << ctime(&tExpirationTimePoint)
        << ", start processing time: " << ctime(&tStartProcessingTime)
        << endl;
    
    return os;
}
