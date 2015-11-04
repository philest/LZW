// hash.h -- Interface for hash table implementation of string table
// Phil Esterman 
#include <stdbool.h>


typedef struct hash_table hash_table;
typedef struct entry entry; 


/********************************************************
				HASH TABLE FUNCTIONS (ENCODE)
*********************************************************/


/* Return a newly created hash_table,
   initialized with every one character string */
hash_table *
hashCreate(int size); 

/* Insert pair (prefix, char) into the table.
   Return 0 on failure, 1 on success. */
int
hashInsert(hash_table *h, int prefix, int final_char, int code, int times_used);

/* Remove the first entry with (prefix, final_char) pair from
   the hash table. Return 1 if success, 0 if not in hash_table  */
int 
hashDelete(hash_table *h, int prefix, int final_char);

// Return a pointer to the entry found with (prefix, final_char) pair. 
// If not in table, return NULL.
entry * 
hashLookup(hash_table *h, int prefix, int final_char);

/* free all mememory allocated to the table h */
void
hashDestroy(hash_table *h); 

/* return the number of elements currently in hash table h*/
int
hashGetN(hash_table *h); 

/* return the code of the given entry */
int
hashGetCode(entry *e); 

/* Print all entries within the hash table */
void
hashPrintTable(hash_table *h, bool print_array);

// Return true if table is full, false if not full
bool
hashFull(hash_table *h);


/********************************************************
				ARRAY & ENTRY FUNCTIONS (DECODE)
*********************************************************/

// Return a pointer to the entry found the given code. 
// If not in the code array, return NULL.
entry * 
hashCodeLookup(hash_table *h, int code);

// Return the prefix code for a given table and code
int 
hashGetPrefix(hash_table *h, int code);

// Return the final character for a given given table and code
int 
hashGetChar(hash_table *h, int code);


/********************************************************
				MISC
*********************************************************/

//Return the #bits needed to represent all codes in table
int
hashGetNumBits(hash_table *h);

//Return the #bits needed to represent all codes in table
//for decode!
int
decodeHashGetNumBits(hash_table *h);


void string_print(hash_table *h, int code);







