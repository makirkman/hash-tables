/* * * * * * * * *
 * Main program:
 * reads command line options, runs a hash table interpreter
 *
 * created by Maxim Kirkman <max.kirkman94@gmail.com>, following Matt Farrugia
 */

#include   <stdio.h>
#include  <stdlib.h>
#include <stdbool.h>
#include  <string.h>
#include  <getopt.h>

#include "inthash.h"
#include "hashtbl.h"

/* cli options */
#define DEFAULT_SIZE 4
typedef struct options {
	TableType type ;
	int initial_size ;
} Options ;

Options get_options(int argc, char** argv) ;
/* ------------ */

/* interpreter commands */
#define INSERT 'i'
#define LOOKUP 'l'
#define PRINT  'p'
#define STATS  's'
#define HELP   'h'
#define QUIT   'q'
#define MAX_LINE_LEN 80

int get_command(char *operation, int64 *key) ;
/* -------------------- */

void run_interpreter(HashTable *table) ;

int main(int argc, char **argv) {
	// get command line options and create table with specified parameters
	Options options = get_options(argc, argv) ;
	HashTable *table = new_hash_table(options.type, options.initial_size) ;

	// start the interpreter loop
	run_interpreter(table) ;

	// quit
	free_hash_table(table) ;
	return 0 ;
}


// print out the valid operations
void print_operations() {
	printf(" %c number: insert 'number' into table\n",  INSERT) ;
	printf(" %c number: lookup is 'number' in table\n", LOOKUP) ;
	printf(" %c: print table\n", PRINT) ;
	printf(" %c: print stats\n", STATS) ;
	printf(" %c: quit\n", QUIT) ;
}


// run the interpreter
void run_interpreter(HashTable *table) {
	
	printf("enter a command (h for help):\n") ;
	
	char op ;
	int64 key ;
	
	// get and execute commands until 'quit'
	while (true) {

		// read a command, store results in op and key variables
		int argc = get_command(&op, &key) ;
		// no valid command entered, ignore
		if (argc < 1) {
			continue ; 
		}

		// execute the command
		switch (op) {
			case INSERT:
				// insert commands must have an argument
				if (argc < 2) {
					printf("syntax: %c number\n", INSERT) ;
				// perform the insertion
				} else {
					if (hash_table_insert(table, key)) {
						printf("%llu inserted\n", key) ;
					} else {
						printf("%llu already in table\n", key) ;
					}
				}
				break ;

			case LOOKUP:
				// lookup commands must have an argument
				if (argc < 2) {
					printf("syntax: %c number\n", LOOKUP) ;
				} else {
					// perform the lookup
					if (hash_table_lookup(table, key)) {
						printf("%llu found\n", key) ;
					} else {
						printf("%llu not found\n", key) ;
					}
				}
				break ;

			case PRINT:
				hash_table_print(table) ;
				break ;

			case STATS:
				hash_table_stats(table) ;
				break ;

			default:
				printf("unknown operation '%c'\n", op) ;
				// fall through
			case HELP:
				// list available options
				printf("available operations:\n") ;
				print_operations() ;
				break ;
				
			case QUIT:
				printf("exiting\n") ;
				return ;
		}
	}
}


// reads a line from stdin, parses it into an operation character and possibly
// a long long uinteger argument. store results in *operation and *key, resp.
//
// returns the number of tokens successfully read (e.g. 0 for none,
// 1 for operation only, 2 for both operation and integer)
// written by Matt Farrugia
int get_command(char *operation, int64 *key) {
	
	// read a line from stdin, up to MAX_LINE_LENGTH, into character buffer
	char line[MAX_LINE_LEN] ;
	fgets(line, MAX_LINE_LEN, stdin) ;
	// strip trailing newline
	line[strlen(line)-1] = '\0' ;

	// attempt to parse the line string into *operation and *key
	int argc = sscanf(line, "%c %llu", operation, key) ;
	// note: since llu is unsigned, a command like 'i -1' will overflow,
	// resulting in *key = 18446744073709551615 (2^64-1). this is a feature.
	
	// return the number of variables successfully read, as required
	return argc ;
}



// scans command line arguments for program options,
// prints usage info and exits if commands are missing or otherwise invalid
// written by Matt Farrugia
Options get_options(int argc, char** argv) {
	
	// create the Options structure with defaults
	Options options = { .type = NOTYPE, .initial_size = DEFAULT_SIZE } ;

	// scan inputs by flag
	char option ;
	while ((option = getopt(argc, argv, "t:s:")) != EOF) {
		switch (option) {
			// set hash table type
			case 't':
				options.type = strtotype(optarg) ;
				break ;
			// set hash table size
			case 's':
				options.initial_size = atoi(optarg) ;
				break ;
			default:
				break ;
		}
	}

	bool valid = true ;
		
	// check part validity
	if(options.type == NOTYPE) {
		fprintf(stderr,
			"please specify which table type to use, using the -t flag:\n") ;
		fprintf(stderr, " -t 0 or cuckoo:  cuckoo hash table\n") ;
		fprintf(stderr,
			" -t 1 or xtnbdln: n-key extendible hash table\n") ;
		fprintf(stderr, " -t 2 or xuckoo:  extendible cuckoo table\n") ;
		valid = false ;
	}

	// validate table size
	if(options.initial_size <= 0) {
		fprintf(stderr,
			"please specify initial table size (>0) using the -s flag\n") ;
		valid = false ;
	}

	if(!valid) {
		exit(EXIT_FAILURE) ;
	}

	return options ;
}
