/* * * * * * * * *
 * Dynamic hash table using a combination of extendible hashing and cuckoo
 * hashing with a single key per bucket, resolving collisions by switching keys 
 * between two tables with two separate hash functions, and growing the tables 
 * incrementally in response to cycles
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>
 */

#include  <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include   <time.h>

#include "xuckoo.h"

#define FIRST_COUNT_MAX 20000
#define FINAL_COUNT_MAX 21000

// a bucket stores a single key, or is empty
// it also knows how many bits are shared between possible keys, and the first 
// table address that references it
typedef struct bucket {
	int		   id ; // a unique id for this bucket, equal to the first address
				    // in the table which points to it
	int		depth ; // how many hash value bits are being used by this bucket
	bool	 full ; // does this bucket contain a key
	int64	  key ; // the key stored in this bucket
} Bucket ;

// an inner table is an extendible hash table with an array of slots pointing 
// to buckets holding up to 1 key, along with some information about the number 
// of hash value bits to use for addressing
typedef struct inner_table {
	Bucket **buckets ;  // array of pointers to buckets
	int		size ;      // how many entries in the table of pointers (2^depth)
	int		depth ;     // how many bits of the hash value to use (log2(size))
	int		nkeys ;     // how many keys are being stored in the table
	int		nbuckets ;  // total number of buckets in table
	int		id ;        // this table's id number (1 or 2)
} InnerTable ;

// a xuckoo hash table is just two inner tables for storing inserted keys
struct xuckoo_table {
	InnerTable *table1 ;
	InnerTable *table2 ;
	int			  time ; // CPU time elapsed
} ;

// macro to calculate the rightmost n bits of a number x
#define rightmostnbits(n, x) (x) & ((1 << (n)) - 1)

/* * * *
 * helper functions
 */

// creates a new empty bucket with first_address as its id
static Bucket *new_bucket (int first_address, int depth) {

	Bucket *bucket = malloc(sizeof *bucket) ;
	assert(bucket) ;

	bucket->id = first_address ;
	bucket->depth = depth ;
	bucket->full = false ;

	return bucket ;
}

// initialises an empty InnerTable to be used in a larger XuckooHashTable
static void initialise_in_table(InnerTable *table) {
	assert(table->size < MAX_TABLE_SIZE &&
	  "error: table has grown too large!") ;

	table->buckets = malloc(sizeof *table->buckets) ;
	assert(table->buckets) ;

	table->buckets[0] = new_bucket(0, 0) ;

	table->size = 1 ;
	table->depth = 0 ;
	table->nkeys = 0 ;
	table->nbuckets = 1 ;
	table->id = 0 ;
}

// after splitting a bucket & removing its key, reinserts that key into table
static void reinsert(InnerTable *table, int64 key) {
	int address ;
	/* find if key was in table1 or table2 */
	if (table->id == 1) {
		address = rightmostnbits(table->depth, h1(key)) ;
	} else {
		address = rightmostnbits(table->depth, h2(key)) ;
	}
	/* ----------------------------------- */

	table->buckets[address]->key = key ;
	table->buckets[address]->full = true ;
}

// doubles the table of bucket pointers, duplicating pointers from 1st
//  half of table into 2nd
// once a new array of pointers is made, removes all keys from the innner table
//  and inserts them again into the hash_table
static void double_inner_table(XuckooHashTable *hash_table, InnerTable *table) {

	int size = table->size * 2 ;
	assert(size < MAX_TABLE_SIZE && "error: table has grown too large!") ;

	// create new array of double the number of bucket pointers
	table->buckets = realloc(table->buckets, (sizeof *table->buckets) * size) ;
	assert (table->buckets) ;
	// copy the pointers down the array
	int i ;
	for (i=0; i<table->size; i++) {
		table->buckets[table->size + i] = table->buckets[i] ;
	}

	// increase table size & depth
	table->size = size ;
	table->depth++ ;

	// remove & reinsert all keys in newly doubled table
	for (i=table->size-1; i>=0; i--) {
		if (table->buckets[i]->full && table->buckets[i]->id == i) {
			table->buckets[i]->full = false ;
			table->nkeys-- ;
			xuckoo_hash_table_insert(hash_table, table->buckets[i]->key) ;
		}
	}
}

// splits the bucket in a table at address, grows table if necessary
static void split_xuck_bucket(XuckooHashTable *hash_table, InnerTable *table, int address) {

	// check if table growth is needed
	if (table->buckets[address]->depth == table->depth) {
		double_inner_table(hash_table, table) ;
	}

	/* create new bucket and update depths of both */
	Bucket *o_bucket = table->buckets[address] ;
	int depth = o_bucket->depth ;
	int first_address = o_bucket->id ;

	int new_depth = depth+1 ;
	o_bucket->depth = new_depth ;

	// new first address is 1 bit plus old first address
	int new_first_address = 1 << depth | first_address ;
	Bucket *n_bucket = new_bucket(new_first_address, new_depth) ;
	table->nbuckets++ ;
	/* ------------------------------------------- */

	/* redirect every second address from old bucket to new bucket
		using joining of prefix & suffix to construct address      */

	// suffix is 1 bit followed by previous bucket bit address
	int bit_address = rightmostnbits(depth, first_address) ;
	int suffix = (1 << depth) | bit_address ;

	// prefix is all bitstrings of length equal to the difference
	//   between the new bucket depth & the table depth
	int max_pref = 1 << (table->depth - new_depth) ;
	int prefix ;

	for (prefix=0; prefix<max_pref; prefix++) {
		// construct each address by joining prefix & suffix
		int a = (prefix << new_depth) | suffix ;
		// redirect this address in table to point to new bucket
		table->buckets[a] = n_bucket ;
	}
	/* ----------------------------------------------------------- */
	
	// remove and reinsert the key
	o_bucket->full = false ;
	reinsert(table, o_bucket->key) ;
}

// inserts key into table
// if the address is already taken, bumps the pre-existing key to the other
//  table, continuing this process recursively.
// uses a count variable to guess whether it has made too many recursive calls,
//  and if so, doubles the table
static void in_table_insert(XuckooHashTable *hash_table, InnerTable *table,
  InnerTable *other_table, int64 key, int count) {

	count++ ;
	/* find hash & address depending on which table was passed */
	int hash ;
	if (table->id == 1) {
		hash = h1(key) ;
	} else {
		hash = h2(key) ;
	}
	int address = rightmostnbits(table->depth, hash) ;
	/* ------------------------------------------------------- */

	// if the address is free, insert the key immediately
	if (!table->buckets[address]->full) {
		table->buckets[address]->key = key ;
		table->buckets[address]->full = true ;
		table->nkeys++ ;
		return ;
	}

	// a key is already present, insert anyway & store the old key
	int64 old_key = table->buckets[address]->key ;
	table->buckets[address]->key = key ;

	/* if count reaches a lower limit AND is at a bucket with
		more potential pointers, split bucket                 */
	if (count >= FIRST_COUNT_MAX &&
		(table->buckets[address]->depth != table->depth)) {
		split_xuck_bucket(hash_table, table, address) ;
	}
	/* ------------------------------------------------------ */

	/* if count reaches a larger limit, split current
		bucket regardless of potential pointers       */
	if (count >= FINAL_COUNT_MAX) {
		split_xuck_bucket(hash_table, table, address) ;
	}
	/* ---------------------------------------------- */

	// try to re-insert the old key in the other table
	in_table_insert(hash_table, other_table, table, old_key, count) ;
}

/* * * *
 * main functions
 */

// initialises an extendible cuckoo hash table
XuckooHashTable *new_xuckoo_hash_table() {
	XuckooHashTable *hash_table = malloc((sizeof *hash_table)) ;
	assert(hash_table) ;

	/* initialise each inner table & their contents */
	hash_table->table1 = malloc((sizeof *hash_table->table1)) ;
	hash_table->table2 = malloc((sizeof *hash_table->table2)) ;
	initialise_in_table(hash_table->table1) ;
	hash_table->table1->id = 1 ;
	initialise_in_table(hash_table->table2) ;
	hash_table->table2->id = 2 ;
	/* -------------------------------------------- */

	hash_table->time = 0 ;
	return hash_table ;
}

// frees all memory associated with a given extendible cuckoo hash table
void free_xuckoo_hash_table(XuckooHashTable *hash_table) {
	assert(hash_table != NULL) ;

	/* work backwards freeing each bucket in each table */
	int i ;
	for (i=hash_table->table1->size-1; i>=0; i--) {
		if (hash_table->table1->buckets[i]->id == i) {
			free(hash_table->table1->buckets[i]) ;
		}
	}
	for (i=hash_table->table2->size-1; i>=0; i--) {
		if (hash_table->table2->buckets[i]->id == i) {
			free(hash_table->table2->buckets[i]) ;
		}
	}
	/* ------------------------------------------------ */
	
	/* free the buckets array, tables, & hash_table */
	free(hash_table->table1->buckets) ;
	free(hash_table->table2->buckets) ;

	free(hash_table->table1) ;
	free(hash_table->table2) ;

	free(hash_table) ;
	/* -------------------------------------------- */
}

// inserts a new key into an extendible cuckoo hash table
// returns true if successful, false if the key was already present
bool xuckoo_hash_table_insert(XuckooHashTable *hash_table, int64 key) {
	assert(hash_table != NULL) ;
	int start_time = clock() ;

	// create addresses
	int address_1 = rightmostnbits(hash_table->table1->depth, h1(key)) ;
	int address_2 = rightmostnbits(hash_table->table2->depth, h2(key)) ;

	/* check if key is already in either table */
	if (hash_table->table1->buckets[address_1]->full &&
	  (hash_table->table1->buckets[address_1]->key == key)) {
		hash_table->time += clock() - start_time ;
		return false ;
	}
	else if (hash_table->table2->buckets[address_2]->full &&
	  (hash_table->table2->buckets[address_2]->key == key)) {
		hash_table->time += clock() - start_time ;
		return false ;
	}
	/* --------------------------------------- */

	/* insert in table with smallest number of keys */
	if (hash_table->table1->nkeys <= hash_table->table2->nkeys) {
		in_table_insert(hash_table, hash_table->table1,
		  hash_table->table2, key, 0) ;
	} else {
		in_table_insert(hash_table, hash_table->table2,
		  hash_table->table1, key, 0) ;
	}

	hash_table->time += clock() - start_time ;
	return true ;
		/* -------------------------------------------- */
}


// looks up whether a key is inside an extendible cuckoo hash table
// returns true if found, false if not
bool xuckoo_hash_table_lookup(XuckooHashTable *hash_table, int64 key) {
	assert(hash_table) ;
	int start_time = clock() ;

	// calculate table addresses for this key
	int address_1 = rightmostnbits(hash_table->table1->depth, h1(key)) ;
	int address_2 = rightmostnbits(hash_table->table2->depth, h2(key)) ;

	bool found = false ;
	if (hash_table->table1->buckets[address_1]->full) {
		found = hash_table->table1->buckets[address_1]->key == key ;
	}
	if (hash_table->table2->buckets[address_2]->full && !found) {
		found = hash_table->table2->buckets[address_2]->key == key ;
	}

	hash_table->time += clock() - start_time ;
	return found ;
}


// prints the contents of an extendible cuckoo hash table to stdout
void xuckoo_hash_table_print(XuckooHashTable *table) {
	assert(table != NULL) ;

	printf("--- table ---\n") ;

	// loop through the two tables, printing them
	InnerTable *innertables[2] = {table->table1, table->table2} ;
	int t ;
	for (t = 0; t < 2; t++) {
		// print header
		printf("table %d\n", t+1) ;

		printf("  table:               buckets:\n") ;
		printf("  address | bucketid   bucketid [key]\n") ;
		
		// print table and buckets
		int i ;
		for (i = 0; i < innertables[t]->size; i++) {
			// table entry
			printf("%9d | %-9d ", i, innertables[t]->buckets[i]->id) ;

			// if this is the first address at which a bucket occurs, print it
			if (innertables[t]->buckets[i]->id == i) {
				printf("%9d ", innertables[t]->buckets[i]->id) ;
				if (innertables[t]->buckets[i]->full) {
					printf("[%llu]", innertables[t]->buckets[i]->key) ;
				} else {
					printf("[ ]") ;
				}
			}
			printf("\n") ;
		}
	}
	printf("--- end table ---\n") ;
}


// prints statistics about an extendible cuckoo hash table to stdout
void xuckoo_hash_table_stats(XuckooHashTable *hash_table) {
	
	assert(hash_table != NULL) ;
	int total_size = hash_table->table1->size + hash_table->table2->size ;
	int total_buckets = hash_table->table1->nbuckets +
	  hash_table->table2->nbuckets ;
	int total_keys = hash_table->table1->nkeys + hash_table->table2->nkeys ;
	float seconds = hash_table->time * 1.0 / CLOCKS_PER_SEC ;

	printf("\n----- table stats -----\n") ;

	// print high level cuckoo table info
	printf("\n    --- overall ---\n") ;
	printf("CPU time spent   :\t%.6f sec\n", seconds) ;
	printf("total size       :\t%d potential slots\n", total_size) ;
	printf("total keys       :\t%d\n", total_keys) ;
	printf("total buckets    :\t%d\n", total_buckets) ;
	printf("total space usage:\t%.3f%%\n", total_keys * 100.0 /
	  total_size) ;
	printf("    ---------------\n") ;

	// print internal table info
	printf("\n    ---  inner  ---\n") ;
	printf("table 1:\n") ;
	printf("  size       :\t%d slots\n", hash_table->table1->size) ;
	printf("  keys       :\t%d\n", hash_table->table1->nkeys) ;
	printf("  buckets    :\t%d\n", hash_table->table1->nbuckets) ;
	printf("  space usage:\t%.3f%%\n", hash_table->table1->nkeys * 100.0 /
	  hash_table->table1->size) ;

	printf("table 2:\n") ;
	printf("  size   :\t%d slots\n", hash_table->table2->size) ;
	printf("  keys   :\t%d\n", hash_table->table2->nkeys) ;
	printf("  buckets:\t%d\n", hash_table->table2->nbuckets) ;
	printf("  space usage:\t%.3f%%\n", hash_table->table2->nkeys * 100.0 /
	  hash_table->table2->size) ;
	printf("    ---------------\n") ;
	printf("\n   --- end stats ---\n") ;

	return ;
}
