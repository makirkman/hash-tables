/* * * * * * * * *
 * Hash functions for 64-bit unsigned integers
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>, following Matt Farrugia
 */

#ifndef INTHASH_H
#define INTHASH_H

#include <stdint.h>

// the maximum allowable table size: 2^27
// a table of 64-bit integers (8 bytes) with this many entries
// would take up 2^27 * 8 bytes = 2^30 bytes = 1GB of memory
#define MAX_TABLE_SIZE 134217728

// unsigned 64-bit integer type
typedef uint64_t int64 ;

// first hash function
int h1(int64 k) ;

// second hash function
int h2(int64 k) ;

#endif