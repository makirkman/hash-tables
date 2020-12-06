/* * * * * * * * *
 * Dynamic hash table using extendible hashing with multiple keys per bucket,
 * resolving collisions by incrementally growing the hash table
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>
 */

#ifndef XTNDBLN_H
#define XTNDBLN_H

#include <stdbool.h>
#include "../inthash.h"

typedef struct xtndbln_table XtndblNHashTable ;

// initialise an extendible hash table with the given keys per bucket
XtndblNHashTable *new_xtndbln_hash_table(int bucketsize) ;

// free all memory associated with a given extendible hash table
void free_xtndbln_hash_table(XtndblNHashTable *table) ;

// insert a new key into an extendible hash table
// returns true if successful, false if the key was already present
bool xtndbln_hash_table_insert(XtndblNHashTable *table, int64 key) ;

// lookup whether a key is inside an extendible table
// returns true if found, false if not
bool xtndbln_hash_table_lookup(XtndblNHashTable *table, int64 key) ;

// print the contents of an extendible hash table to stdout
void xtndbln_hash_table_print(XtndblNHashTable *table) ;

// print statistics about an extendible hash table to stdout
void xtndbln_hash_table_stats(XtndblNHashTable *table) ;

#endif