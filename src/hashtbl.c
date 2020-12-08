/* * * * * * * * *
 * Interface for accessing and manipulating hash table data structures of
 *  any type
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>, following Matt Farrugia
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "hashtbl.h"

#include "tables/cuckoo.h"
#include "tables/xtndbln.h"
#include "tables/xuckoo.h"

// get a TableType constant from a string representation:
TableType strtotype(char *str) {
	if (strcmp("0", str) == 0 || strcmp("cuckoo",  str) == 0) {
		return CUCKOO ;
	}
	if (strcmp("1", str) == 0 || strcmp("xtndbln", str) == 0) {
		return XTNDBLN ;
	}
	if (strcmp("2", str) == 0 || strcmp("xuckoo",  str) == 0) {
		return XUCKOO ;
	}
	return NOTYPE ;
}

// a wrapper for a table of any type, also storing its type
struct table {
	TableType type  ;
	void *table ;
} ;

// initialise a hash table with the given paramaters and return its pointer
HashTable *new_hash_table(TableType type, int size) {
	
	// allocate space for the table wrapper
	HashTable *table = malloc(sizeof *table) ;
	assert(table) ;
	// store the type
	table->type = type ;

	// create and store the table itself
	switch (type) {
		case CUCKOO:
			table->table = new_cuckoo_hash_table(size) ;
			break ;
		case XTNDBLN:
			table->table = new_xtndbln_hash_table(size) ;
			break ;
		case XUCKOO:
			table->table = new_xuckoo_hash_table() ;
			break ;
		default:
			// unexpected table type - error
			free(table) ;
			return NULL ;
	}

	return table ;
}

// free all memory associated with a given table
void free_hash_table(HashTable *table) {
	assert(table != NULL) ;

	switch (table->type) {
		case CUCKOO:
			free_cuckoo_hash_table(table->table) ;
			break ;
		case XTNDBLN:
			free_xtndbln_hash_table(table->table) ;
			break ;
		case XUCKOO:
			free_xuckoo_hash_table(table->table) ;
			break ;
		default:
			break ;
	}

	// free the wrapper as well
	free(table) ;
}

// insert a new key into a table
// returns true if successful, false if the key was already present
bool hash_table_insert(HashTable *table, int64 key) {
	assert(table != NULL) ;

	switch (table->type) {
		case CUCKOO:
			return cuckoo_hash_table_insert(table->table, key) ;
		case XTNDBLN:
			return xtndbln_hash_table_insert(table->table, key) ;
		case XUCKOO:
			return xuckoo_hash_table_insert(table->table, key) ;
		default:
			return false ;
	}
}

// lookup whether a key is inside a table
// returns true if found, false if not
bool hash_table_lookup(HashTable *table, int64 key) {
	assert(table != NULL) ;

	switch (table->type) {
		case CUCKOO:
			return cuckoo_hash_table_lookup(table->table, key) ;
		case XTNDBLN:
			return xtndbln_hash_table_lookup(table->table, key) ;
		case XUCKOO:
			return xuckoo_hash_table_lookup(table->table, key) ;
		default:
			return false ;
	}
}

// print the contents of a table to stdout
void hash_table_print(HashTable *table) {
	assert(table != NULL) ;

	switch (table->type) {
		case CUCKOO:
			cuckoo_hash_table_print(table->table) ;
			break ;
		case XTNDBLN:
			xtndbln_hash_table_print(table->table) ;
			break ;
		case XUCKOO:
			xuckoo_hash_table_print(table->table) ;
			break ;
		default:
			break ;
	}
}


// print statistics about a table to stdout
void hash_table_stats(HashTable *table) {
	assert(table != NULL) ;

	switch (table->type) {
		case CUCKOO:
			cuckoo_hash_table_stats(table->table) ;
			break ;
		case XTNDBLN:
			xtndbln_hash_table_stats(table->table) ;
			break ;
		case XUCKOO:
			xuckoo_hash_table_stats(table->table) ;
			break ;
		default:
			break ;
	}
}