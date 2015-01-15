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
// the my_hash_map hash table template.

// Warn the user that NDEBUG is turned on.
#ifndef NDEBUG
#warning "Debugging assertions are turned on."
#warning "After you have got all the bugs out,"
#warning "turn off checking via #define NDEBUG"
#warning "or compiling with -DNDEBUG."
#endif

// Assume that we want to keep track of people by their ID number.
// Thus we will be hashing based on an integer key (the ID number).
// The first step is to create a function object (called a functor
// in STL parlance) which does the hashing.
//
// You can either create a simple hasher which just implements
// one hash function as in STL, or you can implement a more
// advanced hash functor which has 2 hash functions defined.
// Your hash table will be more efficient if you implement
// the more advanced hash functor so we show that example first.
// If you just want to do something simple skip to the section 
// entitled SIMPLE HASH FUNCTOR.
//
// We will create the struct IDHasher.  It will be a struct with
// two member functions: operator() and SecondHashValue.  Since we are
// hashing based on ID number (an integer), both these functions
// take integer arguments.  Note the second const keyword in the 
// definition of both functions.  This const tells the compiler 
// that neither the Hash function nor the SecondHashValue modifies
// the IDHasher struct.  You MUST create functions which do not
// modify the hash function object and tell the compiler this with
// the const keyword.

struct IDHasher {

  // Since we are hashing based on integers we can just use the
  // GENERIC_HASH macro.  GENERIC_HASH is a macro which takes
  // an integer and scrambles it up.
  int operator()(const int & key)const {return GENERIC_HASH(key);}

  // SecondHashValue also takes an integer and scrambles it up.
  // However, for this to be useful we need SecondHashValue to
  // scramble the key up in a different way.  Therefore we multiply
  // key by 3 before calling GENERIC_HASH.  Also note that we have
  // set things up so SecondHashValue ALWAYS returns an odd number.
  // This is REQUIRED.  SecondHashValue must ALWAYS return an odd
  // number for the hash table to be properly searched.
  int SecondHashValue(const int & key)const {
    return 2*GENERIC_HASH(3*key) + 1;
  }

};

// SIMPLE HASH FUNCTOR:
//
// If you don't want to go to all the trouble of implementing
// 2 hash functinos there is a simpler way.  Just define one
// hash function which inherits from the template MyHasherModel 
// as shown below.  If you inherit from MyHasherModel then
// the second hash function will be automagically defined. 

struct SimpleIDHasher : public MyHasherModel<int> {
public:
  // Since we are hashing based on integers we can just use the
  // GENERIC_HASH macro.  GENERIC_HASH is a macro which takes
  // an integer and scrambles it up.
  int operator()(const int & key)const {return GENERIC_HASH(key);}

};

struct IDEqualCmp {
  bool operator()(const int x, const int y) const {return x == y;}
};

// Now that we have defined the IDHasher object, let us define
// the Person class so that we can put something interesting in
// the hash table.  We let the Person object have a name and age
// field.

class Person {
public:
  
  Person(const char * const name, const int age)
    :_name(new char[(strlen(name)+1)]), _age(age)
    {
      strcpy(_name, name);
    }

  ~Person()
    {
      delete [] _name;
    }

  const char * const GetName() const {return _name;}
  
  int GetAge() const {return _age;}

private:
  char * _name;
  int _age;
};

// Next we set up typedefs so we can create a my_hash_map object.
// Note the arguments in the brackets < >.  The first argument says
// that we will be using integers as the keys in the table.  The
// second argument says that we will be storing Person* in the table
// and the third argument says that the IDHasher struct will do the
// hashing. Finally the fourth argument says we will be using the
// IDEqualCmp class to do key comparisons.

typedef my_hash_map<int,Person*,IDHasher, IDEqualCmp> MyPersonTable;
typedef my_hash_map<int,Person*,SimpleIDHasher, IDEqualCmp> MySimplePersonTable;

void DoTestsWithAdvancedHasher() {

  int didInsert;

  // The first thing we need to do is create an instance of the
  // IDHasher class so we can pass it to a my_hash_map object.
  IDHasher hasher;
  IDEqualCmp equaltiyComparator;

  cout << " Doing tests with advanced hasher " << endl;

  // Next we create a my_hash_map object.  
  // The 10 means that personTable will initially be able to hold
  // at least 10 objects (it will grow automagically when needed).
  // The second argument is the IDHasher object we created above.
  MyPersonTable personTable (10, hasher, equaltiyComparator);
		     
  // Let's say we want to find the person with ID 10.  Recall
  // that the find member function returns an iterator.  An 
  // iterator is basically like a pointer.  So the line below
  // declares a new iterator x and assigns to it the result
  // of searching personTable for a person with id number 11.
  MyPersonTable::iterator x = personTable.find(11);

  // We haven't put anyone in the table so the iterator x should be
  // pointing to end so we assert that to make sure.
  assert( ! (x < personTable.end()));


  // Okay now let's put some people in the table.  
  // I made up some random id numbers, names, and ages.
  cout << "Inserting people into table" << endl;
  personTable.insert(1, new Person("Mike",25));
  personTable.insert(57, new Person("Tom",24));
  personTable.insert(29, new Person("John",17));
  personTable.insert(10, new Person("Esmerelda",24));
  personTable.insert(63, new Person("Ernie",16));

  // First we have to check if the iterator x is "pointing" to something
  // valid.  When we search for something not in the table the find
  // function will return an iterator which equals personTable.end().
  if (x == personTable.end()) 
    cout << "Nobody found with id# 11 (this is the correct result)" <<endl;
  else
    cout << "Someone found with id# 11 (this is an error!)" <<endl;

  // Now let's search for somebody who is in the table.
  // We search for someone with id #10.  
  x = personTable.find(10);
  if (x == personTable.end()) 
    cout << "Nobody found with id# 10 (this is an error!)" <<endl;
  else {
    // In this branch we have found someone with id #10.  The iterator, x,
    // is pointing to a record in personTable for id #10.  The value
    // of x->first will be the key for the record which in this case 
    // is an integer.  The value of x->second will be the record stored
    // in the table which for personTable is a Person*.  If we had
    // declared things as
    //
    //     my_hash_map<char,double,IDHasher> personTable (10, hasher );
    //     ...
    //     my_hash_map<char,double,IDHasher>::iterator x;
    //
    // then x->first would be a char and x->second would be a double.
    cout << "Found someone with id# 10 (this is correct)" <<endl;
    cout << '\t' << x->second->GetName() << ", age " << x->second->GetAge();
    cout << ", id# " << x->first << endl;
  }
      
  // If we insert an item with a key that already exists in the hash_map,
  // we will overwrite the previous value.  For example, if we insert
  // a person named Bob into the table with id # 10 we will overwrite
  // the previous person with id # 10.  The following code demonstrates this.
  
  // WARNING: the following line leaks memory:
  personTable.insert(10, new Person("Bob",33));
  if (x == personTable.end()) 
    cout << "Nobody found with id# 10 (this is an error!)" <<endl;
  else {
    // In this branch we have found someone with id #10.  The person
    // will be Bob since we just inserted him into the table.  In so
    // doing we overwrote the previous person with id #10.  This is bad
    // for two reasons: (1) We have now forgotten about the previous
    // person with id # 10 and (2) we no longer have a pointer to the
    // person who originally had id #10.  Even if we are done with the
    // person who was overwritten we should have delete'd the pointer.
    cout << "Found someone with id# 10 (this is correct)" <<endl;
    cout << '\t' << x->second->GetName() << ", age " << x->second->GetAge();
    cout << ", id# " << x->first << endl;
  }
  
  // One way to avoid the problem with overwriting previous existing
  // entries is to use the InsertWithoutDuplication function.  This
  // function checks if its argument is already in the table.  If so
  // it returns an iterator pointing to the item in the table and
  // DOES NOT modify the table.  Otherwise it DOES modify the table 
  // by putting the new entry into the table and returning an iterator
  // pointing to the newly inserted item.  The third argument to
  // InsertWithoutDuplication duplication must be a pointer to an integer.
  // It gets set to 1 if an item is inserted and 0 otherwise.
  
  Person* newPeople[2] ;
  newPeople[0] = new Person("James",11);
  newPeople[1] = new Person("Jerry",29);
  for (int i=0; i < 2; i++) {
    x = personTable.InsertWithoutDuplication(newPeople[i]->GetAge(),
					     newPeople[i],&didInsert);
    if (!didInsert) {
      // Oops we tried to insert some with an id # already in the table!
      // Complain about duplicate id numbers.
      cout << "Tried to insert " << newPeople[i]->GetName() 
	   << " into table, but " << x->second->GetName() << endl
	   << " was already there and they both have id # " 
	   << x->first << endl;
      cout << "Deleting " << newPeople[i]->GetName() << endl;
      delete newPeople[i];
    } else {
      cout << "Inserted " << newPeople[i]->GetName() 
	   << " without duplication." << endl;
    }
  }
  
  // Ok now we are done so let us step through the table and report
  // everyone in the table.  After reporting each person we will delete
  // them from the table and free their memory.  We are going to
  // use an iterator to do this in a for loop.
  MyPersonTable::iterator currentPerson;
  for (currentPerson = personTable.begin();
       currentPerson != personTable.end(); currentPerson++) {
    cout << currentPerson->second->GetName() << ": id#" 
	 << currentPerson->first << ", age: " 
	 << currentPerson->second->GetAge() << endl;
    delete currentPerson->second;
  }
  
  return;
}

// now redo everything with the simple hasher.
void DoTestsWithSimpleHasher() {

  int didInsert;
  SimpleIDHasher hasher;
  IDEqualCmp equaltiyComparator;
  
  cout << " Doing tests with simple hasher " << endl;

  MySimplePersonTable personTable (10, hasher, equaltiyComparator);
  MySimplePersonTable::iterator x = personTable.find(11);

  assert( ! (x < personTable.end()));

  cout << "Inserting people into table" << endl;
  personTable.insert(1, new Person("Mike",25));
  personTable.insert(57, new Person("Tom",24));
  personTable.insert(29, new Person("John",17));
  personTable.insert(10, new Person("Esmerelda",24));
  personTable.insert(63, new Person("Ernie",16));

  if (x == personTable.end()) 
    cout << "Nobody found with id# 11 (this is the correct result)" <<endl;
  else
    cout << "Someone found with id# 11 (this is an error!)" <<endl;

  x = personTable.find(10);
  if (x == personTable.end()) 
    cout << "Nobody found with id# 10 (this is an error!)" <<endl;
  else {
    cout << "Found someone with id# 10 (this is correct)" <<endl;
    cout << '\t' << x->second->GetName() << ", age " << x->second->GetAge();
    cout << ", id# " << x->first << endl;
  }
  
  // WARNING: the following line leaks memory:
  personTable.insert(10, new Person("Bob",33));
  if (x == personTable.end()) 
    cout << "Nobody found with id# 10 (this is an error!)" <<endl;
  else {
    cout << "Found someone with id# 10 (this is correct)" <<endl;
    cout << '\t' << x->second->GetName() << ", age " << x->second->GetAge();
    cout << ", id# " << x->first << endl;
  }
  
  Person* newPeople[2] ;
  newPeople[0] = new Person("James",11);
  newPeople[1] = new Person("Jerry",29);
  for (int i=0; i < 2; i++) {
    x = personTable.InsertWithoutDuplication(newPeople[i]->GetAge(),
					     newPeople[i],&didInsert);
    if (!didInsert) {
      cout << "Tried to insert " << newPeople[i]->GetName() 
	   << " into table, but " << x->second->GetName() << endl
	   << " was already there and they both have id # " 
	   << x->first << endl;
      cout << "Deleting " << newPeople[i]->GetName() << endl;
      delete newPeople[i];
    } else {
      cout << "Inserted " << newPeople[i]->GetName() 
	   << " without duplication." << endl;
    }
  }
  
  MySimplePersonTable::iterator currentPerson;
  for (currentPerson = personTable.begin();
       currentPerson != personTable.end(); currentPerson++) {
    cout << currentPerson->second->GetName() << ": id#" 
	 << currentPerson->first << ", age: " 
	 << currentPerson->second->GetAge() << endl;
    delete currentPerson->second;
  }
  
  return;
}

int main() {
  DoTestsWithAdvancedHasher();
  DoTestsWithSimpleHasher();
}
