// 
// Copyright Emin Martinian, emin@allegro.mit.edu, 1998
// Permission granted to copy and distribute provided this
// comment is retained at the top.
//
// uncomment the following line if you are using Windows pre-compiled headers
// #include "stdafx.h"
#include <assert.h>
#include "regression_tests.h"
#include "regress_1.h"

// This file runs the various regression tests.  The functions
// implementing the regerssion tests are in regress_1.cpp.  

int main() {
  TestTable* table = new TestTable(10, LameHasher(), CompareInts() );
		     
  if (CheckForBugInInsertWithoutDuplication(table) &&
      CheckForBugInDelete(table) &&
      RandomlyAddRemoveAndClear(&table) &&
      CheckForBugInSearch()) {
    delete table;
    return 0;
  } else {
    cerr << "At least 1 test failed" << endl;
    return 1;
  }
}
