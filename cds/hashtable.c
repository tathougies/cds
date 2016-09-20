#include <string.h>

#include "hashtable.h"

#define HASH_TABLE_INITIAL_SZ 8
/* Allow the table to be 60% filled at most */
#define HASH_TABLE_FILL_RATIO 0.6
#define HASH_TABLE_LARGE_PRIME (2654435761LL)

int InitHashTable(HashTableOps *ops, HashTable *ht) {
  int err;

  ht->tableSz = ht->tableAllocedSz = 0;
  ht->table = NULL;

  err = HashTableReserveCapacity(ops, ht, HASH_TABLE_INITIAL_SZ);
  if ( err < 0 )
    return -1;

  return 0;
}

void DestroyHashTable(HashTableOps *ops, HashTable *ht) {
  size_t i;

  for (i = 0; i < ht->tableSz; ++i) {
    void *entry = ht->table[i];
    if ( entry )
      ops->releaseEntry(entry);
  }

  free(ht->table);
}

int HashTableReserveCapacity(HashTableOps *ops, HashTable *ht, size_t newSz) {
  if ( newSz > ht->tableAllocedSz ) {
    void **oldTable = ht->table;

    /* TODO need to re-hash the keys as well */
    ht->table = malloc(newSz * sizeof(void *));
    if ( !ht->table ) {
      ht->table = oldTable;
      return -1;
    }
    memset(ht->table, 0, newSz * sizeof(void *));
    ht->tableAllocedSz = newSz;
    ht->tableSz = 0;

    if ( oldTable ) {
      size_t i;
      for (i = 0; i < ht->tableSz; ++i)
        HashTableSet(ops, ht, ht->table[i], NULL);
    }

    return 0;
  } else
    return 0;
}

void *HashTableGet(HashTableOps *ops, HashTable *ht, void *key) {
  hash_t hash = ops->hashKey(key);

  void *entry;
  int cmpRes;

  goto fetchEntry;
  do {
    hash ++; /* If we're repeating, then get a new hash */

  fetchEntry:
    /* Get the entry at the current hash index */
    entry = ht->table[hash % ht->tableAllocedSz];

    if ( entry )
      cmpRes = ops->compareKeys(ops->getKey(entry), key);
  } while ( entry && cmpRes != 0 );

  /* If we couldn't find an entry there, or the comparison result
   * shows the keys were not equal, then we return NULL */
  if ( !entry || cmpRes != 0 )
    return NULL;
  else
    return entry;
}

int HashTableSet(HashTableOps *ops, HashTable *ht, void *newEntry, void **oldEntry) {
  void *newEntryKey = ops->getKey(newEntry);
  hash_t hash = ops->hashKey(newEntryKey);

  void *entry;
  int cmpRes, err;

  goto fetchEntry;
  do {
    hash ++; /* If we're repeating, then get a new hash */

  fetchEntry:
    entry = ht->table[hash % ht->tableAllocedSz];

    if ( entry )
      cmpRes = ops->compareKeys(ops->getKey(entry), newEntryKey);
  } while ( entry && cmpRes != 0 );

  if ( !entry ) {
    /* We're adding an entry into the table */
    ht->table[hash % ht->tableAllocedSz] = newEntry;
    ht->tableSz++;

    if ( ((double) ht->tableSz) / ((double) ht->tableAllocedSz) >
         HASH_TABLE_FILL_RATIO ) {

      /* If it's over-filled, then reserve extra capacity */
      err = HashTableReserveCapacity(ops, ht, ht->tableAllocedSz * 2);
      if ( err < 0 )
        return -1;
    }

    return 0;
  } else {
    /* We're replacing an old entry with a new one */
    if ( oldEntry )
      *oldEntry = entry;
    /* If we have no place to put the old entry, then try to release it */
    else
      ops->releaseEntry(entry);

    ht->table[hash % ht->tableAllocedSz] = newEntry;
    return 0;
  }
}

/* Checks if the next hash entry in the chain hashes to the same
 * hash. If so, move that entry into this one, and recurse. */
static void _HashTableReplace(HashTableOps *ops, HashTable *ht, hash_t origHash, hash_t entryBucket) {
  hash_t nextEntryBucket = entryBucket + 1;
  void *nextEntry = ht->table[nextEntryBucket % ht->tableAllocedSz];

  if ( nextEntry ) {
    /* If there is a next entry, check what its hash would be */
    hash_t nextEntryHash = ops->hashKey(ops->getKey(nextEntry));
    /* If the next entry's hash matches our original hash, then it can be moved into this bucket */
    if ( nextEntryHash == origHash ) {
      ht->table[entryBucket % ht->tableAllocedSz] = nextEntry;
      _HashTableReplace(ops, ht, origHash, nextEntryBucket);
    } else {
      /* The next entry is for another hash, so we can clear this bucket */
      ht->table[entryBucket % ht->tableAllocedSz] = NULL;
    }
  } else {
    /* If there is no next entry, then this bucket can be cleared */
    ht->table[entryBucket % ht->tableAllocedSz] = NULL;
  }
}

int HashTableDelete(HashTableOps *ops, HashTable *ht, void *key, void **oldEntry) {
  hash_t origHash = ops->hashKey(key);
  hash_t hash = origHash;

  void *entry;
  int cmpRes;

  goto fetchEntry;
  do {
    hash ++;

  fetchEntry:
    entry = ht->table[hash % ht->tableAllocedSz];

    if ( entry )
      cmpRes = ops->compareKeys(ops->getKey(entry), key);
  } while ( entry && cmpRes != 0 );

  if ( entry ) {
    /* An entry was found. Set oldEntry if necessary, or delete this entry */
    if ( oldEntry )
      *oldEntry = entry;
    else
      ops->releaseEntry(entry);

    /* Adjust the table size and move any entries that were displaced by this entry into position */
    ht->tableSz--;

    _HashTableReplace(ops, ht, origHash, hash);
    return 0;
  } else {
    /* No entry was found for the given key. Do nothing */
    if ( oldEntry )
      *oldEntry = NULL;
    return -1;
  }
}

/* Hash and comparison functions */
int IntCompare(void *a_, void *b_) {
  int *a = (int *)a_, *b = (int *)b_;
  return *a - *b;
}

hash_t IntHash(void *a_)
{
  int *a = (int *)a_;
  return ((hash_t) *a) * HASH_TABLE_LARGE_PRIME;
}
