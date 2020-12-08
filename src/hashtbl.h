/* * * * * * * * *
 * Interface for accessing and manipulating hash table data structures of
 *  any type
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>, following Matt Farrugia
 */

#ifndef HASHTBL_H
#define HASHTBL_H

#include <stdbool.h>
#include "inthash.h"

// enum with the different types of hash table
typedef enum type {
	NOTYPE = -1, CUCKOO, XTNDBLN, XUCKOO
} TableType ;

// get a TableType constant from a string representation:
TableType strtotype(char *str) ;

typedef struct table HashTable ;

// initialise a hash table with the given paramaters and return its pointer
HashTable *new_hash_table(TableType type, int size) ;

// free all memory associated with a given table
void free_hash_table(HashTable *table) ;

// insert a new key into a table
// returns true if successful, false if the key was already present
bool hash_table_insert(HashTable *table, int64 key) ;

// lookup whether a key is inside a table
// returns true if found, false if not
bool hash_table_lookup(HashTable *table, int64 key) ;

// print the contents of a table to stdout
void hash_table_print(HashTable *table) ;

// print statistics about a table to stdout
void hash_table_stats(HashTable *table) ;

#endif