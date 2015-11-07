// lzw.h                                          
// Interface for lzw.c 
// Phil Esterman

#define _GNU_SOURCE
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define EMPTYCODE (0)
#define INC_BIT_CODE (1)
#define PRUNE_CODE (2)
#define DEC_BIT_CODE (3)

#define NUM_SPEC_CODES (4)

#define DEFAULT_MAX_BITS (12)
#define POW_OF_2(exp) (1 << exp)

// Write message to stderr using format FORMAT
#define WARN(format,...) fprintf (stderr, "Far: " format "\n", __VA_ARGS__)

// Write message to stderr using format FORMAT and exit.
#define DIE(format,...)  WARN(format,__VA_ARGS__), exit (EXIT_FAILURE)


//an entry in the hash table 
struct entry {
	struct entry *next_entry; //pointer used for chaining
	int prefix; //code for prefix
	int final_char; // last character
	int code; // this entry's code.
	int times_used; // for pruning: the times accessed
};
