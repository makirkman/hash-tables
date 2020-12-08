/* * * * * * * * *
 * Dynamic hash table using cuckoo hashing, resolving collisions by switching
 * keys between two tables with two separate hash functions
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>
 */

#include  <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include   <time.h>

#include "cuckoo.h"

// an inner table represents one of the two internal tables for a cuckoo
// hash table. it stores two parallel arrays: 'slots' stores the keys and
// 'inuse' is a boolean indicating if a slot is filled
typedef struct inner_table {
	int64 *slots ;  // array of slots holding keys
	bool  *inuse ;  // array indicating if a slot is in use or not
	int    load  ;  // total number of inuse slots
	int    id    ;  // this table's id number (1 or 2)
} InnerTable ;

// a hash table which stores its keys in two inner tables
struct cuckoo_table {
	InnerTable *table1 ; // first table
	InnerTable *table2 ; // second table
	int			size   ; // size of each table
	int			time   ; // CPU time elapsed
} ;

/* * * *
 * helper functions
 */

// initialise the internal arrays of a single cuckoo inner table
static void initialise_in_table(InnerTable *table, int size) {
	assert(size < MAX_TABLE_SIZE && "error: table has grown too large!") ;

	table->slots = malloc((sizeof *table->slots) * size) ;
	assert(table->slots) ;
	table->inuse = malloc((sizeof *table->inuse) * size) ;
	assert(table->inuse) ;

	table->load = 0 ;
	int i ;
	for (i=0; i<size; i++) {
		table->inuse[i] = false ;
	}

}

// doubles cuckoo hash table size & rehashes its contents
static void double_cuckoo_table(CuckooHashTable *hash_table) {

	int o_size = hash_table->size ;
	int n_size = o_size * 2 ;

	// save the details of the old tables
	int64 *old_slots_table1 = hash_table->table1->slots ;
	bool  *old_inuse_table1 = hash_table->table1->inuse ;

	int64 *old_slots_table2 = hash_table->table2->slots ;
	bool  *old_inuse_table2 = hash_table->table2->inuse ;

	// resize each table
	initialise_in_table(hash_table->table1, n_size) ;
	initialise_in_table(hash_table->table2, n_size) ;
	hash_table->size = n_size ;

	// rehash old contents
	int i ;
	for (i = 0; i<o_size; i++) {
		if (old_inuse_table1[i]) {
			cuckoo_hash_table_insert(hash_table, old_slots_table1[i]) ;
		}
		if (old_inuse_table2[i]) {
			cuckoo_hash_table_insert(hash_table, old_slots_table2[i]) ;
		}
	}

	free(old_slots_table1) ;
	free(old_inuse_table1) ;
	free(old_slots_table2) ;
	free(old_inuse_table2) ;
}

// inserts a given key into a table & displaces the old key into the other table
//  if the current key has been tried before, doubles & rehashes both tables
static void in_table_insert(CuckooHashTable *hash_table, InnerTable *table,
  InnerTable *other_table, int64 cur_key, int64 init_key) {

	// double hash table & insert key if loop detected
	if (cur_key == init_key) {
		double_cuckoo_table(hash_table) ;
		cuckoo_hash_table_insert(hash_table, cur_key) ;
		return ;
	}

	/* table size is fine, perform insertion */
	int address ;
	if (table->id == 1) {
		address = h1(cur_key) % hash_table->size ;
	} else {
		address = h2(cur_key) % hash_table->size ;
	}

	// insert key and return if slot is empty
	if (!table->inuse[address]) {
		table->slots[address] = cur_key ;
		table->inuse[address] = true ;
		table->load++ ;
		return ;
	}

	// try to re-insert the old key in the other table
	int64 old_key = table->slots[address] ;
	table->slots[address] = cur_key ;

	in_table_insert(hash_table, other_table, table, old_key, init_key) ;
	/* ------------------------------------- */
}

/* * * *
 * main functions
 */

// initialises a cuckoo hash table with the given size
CuckooHashTable *new_cuckoo_hash_table(int size) {

	CuckooHashTable *hash_table = malloc((sizeof *hash_table) * size) ;
	assert(hash_table) ;

	/* initialise each inner table & their contents */
	hash_table->table1 = malloc((sizeof *hash_table->table1) * size) ;
	hash_table->table2 = malloc((sizeof *hash_table->table2) * size) ;
	initialise_in_table(hash_table->table1, size) ;
	initialise_in_table(hash_table->table2, size) ;
	hash_table->table1->id = 1 ;
	hash_table->table2->id = 2 ;
	/* -------------------------------------------- */

	// prepare high level details
	hash_table->size = size ;
	hash_table->time = 0 ;
	return hash_table ;
}


// frees all memory associated with a given cuckoo hash table
void free_cuckoo_hash_table(CuckooHashTable *hash_table) {
	assert(hash_table != NULL) ;

	free(hash_table->table1->slots) ;
	free(hash_table->table1->inuse) ;
	free(hash_table->table2->slots) ;
	free(hash_table->table2->inuse) ;

	free(hash_table->table1) ;
	free(hash_table->table2) ;

	free(hash_table) ;
}

// inserts a new key into a cuckoo hash table
// returns true if successful, false if the key was already present
bool cuckoo_hash_table_insert(CuckooHashTable *hash_table, int64 key) {
	assert(hash_table != NULL) ;
	int start_time = clock() ;

	int v = h1(key) % hash_table->size ;
	int w = h2(key) % hash_table->size ;

	/* insert key in table1 slot if empty */
	if (!hash_table->table1->inuse[v]) {
		hash_table->table1->slots[v] = key ;
		hash_table->table1->inuse[v] = true ;
		hash_table->table1->load++ ;

		hash_table->time += clock() - start_time ;
		return true ;
	}
	/* ---------------------------------- */

			   // 1st slot is taken
	/* check if key is already in either table */
	if (hash_table->table1->slots[v] == key) {
		hash_table->time += clock() - start_time ;
		return false ;
	}
	else if (hash_table->table2->inuse[w] &&
	  (hash_table->table2->slots[w] == key)) {
		hash_table->time += clock() - start_time ;
		return false ;
	}
	/* --------------------------------------- */

	/* if not, place key in table1 and move old key to table2 */
	int64 old_key = hash_table->table1->slots[v] ;
	hash_table->table1->slots[v] = key ;
	in_table_insert(hash_table, hash_table->table2,
		hash_table->table1, old_key, key) ;

	hash_table->time += clock() - start_time ;
	return true ;
	/* ------------------------------------------------------ */
}

// looks up whether a key is inside a cuckoo hash table
// returns true if found, false if not
bool cuckoo_hash_table_lookup(CuckooHashTable *hash_table, int64 key) {
	assert (hash_table != NULL) ;
	int start_time = clock() ;

	int v = h1(key) % hash_table->size ;
	int w = h2(key) % hash_table->size ;

	// look for the key in each slot
	if (hash_table->table1->inuse[v] &&
	  (hash_table->table1->slots[v] == key)) {
		hash_table->time += clock() - start_time ;
		return true ;
	}
	if (hash_table->table2->inuse[w] &&
	  (hash_table->table2->slots[w] == key)) {
		hash_table->time += clock() - start_time ;
		return true ;
	}
	else {
		hash_table->time += clock() - start_time ;
		return false ;
	}
}

// prints the contents of a cuckoo hash table to stdout
void cuckoo_hash_table_print(CuckooHashTable *hash_table) {
	assert(hash_table) ;
	printf("--- table size: %d\n", hash_table->size) ;

	// print header
	printf("                    table one         table two\n") ;
	printf("                  key | address     address | key\n") ;

	// print rows of each table
	int i ;
	for (i = 0; i < hash_table->size; i++) {

		// table 1 key
		if (hash_table->table1->inuse[i]) {
			printf(" %20llu ", hash_table->table1->slots[i]) ;
		} else {
			printf(" %20s ", "-") ;
		}

		// addresses
		printf("| %-9d %9d |", i, i) ;

		// table 2 key
		if (hash_table->table2->inuse[i]) {
			printf(" %llu\n", hash_table->table2->slots[i]) ;
		} else {
			printf(" %s\n",  "-") ;
		}
	}

	printf("--- end table ---\n") ;
}

// prints statistics about a cuckoo hash table to stdout
void cuckoo_hash_table_stats(CuckooHashTable *hash_table) {

	assert(hash_table != NULL) ;
	int total_load = hash_table->table1->load + hash_table->table2->load ;
	float seconds = hash_table->time * 1.0 / CLOCKS_PER_SEC ;

	printf("\n----- table stats -----\n") ;

	// print high level cuckoo table info
	printf("\n    --- overall ---\n") ;
	printf("CPU time spent:\t\t%.6f sec\n", seconds) ;
	printf("total size:\t\t%d slots\n", hash_table->size * 2) ;
	printf("    (%d slots in 2 tables)\n", hash_table->size) ;
	printf("total load:\t\t%d items\n", total_load) ;
	printf("total load factor:\t%.3f%%\n",
	  total_load * 100.0 / (hash_table->size * 2)) ;
	printf("    ---------------\n") ;

	// print internal table info
	printf("\n    ---  inner  ---\n") ;
	printf("table 1:\n") ;
	printf("  load:\t\t%d items\n", hash_table->table1->load) ;
	printf("  load factor:\t%.3f%%\n",
	  hash_table->table1->load * 100.0 / hash_table->size) ;

	printf("table 2:\n") ;
	printf("  load:\t\t%d items\n", hash_table->table2->load) ;
	printf("  load factor:\t%.3f%%\n",
	  hash_table->table2->load * 100.0 / hash_table->size) ;
	printf("    ---------------\n") ;
	printf("\n   --- end stats ---\n") ;
}
