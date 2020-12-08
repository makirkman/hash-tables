/* * * * * * * * *
 * Dynamic hash table using a combination of extendible hashing and cuckoo
 * hashing with a single key per bucket, resolving collisions by switching keys 
 * between two tables with two separate hash functions, and growing the tables 
 * incrementally in response to cycles
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>
 */

#ifndef XUCKOO_H
#define XUCKOO_H

#include <stdbool.h>
#include "../inthash.h"

typedef struct xuckoo_table XuckooHashTable ;

// initialises an extendible cuckoo hash table
XuckooHashTable *new_xuckoo_hash_table() ;

// frees all memory associated with a given extendible cuckoo hash table
void free_xuckoo_hash_table(XuckooHashTable *hash_table) ;

// inserts a new key into an extendible cuckoo hash table
// returns true if successful, false if the key was already present
bool xuckoo_hash_table_insert(XuckooHashTable *hash_table, int64 key) ;

// looks up whether a key is inside an extendible cuckoo hash table
// returns true if found, false if not
bool xuckoo_hash_table_lookup(XuckooHashTable *hash_table, int64 key) ;

// prints the contents of an extendible cuckoo hash table to stdout
void xuckoo_hash_table_print(XuckooHashTable *table) ;

// prints statistics about an extendible cuckoo hash table to stdout
void xuckoo_hash_table_stats(XuckooHashTable *hash_table) ;

#endif
