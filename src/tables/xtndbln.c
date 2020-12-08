/* * * * * * * * *
 * Dynamic hash table using extendible hashing with multiple keys per bucket,
 * resolving collisions by incrementally growing the hash table
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>
 */

#include  <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include   <time.h>

#include "xtndbln.h"

// macro to calculate the rightmost n bits of a number x
#define rightmostnbits(n, x) (x) & ((1 << (n)) - 1)

// a bucket stores an array of keys
// it also knows how many bits are shared between possible keys, and the first 
// table address that references it
typedef struct xtndbln_bucket {
	int id ;        // a unique id for this bucket, equal to the first address
                    // in the table which points to it
	int depth ;     // number of hash value bits being used by this bucket
	int nkeys ;     // number of keys currently contained in this bucket
	int64 *keys ;   // the keys stored in this bucket
} Bucket ;

typedef struct stats {
	int nbuckets ;  // number of distinct buckets does the table point to
	int nkeys ;     // number of keys being stored in the table
	int time ;      // CPU time elapsed to insert/lookup keys in this table
} Stats ;

// a hash table is an array of slots pointing to buckets holding up to 
// bucketsize keys, along with some information about the number of hash value 
// bits to use for addressing
struct xtndbln_table {
	Bucket **buckets ;  // array of pointers to buckets
	int size ;          // number of entries in the table of pointers (2^depth)
	int depth ;         // how many bits of the hash value to use (log2(size))
	int bucketsize ;    // maximum number of keys per bucket
	Stats stats ;
} ;

/* * * *
 * helper functions
 */

// creates a new empty bucket with first_address as its id
static Bucket *new_bucket(int first_address, int depth, int bucketsize) {
	Bucket *bucket = malloc(sizeof *bucket) ; 
	assert(bucket) ;

	bucket->id = first_address ;
	bucket->depth = depth ;
	bucket->nkeys = 0 ;
	
	bucket->keys = malloc((sizeof *bucket->keys) * bucketsize) ;
	assert(bucket->keys) ;

	return bucket ;
}

// doubles the table of bucket pointers, duplicating pointers from 1st
//  half of table into 2nd
static void double_xn_table(XtndblNHashTable *table) {

	int size = table->size * 2 ;
	assert (size < MAX_TABLE_SIZE && "error: table has grown too large!") ;

	// create new array of double the number of bucket pointers
	table->buckets = realloc(table->buckets, (sizeof *table->buckets) * size) ;
	assert (table->buckets) ;
	// copy the pointers down the array
	int i ;
	for (i=0; i<table->size; i++) {
		table->buckets[table->size + i] = table->buckets[i] ;
	}

	// increase recorded size & depth
	table->size = size ;
	table->depth++ ;
}

// reinserts a key into an extendible hash table
//  for use only when a bucket has been split & its keys removed
static void reinsert_key(XtndblNHashTable *table, int64 key) {
	int address = rightmostnbits(table->depth, h1(key)) ;
	int b_nkeys = table->buckets[address]->nkeys ;

	table->buckets[address]->keys[b_nkeys] = key ;
	table->buckets[address]->nkeys++ ;
}

// splits the bucket in an extendible table at address, grows table if necessary
static void split_xn_bucket(XtndblNHashTable *table, int address) {

	// check if table growth is needed
	if (table->buckets[address]->depth == table->depth) {
		double_xn_table(table) ;
	}

	/* create new bucket and update depths of both */
	Bucket *o_bucket = table->buckets[address] ;
	int depth = o_bucket->depth ;
	int first_address = o_bucket->id ;

	int new_depth = depth + 1 ;
	o_bucket->depth = new_depth ;

	// new first address is 1 bit plus old first address
	int new_first_address = 1 << depth | first_address ;
	Bucket *n_bucket = new_bucket(new_first_address, new_depth,
	  table->bucketsize) ;
	table->stats.nbuckets++ ;
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

	/* reinsert keys from old bucket into table */
	int64 key ;
	int i ;
	int b_nkeys = o_bucket->nkeys ;
	o_bucket->nkeys = 0 ;
	for (i=0; i<b_nkeys; i++) {
		key = o_bucket->keys[i] ;
		reinsert_key(table, key) ;
	}
	/* ------------------------------------- */
}

/* * * *
 * main functions
 */

// initialises an extendible hash table with the given keys per bucket
XtndblNHashTable *new_xtndbln_hash_table(int bucketsize) {
	XtndblNHashTable *table = malloc(sizeof *table) ;
	assert(table) ;

	/* initialise internal table data */
	table->bucketsize = bucketsize ;
	table->size = 1 ;
	table->buckets = malloc(sizeof *table->buckets) ;
	assert(table->buckets) ;
	table->buckets[0] = new_bucket(0, 0, bucketsize) ;
	table->depth = 0 ;
	/* ------------------------------ */

	/* initialise table stats */
	table->stats.nbuckets = 1 ;
	table->stats.nkeys = 0 ;
	table->stats.time = 0 ;
	/* ---------------------- */

	return table ;
}

// frees all memory associated with a given extendible hash table
void free_xtndbln_hash_table(XtndblNHashTable *table) {
	assert(table) ;

	// iterate backwards freeing each bucket by their 1st reference
	int i ;
	for (i=table->size-1; i>=0; i--) {
		if (table->buckets[i]->id == i) {
			free(table->buckets[i]) ;
		}
	}

	// free the buckets array & the table
	free(table->buckets) ;
	free(table) ;
}

// inserts a new key into an extendible hash table
// returns true if successful, false if the key was already present
bool xtndbln_hash_table_insert(XtndblNHashTable *table, int64 key) {
	assert (table) ;
	int start_time = clock() ;
	
	// calculate the table address
	int hash = h1(key) ;
	int address = rightmostnbits(table->depth, hash) ;

	/* check if key is already present */
	int i ;
	int b_nkeys = table->buckets[address]->nkeys ;

	if (b_nkeys > 0) {
		for (i=0; i<b_nkeys; i++) {
			if (table->buckets[address]->keys[i] == key) {
				table->stats.time += clock() - start_time;
				return false ;
			}
		}
	}
	/* ------------------------------- */

	/* make space in table if bucket is full */
	while (table->buckets[address]->nkeys == table->bucketsize) {
		split_xn_bucket(table, address) ;
		address = rightmostnbits(table->depth, hash) ;
	}
	b_nkeys = table->buckets[address]->nkeys ;
	/* ------------------------------------- */

	/* space is available, insert key */
	table->buckets[address]->keys[b_nkeys] = key ;
	table->buckets[address]->nkeys++ ;
	table->stats.nkeys++ ;
	/* ------------------------------ */

	table->stats.time += clock() - start_time ;
	return true ;
}

// looks up whether a key is inside an extendible hash table
// returns true if found, false if not
bool xtndbln_hash_table_lookup(XtndblNHashTable *table, int64 key) {
	assert(table) ;

	int start_time = clock() ;

	/* calculate table address for this key and look through that bucket */
	int address = rightmostnbits(table->depth, h1(key)) ;

	if (table->buckets[address]->nkeys > 0) {
		int i ;
		for (i=0; i<table->bucketsize; i++) {
			if (table->buckets[address]->keys[i] == key) {
				table->stats.time += clock() - start_time ;
				return true ;
			}
		}
	}
	/* ----------------------------------------------------------------- */

	table->stats.time += clock() - start_time ;
	return false ;
}

// prints the contents of an extendible hash table to stdout
void xtndbln_hash_table_print(XtndblNHashTable *table) {
	assert(table);
	printf("--- table size: %d\n", table->size);

	// print header
	printf("  table:               buckets:\n");
	printf("  address | bucketid   bucketid [key]\n");
	
	// print table and buckets
	int i;
	for (i = 0; i < table->size; i++) {
		// table entry
		printf("%9d | %-9d ", i, table->buckets[i]->id);

		// if this is the first address at which a bucket occurs, print it now
		if (table->buckets[i]->id == i) {
			printf("%9d ", table->buckets[i]->id);

			// print the bucket's contents
			printf("[");
			for(int j = 0; j < table->bucketsize; j++) {
				if (j < table->buckets[i]->nkeys) {
					printf(" %llu", table->buckets[i]->keys[j]);
				} else {
					printf(" -");
				}
			}
			printf(" ]");
		}
		// end the line
		printf("\n");
	}

	printf("--- end table ---\n");
}

// prints statistics about an extendible hash table to stdout
void xtndbln_hash_table_stats(XtndblNHashTable *table) {
	assert(table) ;

	printf("\n----- table stats -----\n") ;

	// print table info
	printf("current table size:\t%d\n", table->size) ;
	printf("number of keys    :\t%d\n", table->stats.nkeys) ;
	printf("number of buckets :\t%d\n\n", table->stats.nbuckets) ;
	printf("space usage factor:\t%.3f%%\n", table->stats.nkeys * 100.0 /
	  (table->size * table->bucketsize));
	printf("bucket size       :\t%d\n", table->bucketsize) ;

	// calculate print time details
	float seconds = table->stats.time * 1.0 / CLOCKS_PER_SEC ;
	printf("CPU time spent    :\t%.6f sec\n", seconds) ;
	
	printf("   --- end stats ---\n") ;
}