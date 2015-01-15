// 
// Copyright Emin Martinian, emin@allegro.mit.edu, 1998
// Permission granted to copy and distribute provided this
// comment is retained at the top.
//
// uncomment the following line if you are using Windows pre-compiled headers
// #include "stdafx.h"
#include <assert.h>
#include <string.h>
#include "my_hash_map.H"

// This file contains some sample code snippets demonstrating how to use
// the my_hash_map hash table template for a hash table with strings as
// keys and floats as values.

// Warn the user that NDEBUG is turned on.
#ifndef NDEBUG
#warning "Debugging assertions are turned on."
#warning "After you have got all the bugs out,"
#warning "turn off checking via #define NDEBUG"
#warning "or compiling with -DNDEBUG."
#endif


struct StringHasher : MyHasherModel<char*> {
public:

  // We need a hash function for strings.  There are many different
  // ways to obtain or create a hash function, but anything reasonable
  // should work fine.  The hash function below is pretty simple
  // minded, but it should be ok.  See the book
  // _Introduction_To_Algorithms_ by Cormen, Leisserson, and Rivest
  // for good description of hash functions and suggestions on how to
  // choose better hash functions.

  int operator()(char*const & key)const {
    int i, result = 0;
    const int length = strlen(key);
    for(i=0; i<length ; i++) {
      result = result*5 + key[i];
    }
    return GENERIC_HASH(result); // GENERIC_HASH is defined in my_hash_map.H
  }

};

struct StringCmp {
  bool operator()(const char*const x, const char*const y) const {
    return !strcmp(x,y);
  }
};

typedef my_hash_map<char*,float,StringHasher,StringCmp> CurrencyTable;

int main() {
  StringHasher hasher;
  StringCmp comparer;
  CurrencyTable cTable(10,hasher,comparer);
  char* myString = new char[10];

  strcpy(myString,"POUND");
  cTable["CAD"]= 0.65;
  cTable["YEN"]= 1.95;
  cTable[myString] = 1.7;
  cout << "rate for CAD is " << cTable["CAD"] << endl;
  cout << "rate for YEN is " << cTable["YEN"] << endl;
  cout << "rate for " << myString << " is " << cTable[myString] << endl;
  cout << "rate for FOO is " << cTable["FOO"] << endl
       << "because FOO wasn't in the table but got inserted " << endl
       << "automatically and defaulted to 0 via the [] operator." << endl;
  delete[] myString;

  return 0;
}
