// hash.c -- Hash table for storing (PREFIX, CHAR) pairs. 
//Implementation guided by James Aspnes, CPSC 223 Notes
// Phil Esterman 

#include "hash.h"
#include "lzw.h"

#define SIZE (4096) //2^12, where 12 is the maxbits of a code


//an entry in the hash table 
struct entry {
	struct entry *next_entry; //pointer used for chaining
	int prefix; //code for prefix
	int final_char; // last character
	int code; // this entry's code.
	int times_used; // for pruning: the times accessed
};

struct hash_table {
	struct entry **table; //array of pointers to entries, for chaining
	int n; /*number of elements stored*/
	int size; //the current size of the table
};


/* 	TODO hashPrintTable 
		 */


//Hash function from Stan Eisenstat
/*returns a unique hashed value for prefix, char pair */
int
hashFunction(int p, int k);

/* Grow the hashtable by a multiple of 2*/
hash_table *
hashGrow(hash_table *h);

/* Return a newly created hash_table of the given size*/
hash_table *
hashInternalCreate(int size);



int 
hashFunction(int p, int k) {
	return (((unsigned)(p) << CHAR_BIT) ^ ((unsigned) (k))) % SIZE;
} 


hash_table *
hashInternalCreate(int size)
{
	hash_table *h;
	int i;

	h = malloc(sizeof(hash_table));
	h->n = 0; //no elts in table

	//space for entry pointer
	h->table = malloc(sizeof(struct entry *) * size);

	h->size = size; 

	for(i = 0; i < size; i++)
	{
		h->table[i] = 0; //set each index in table to empty
	}

	return h;
}

hash_table *
hashCreate(void) {

	hash_table *h = hashInternalCreate(SIZE);

	// initialize with each one character string 
	int i;
	for (i = 0; i < 256; i++)
		hashInsert(h, EMPTYCODE, i, h->n); // 0 times used

	return h;
}


//NOTE: DOES NOT TRANSFER TIMES_USED
hash_table *
hashGrow(hash_table *h) {

	hash_table *h2; // new table to create
	hash_table swap; // container for destroying old table
	struct entry *e; // iterator for entries
	int i;

	int new_size = h->size * 2; //double the size

	h2 = hashInternalCreate(new_size);

	for(i = 0; i < h->size; i++) //traverse table
	{
		for(e = h->table[i]; e != 0; e = e->next_entry) //traverse chain
		{
			/*insert old entrys into new hash_table*/
			hashInsert(h2, e->prefix, e->final_char, e->code); //TODO insert times
		}
	}

	//swap h2 and h1, then destroy h2
	swap = *h;
	*h = *h2;
	*h2 = swap;

	hashDestroy(h2);

	return h; //new table
}


int
hashInsert(hash_table *h, int prefix, int final_char, int code) {

	int hash_key; 
	struct entry *e;

	//table full
	if (h->n >= h->size)
	{
		printf("%s", "Hash Table is full!");
		return 0; // failure
	}

	// if(h->n >= h->size) 
	// 	h = hashGrow(h);

	e = malloc(sizeof(struct entry)); // the new pair

	//initialize the entry pair
	e->prefix = prefix;
	e->final_char = final_char;
	e->code = code;
	e->times_used = 666; //NOTE CHANGE THIS

	hash_key = hashFunction(prefix, final_char);

	// chaining: insert element at front of that index's chain
	e->next_entry = h->table[hash_key];
	h->table[hash_key] = e;

	h->n++; //one more entry in table

	return 1; //success
}



int 
hashDelete(hash_table *h, int prefix, int final_char) {

	int hash_key = hashFunction(prefix,final_char);
	struct entry* e; // entry to delete
	struct entry* prev; // for patching up chain

	prev = 0; /*new: initialize*/

	e = h->table[hash_key];
		while(e) 
		{	
			// match
			if(e->prefix == prefix && e->final_char == final_char)
			{
				if (prev) // patch previous to next
					prev->next_entry = e->next_entry;
				else
					h->table[hash_key] = e->next_entry;
			
				free(e);
				h->n--; 

				return 1; /*sucess*/
			}

			// iterate
			prev = e;
			e = e->next_entry; 
		}

		return 0; /*error: not found*/

}


// Return a pointer to the entry found with (prefix, final_char) pair. 
// If not in table, return NULL.
entry * 
hashLookup(hash_table *h, int prefix, int final_char) {

	int hash_key = hashFunction(prefix, final_char);
	struct entry *e; // entry iterator

	for(e = h->table[hash_key]; e != 0; e = e->next_entry)
	{
		// the match
		if (e->prefix == prefix && e->final_char == final_char)
			return e;
	}

	// no match found
	return NULL;
}



void
hashDestroy(hash_table *h) {

	int i;
	struct entry * e;
	struct entry * next;

	for(i = 0; i < h->size; i++)
	{
		for ( e = h->table[i]; e != 0; e = next)
		{
			next = e->next_entry;
			free(e);
		}
	}

	free(h->table);
	free(h);
}


int
hashGetN(hash_table *h) {

	return h->n;
}


void
hashPrintTable(hash_table *h)
{

	int i;
	struct entry * e;

	printf(" (Prefix | Char) | Code    | String \n");
    printf("--------   ------  -------   ------\n");

	for(i = 0; i < h->size; i++)
		for ( e = h->table[i]; e != 0; e = e->next_entry)
			printf("%8d | %5d | %8d | %s\n", 
				e->prefix, e->final_char, e->code, "haven't done it");

}







