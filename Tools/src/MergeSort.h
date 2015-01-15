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


#ifndef MergeSort_h
	#define MergeSort_h

	#include <iostream>		// altrimenti non riconosce NULL


	#ifndef Boolean_t
		typedef long	Boolean_t;
	#endif

	#ifndef Boolean_p
		typedef long	*Boolean_p;
	#endif

	#ifndef false
		#define false	0L
	#endif

	#ifndef true
		#define true	1L
	#endif


	/**
		MergeSort
			computational complexity is worst case=2*N
			best case=N*log2N
			the order is determinated using 'Ascending', 'Descending' appendix
	*/

	template<class T> class MergeSort

	{

		private:
			long mergeSortAscendingInternal (T *pTElements,
				long lFirstIndex, long lLastIndex);

			long mergeSortDescendingInternal (T *pTElements,
				long lFirstIndex, long lLastIndex);

			long mergeAscending (
				T *pTElements1, long lFirstIndex1, long lLastIndex1,
				T *pTElements2, long lFirstIndex2, long lLastIndex2,
				T *pTMergedElements, long *lMergedElementsNumber);

			long mergeDescending (
				T *pTElements1, long lFirstIndex1, long lLastIndex1,
				T *pTElements2, long lFirstIndex2, long lLastIndex2,
				T *pTMergedElements, long *lMergedElementsNumber);

			long copyBuffers (
				T *pTSrcElements, long lSrcFirstIndex, long lSrcLastIndex,
				T *pTDestElements, long lDestFirstIndex, long lDestLastIndex);

		public:
			long mergeSortAscending (T *pTElements, long lElementsNumber);

			long mergeSortDescending (T *pTElements, long lElementsNumber);

	} ;



	template<class T>
	long MergeSort<T>:: mergeSortAscending (T *pTElements, long lElementsNumber)

	{

		return mergeSortAscendingInternal (pTElements, 0, lElementsNumber - 1);
	}


	template<class T>
	long MergeSort<T>:: mergeSortDescending (T *pTElements,
		long lElementsNumber)

	{

		return mergeSortDescendingInternal (pTElements, 0, lElementsNumber - 1);
	}


	template<class T>
	long MergeSort<T>:: mergeSortAscendingInternal (T *pTElements,
		long lFirstIndex, long lLastIndex)

	{

		long				lMiddleIndex;
		T					*pTSortedElements;
		long				lSortedElementsNumber;


		if (lFirstIndex >= lLastIndex) 
			return 0;

		lMiddleIndex		= (lFirstIndex + lLastIndex) / 2;

		if (mergeSortAscendingInternal (pTElements, lFirstIndex, lMiddleIndex))
			return 1;

		if (mergeSortAscendingInternal (pTElements, lMiddleIndex + 1,
			lLastIndex))
			return 1;

		if ((pTSortedElements = new T [lLastIndex + 1]) == (T *) NULL)
			return 1;

		lSortedElementsNumber			= 0;

		if (mergeAscending (pTElements, lFirstIndex, lMiddleIndex,
			pTElements, lMiddleIndex + 1, lLastIndex,
			pTSortedElements, &lSortedElementsNumber))
		{
			delete [] pTSortedElements;

			return 1;
		}

		if (copyBuffers (pTSortedElements, 0, lSortedElementsNumber - 1,
			pTElements, lFirstIndex, lLastIndex))
		{
			delete [] pTSortedElements;

			return 1;
		}

		delete [] pTSortedElements;


		return 0;
	}


	template<class T>
	long MergeSort<T>:: mergeSortDescendingInternal (T *pTElements,
		long lFirstIndex, long lLastIndex)

	{

		long				lMiddleIndex;
		T					*pTSortedElements;
		long				lSortedElementsNumber;


		if (lFirstIndex >= lLastIndex)
			return 0;

		lMiddleIndex		= (lFirstIndex + lLastIndex) / 2;

		if (mergeSortDescendingInternal (pTElements, lFirstIndex, lMiddleIndex))
			return 1;

		if (mergeSortDescendingInternal (pTElements, lMiddleIndex + 1,
			lLastIndex))
			return 1;

		if ((pTSortedElements = new T [lLastIndex + 1]) == (T *) NULL)
			return 1;

		lSortedElementsNumber			= 0;

		if (mergeDescending (pTElements, lFirstIndex, lMiddleIndex,
			pTElements, lMiddleIndex + 1, lLastIndex,
			pTSortedElements, &lSortedElementsNumber))
		{
			delete [] pTSortedElements;

			return 1;
		}

		if (copyBuffers (pTSortedElements, 0, lSortedElementsNumber - 1,
			pTElements, lFirstIndex, lLastIndex))
		{
			delete [] pTSortedElements;

			return 1;
		}

		delete [] pTSortedElements;


		return 0;
	}


	template<class T>
	long MergeSort<T>:: mergeAscending (
		T *pTElements1, long lFirstIndex1, long lLastIndex1,
		T *pTElements2, long lFirstIndex2, long lLastIndex2,
		T *pTMergedElements, long *lMergedElementsNumber)

	{

		long			lIndex1;
		long			lIndex2;
		long			lMergedElementsIndex;
		Boolean_t		bIsOneBufferFinished;


		lIndex1						= lFirstIndex1;
		lIndex2						= lFirstIndex2;
	
		lMergedElementsIndex			= 0;

		bIsOneBufferFinished		=
			!((lIndex1 <= lLastIndex1 && lIndex2 <= lLastIndex2));

		while (!bIsOneBufferFinished)
		{
			if (pTElements1 [lIndex1] < pTElements2 [lIndex2])
			{
				pTMergedElements [lMergedElementsIndex++]			=
					pTElements1 [lIndex1];

				lIndex1++;
			}
			else
			{
				pTMergedElements [lMergedElementsIndex++]			=
					pTElements2 [lIndex2];
				lIndex2++;
			}

			bIsOneBufferFinished		=
				!((lIndex1 <= lLastIndex1 && lIndex2 <= lLastIndex2));
		}

		{
			long			lIndex;

			// at this point (lIndex1 > lLastIndex1 or lIndex2 > lLastIndex2)
			for (lIndex = lIndex1; lIndex <= lLastIndex1; lIndex++)
				pTMergedElements [lMergedElementsIndex++]		=
					pTElements1 [lIndex];

			for (lIndex = lIndex2; lIndex <= lLastIndex2; lIndex++)
				pTMergedElements [lMergedElementsIndex++]		=
					pTElements2 [lIndex];
		}

		*lMergedElementsNumber			=
			(lLastIndex1 - lFirstIndex1 + 1) + (lLastIndex2 - lFirstIndex2 + 1);


		return 0;
	}


	template<class T>
	long MergeSort<T>:: mergeDescending (
		T *pTElements1, long lFirstIndex1, long lLastIndex1,
		T *pTElements2, long lFirstIndex2, long lLastIndex2,
		T *pTMergedElements, long *lMergedElementsNumber)

	{

		long			lIndex1;
		long			lIndex2;
		long			lMergedElementsIndex;
		Boolean_t		bIsOneBufferFinished;


		lIndex1						= lFirstIndex1;
		lIndex2						= lFirstIndex2;

		lMergedElementsIndex			= 0;

		bIsOneBufferFinished		=
			!((lIndex1 <= lLastIndex1 && lIndex2 <= lLastIndex2));

		while (!bIsOneBufferFinished)
		{
			if (pTElements1 [lIndex1] > pTElements2 [lIndex2])
			{
				pTMergedElements [lMergedElementsIndex++]			=
					pTElements1 [lIndex1];

				lIndex1++;
			}
			else
			{
				pTMergedElements [lMergedElementsIndex++]			=
					pTElements2 [lIndex2];
				lIndex2++;
			}

			bIsOneBufferFinished		=
				!((lIndex1 <= lLastIndex1 && lIndex2 <= lLastIndex2));
		}

		{
			long			lIndex;

			// at this point (lIndex1 > lLastIndex1 or lIndex2 > lLastIndex2)
			for (lIndex = lIndex1; lIndex <= lLastIndex1; lIndex++)
				pTMergedElements [lMergedElementsIndex++]		=
					pTElements1 [lIndex];

			for (lIndex = lIndex2; lIndex <= lLastIndex2; lIndex++)
				pTMergedElements [lMergedElementsIndex++]		=
					pTElements2 [lIndex];
		}

		*lMergedElementsNumber			=
			(lLastIndex1 - lFirstIndex1 + 1) + (lLastIndex2 - lFirstIndex2 + 1);


		return 0;
	}


	template<class T>
	long MergeSort<T>:: copyBuffers (
		T *pTSrcElements, long lSrcFirstIndex, long lSrcLastIndex,
		T *pTDestElements, long lDestFirstIndex, long lDestLastIndex)

	{

		long		lSrcIndex;
		long		lDestIndex;


		lDestIndex			= lDestFirstIndex;

		for (lSrcIndex = lSrcFirstIndex; lSrcIndex <= lSrcLastIndex;
			lSrcIndex++)
		{
			if (lDestIndex <= lDestLastIndex)
				pTDestElements [lDestIndex++]		= pTSrcElements [lSrcIndex];
		}


		return 0;
	}

#endif

