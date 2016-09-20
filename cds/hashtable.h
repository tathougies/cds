#ifndef __FUZZY_HASHTABLE_H__
#define __FUZZY_HASHTABLE_H__

#include <stdlib.h>
#include <stdint.h>

typedef size_t hash_t;

typedef int(*KeyComparisonFn)(void *, void *);
typedef void*(*GetKeyFn)(void *);
typedef void(*FreeEntryFn)(void *);
typedef hash_t(*HashFn)(void *);
typedef struct {
  /* Compares keys */
  KeyComparisonFn compareKeys;
  /* Gets the key corresponding to an entry */
  GetKeyFn getKey;
  /* Hash an entry */
  HashFn hashKey;
  /* Release an entry */
  FreeEntryFn releaseEntry;

  size_t entrySz;
} HashTableOps;

typedef struct {
  size_t tableSz, tableAllocedSz;
  void **table;
} HashTable;

int InitHashTable(HashTableOps *ops, HashTable *ht);
void DestroyHashTable(HashTableOps *ops, HashTable *ht);

int HashTableReserveCapacity(HashTableOps *ops, HashTable *ht, size_t newSz);

/* Gets the entry in the hash table based on the description in ops */
void *HashTableGet(HashTableOps *ops, HashTable *ht, void *key);

/* Sets the entry in the hash table based on the description in ops.
 *
 * If oldEntry is not NULL, then store the pointer to the old entry in
 * *oldEntry.
 *
 * If oldEntry is NULL, then attempt to free the memory associated
 * with the old entry using the description in ops. */
int HashTableSet(HashTableOps *ops, HashTable *ht, void *newEntry, void **oldEntry);

/* Deletes the entry corresponding to key in the hash table (if any).
 *
 * If oldEntry is non-NULL, then oldEntry will be set to the deleted
 * entry.
 *
 * If oldEntry is NULL, and an entry is being deleted, then
 * use ops to release the memory.
 *
 * Returns 0 if an entry was deleted and -1 otherwise.
 */
int HashTableDelete(HashTableOps *ops, HashTable *ht, void *key, void **oldEntry);

#define FOR_EACH_ENTRY_IN_HASH_TABLE(var, ht)           \
  size_t i;                                             \
  for ( i = 0; i < (ht)->tableAllocedSz; ++i )          \
    if ( (var = (ht)->table[i]) )

/* Common hash and comparison functions */
int IntCompare(void *a, void *b);
hash_t IntHash(void *a);

#endif
