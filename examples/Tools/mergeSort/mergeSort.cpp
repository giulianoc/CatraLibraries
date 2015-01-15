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

#include "MergeSort.h"
#include "Buffer.h"
#include "stdlib.h"


#define ELEMENTSNUMBER			100


template class MergeSort<Buffer_t>;


int main ()

{

	MergeSort<Buffer_t>			msMergeSort;

	Buffer_t					pbBuffers [ELEMENTSNUMBER];
	long						lIndex;


	for (lIndex = 0; lIndex < ELEMENTSNUMBER; lIndex++)
		pbBuffers [lIndex]			= ELEMENTSNUMBER - lIndex - 1;
//		pbBuffers [lIndex]			= ltoa (ELEMENTSNUMBER - lIndex - 1);

	std:: cout << "PRIMA DI ORDINARLI: " << std:: endl;

	for (lIndex = 0; lIndex < ELEMENTSNUMBER; lIndex++)
		std:: cout << pbBuffers [lIndex] << std:: endl;

	std:: cout << "Return Ordinamento ascendente: " <<
		msMergeSort. mergeSortAscending (pbBuffers, ELEMENTSNUMBER)
			<< std:: endl;

	std:: cout << "DOPO: " << std:: endl;

	for (lIndex = 0; lIndex < ELEMENTSNUMBER; lIndex++)
		std:: cout << pbBuffers [lIndex] << std:: endl;


	return 0;
}

