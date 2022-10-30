#ifndef DF_H
#define DF_H
#include <cstdint>
#include "list.h"
/* Type definition for a tuple */
class tuple {
  int64_t key;      // value (R.a)
  List payload;  // rowIDs

 public:
  int64_t getKey() { return key; }
  List getPayload() { return payload; }

  tuple(int64_t key, int64_t payload) {
    this->key = key;
    this->payload.append(payload);
  }

  tuple() = default;
};

/*
 * Type definition for a relation.
 * It consists of an array of tuples and a size of the relation.
 */
class relation {
  tuple *tuples;
  uint64_t num_tuples;

 public:
  relation(tuple *tuples, uint64_t num_tuples) {
    this->num_tuples = num_tuples;
    this->tuples = tuples;
  }
  tuple &operator[](int64_t index) { return tuples[index]; }
  uint64_t getAmount() { return num_tuples; }
};

/*
 * Type definition for result.
 * It consists of an array of tuples and a size of the relation.
 */
class result {
  tuple *tuples;
  uint64_t num_tuples;
};

/* Partitioned Hash Join */
result *PartitionedHashJoin(relation *relR, relation *relS);
#endif