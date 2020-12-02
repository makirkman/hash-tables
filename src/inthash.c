/* * * * * * * * *
 * Gash functions for 64-bit unsigned integers
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>
 */

#include <math.h>

#include "inthash.h"

// constants for respective hash function (arbitrary primes)
#define A1 899808677
#define B1 776533253
#define p1 2147483563

#define A2 879191233
#define B2 796929241
#define p2 2147483629

// first hash function
int h1(int64 k, int64 m) {
	return ((k * A1 + B1) % p1) % m ;
}

// second hash function
int h2(int64 k, int64 m) {
	return ((k * A2 + B2) % p2) % m ;
}