
#include "my_hash_map.H"

struct LameHasher {

  int operator()(const int & key)const {return GENERIC_HASH(key);}

  int SecondHashValue(const int & key)const {
    return 2*GENERIC_HASH(3*key) + 1;
  }

};

struct CompareInts {
  inline bool operator()(const int x, const int y) const {return x==y;}
};

typedef my_hash_map<int,int,LameHasher,CompareInts> TestTable;
