// 
// Copyright Emin Martinian, emin@allegro.mit.edu, 1998
// Permission granted to copy and distribute provided this
// comment is retained at the top.
//
// uncomment the following line if you are using Windows pre-compiled headers
// #include "stdafx.h"
#include <assert.h>
#include "my_hash_map.H"
#include "regression_tests.h"
#include "regress_1.h"

#define NDEBUG
#ifndef NDEBUG
#error "This is a regression test and must have NDEBUG defined to work!"
#endif

// This file contains some simple regression test functions to verify the
// presence/abscence of a bug in the InsertWithoutDuplication function.
//
// The idea is that these functions can be called (possibly with
// other tests) from regression_tests.cpp.  See regression_tests.cpp
// for more info.

// ----------------------------------------------------------------------
//
// FUNCTION:	CheckForBugInInsertWithoutDuplication
//
// INPUTS:	table: A table to test.
//
// RETURNS:	true if test passes and false (or core dump) otherwise.
//
// PURPOSE:	This first inserts and deletes a whole bunch of
//		random key-value pairs.  Then it uses 
//              InsertWithoutDuplication to randomly insert and delete
//              lots of random key-value pairs.  This test was 
//              originally designed to verify the presence and
//              and fixed-ness of a bug.  
//
// MODIFIED:	Sat Jan 13, 2001
//
// ----------------------------------------------------------------------

bool CheckForBugInInsertWithoutDuplication(TestTable*const table) {
  int i, didInsert;

  cout << "Testing for bug in InsertWithoutDuplication: ";

  for (i = 0; i < 10000; i++) {
    int key = random() % 10000;
    table->insert(key,0);
    table->Delete(key);
  }

  for (i = 0; i < 10000; i++) {
    int key = random() % 10000;
    table->InsertWithoutDuplication(key,1,&didInsert);
    table->Delete(key);
  }

  if (table->size() == 0) {
    cout << "Test passed." << endl;
    return true;
  } else {
    cout << "Test failed: final size wrong." << endl;
    return false;
  }

}

// ----------------------------------------------------------------------
//
// FUNCTION:	CheckForBugInDelete
//
// RETURNS:	true if test succeeds and false otherwise
//
// PURPOSE:	This test was originaly designed to verify the 
//              presence of a bug in delete and the fact that
//              it was fixed.  This function repeatedly tries
//              to delete stuff for an empty table.
//
// MODIFIED:	Sat Jan 13, 2001
//
// ----------------------------------------------------------------------

bool CheckForBugInDelete(TestTable*const table) {
  int i;

  cout << "Testing for bug in Delete: ";

  for (i = 0; i < 10000; i++) {
    int key = random() % 10000;
    table->Delete(key);
  }

  if (table->size() == 0) {
    cout << "Test passed." << endl;
    return true;
  } else {
    cout << "Test failed: final size wrong." << endl;
    return false;
  }

}

// ----------------------------------------------------------------------
//
// The following functions randomly add, delete, and search a table
// a whole bunch of times.
//

void DoAdd(const int maxSize, int*const simpleTable, int*const simpleCount,
	   TestTable*const table) {
  int didInsert;
  const int key = random() % maxSize;
  const int value = -(random() % maxSize)-1;
  assert(value < 0);
  if (random() % 2) {
    if (random() % 2) { // randomly use either insert or operator[]
      table->insert(key,value);
    } else {
      (*table)[key] = value;
    }
    if (simpleTable[key] == 0)
      (*simpleCount)++;
    else {
      assert(simpleTable[key] < 0);
    }
    simpleTable[key] = value;
  } else {
    if (simpleTable[key] == 0) {
      (*simpleCount)++;
      simpleTable[key] = value;
    } else {
      assert(simpleTable[key] < 0);
    }
    table->InsertWithoutDuplication(key,value,&didInsert);
  }
}

void DoDelete(const int maxSize, int*const simpleTable, int*const simpleCount,
	      TestTable*const table) {
  bool keepGoing = true;
  int key = -1;
  int itemToDelete = (random() % (*simpleCount)) + 1;
  while(keepGoing) {
    assert(key < maxSize);
    if (simpleTable[++key] < 0) {
      if (--itemToDelete <= 0)
	keepGoing = false;
    } 
  }
  
  table->Delete(key);
  (*simpleCount)--;
  simpleTable[key] = 0;
}

void DoClear(const int maxSize, int*const simpleTable, int*const simpleCount,
	     TestTable** table) {
  int i;
  for (i=0; i < maxSize; i++) {
    simpleTable[i] = 0;
  }
  *simpleCount = 0;
  (*table)->clear();
}

void DoCheck(const int maxSize, int*const simpleTable, int*const simpleCount,
	     TestTable*const table) {
  const int key = random() % maxSize;
  TestTable::iterator x = table->find(key);
  if (x == table->end()) {
    assert(simpleTable[key] == 0);
  } else {
    assert(simpleTable[key] == x->second);
    assert( (*table)[key] == simpleTable[key] );
  }
}

bool RandomlyAddRemoveAndClear(TestTable** table) {
  const int numRounds = 2;
  const int startSize = 500;
  const int maxSize = 10000;
  const double startingAddProb = .5;
  const double addProbDecrement = startingAddProb/100000;
  const double deleteProb = .2;
  const double checkProb = 0.49;
  const double clearProb = .01;
  int simpleTable[maxSize];
  int simpleCount = 0;
  int i, j, k;
  int maxCount = -1;
  
  cout << "Doing random add/remove/search/clear tests: ";

  for (i = 0; i < maxSize; i++) simpleTable[i] = 0;

  for (i = 0, k = 0; i < numRounds; i++) {
    double addProb = startingAddProb;
    for (j = 0; j < startSize; j++) {
      DoAdd(maxSize,simpleTable,&simpleCount,*table);
    }
    while ( (simpleCount > 0) && (simpleCount < maxSize) ) {
      if (simpleCount > maxCount) maxCount = simpleCount;
      assert((unsigned)simpleCount == (*table)->size());
      k++;
      addProb -= addProbDecrement;
      double uniformRV = ( (double) random() ) / ( (double) RAND_MAX) ;
      if (uniformRV <= addProb) 
	DoAdd(maxSize,simpleTable,&simpleCount,*table);
      uniformRV = ( (double) random() ) / ( (double) RAND_MAX) ;
      if (uniformRV <= checkProb) 
	DoCheck(maxSize,simpleTable,&simpleCount,*table);
      uniformRV = ( (double) random() ) / ( (double) RAND_MAX) ;
      if (uniformRV <= deleteProb) 
	DoDelete(maxSize,simpleTable, &simpleCount,*table);
      uniformRV = ( (double) random() ) / ( (double) RAND_MAX) ;
      if (uniformRV <= clearProb) {
	DoClear(maxSize,simpleTable, &simpleCount,table);
      }
    }
  }
  cout << "Test passed, maxCount = " << maxCount << "." << endl;
  return true;
}
	
// ----------------------------------------------------------------------
//
// FUNCTION:	CheckForBugInSearch
//
// RETURNS:	true if test succeeds and false otherwise
//
// PURPOSE:	This test was originaly designed to verify the 
//              presence of a bug in Search (reported by 
//              (Carl Andersen) and the fact that
//              it was fixed.  This function repeatedly tries
//              to delete stuff for an empty table.
//
// MODIFIED:	Mon Nov 11, 2002
//
// ----------------------------------------------------------------------

bool CheckForBugInSearch() {
  TestTable table = TestTable(2, LameHasher(), CompareInts() );
  
  cout << "Testing for bug in Search: ";

  table.SetResizeRatio(.5);

  table[0] = 99;
  table.Delete(0);
  table[4] = 66;
  assert(table.end() ==  table.Search(2));

  cout << "Test passed." << endl;

  return true;
}
