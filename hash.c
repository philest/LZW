// hash.c -- Hash table for storing (PREFIX, CHAR) pairs. 
//Implementation guided by James Aspnes, CPSC 223 Notes
// Phil Esterman 

#include "lzw.h"
#include "hash.h"


// //an entry in the hash table 
// struct entry {
// 	struct entry *next_entry; //pointer used for chaining
// 	int prefix; //code for prefix
// 	int final_char; // last character
// 	int code; // this entry's code.
// 	int times_used; // for pruning: the times accessed
// };

struct hash_table {
	struct entry **table; //array of pointers to entries, for chaining
	int n; /*number of elements stored*/
	int size; //the current size of the table

	struct entry *codeArray; //an array of the entries, indexed by code.  
};



//Hash function from Stan Eisenstat
/*returns a unique hashed value for prefix, char pair */
int
hashFunction(hash_table *h, int p, int k);

// Grow the hash table to the new size
hash_table *
hashGrow(hash_table *h, int new_size);

/* Return a newly created hash_table of the given size*/
hash_table *
hashInternalCreate(int size);

void string_print(hash_table *h, int code);


int 
hashFunction(hash_table *h, int p, int k) {
	return (((unsigned)(p) << CHAR_BIT) ^ ((unsigned) (k))) % h->size;
} 


hash_table *
hashInternalCreate(int size)
{
	hash_table *h;
	int i;

	h = malloc(sizeof(hash_table));
	h->n = 0; //no elts in table

	//space for every pointer
	h->table = malloc(sizeof(struct entry *) * size);
	for(i = 0; i < size; i++)
		h->table[i] = 0; //set each index in table to empty

	h->codeArray = calloc(size, sizeof(struct entry)); 

	h->size = size; 

	return h;
}

hash_table *
hashCreate(int size) {

	hash_table *h = hashInternalCreate(size);

	// initialize with each one character string 
	int i;
	for (i = 0; i < 256; i++)
		hashInsert(h, EMPTYCODE, i, h->n + NUM_SPEC_CODES, 0); // 0 times used

	return h;
}

void
hashDecodeUseUpdate(hash_table *h)
{
		/* == DECODE USE COUNT UPDATE ========== */
	//from highest code, at it's use counts to the prefix's. 
	
	for (int code = h->size - 1; code >= 256 + NUM_SPEC_CODES; code--)
	{
		struct entry *ent;
		struct entry *child_ent;

		ent = hashCodeLookup(h, code);
		assert(ent);
		assert(ent->prefix != EMPTYCODE);

		child_ent = hashCodeLookup(h, ent->prefix);
		assert(child_ent);

		//add the top layer's times used to its prefix's. 
		child_ent->times_used += ent->times_used;
	}

}


hash_table *
hashPrune(hash_table *h, int prune_bar, bool encode)
{	

	int last_entry_code, last_entry_prefix, last_entry_char;

	assert(hashGetN(h) == h->size - NUM_SPEC_CODES);

	//create a new table of old's size [with 1) 256 single char strings]
	hash_table *pruned_table = hashCreate(h->size);

	// for mapping from old code (index) to new code (value at index)
	int *code_translate = calloc(h->size, sizeof(int));

	for(int i = NUM_SPEC_CODES; i < 256+NUM_SPEC_CODES; i++)
		code_translate[i] = i; //maps to same value for 0-255 one-char strs


	// int *new_codes = calloc(h->size, sizeof(int));

	//include all strings with use counts at or above prune_bar [2) and 3)]
	for(int code = 256 + NUM_SPEC_CODES; code < h->size; code++)
		{	
			int actual_prefix; //with new code
			struct entry *ent; 
			ent = hashCodeLookup(h, code);
			assert(ent);

			if (ent->times_used >= prune_bar) //meets the threshold
				{	
					actual_prefix = code_translate[ent->prefix];
					assert(actual_prefix == EMPTYCODE || hashCodeLookup(pruned_table, actual_prefix));

					hashInsert(pruned_table, actual_prefix, ent->final_char,
							hashGetN(pruned_table) + NUM_SPEC_CODES, 0);

					code_translate[code] = hashGetN(pruned_table) + NUM_SPEC_CODES - 1;
						//the new code just assigned

					last_entry_code = hashGetN(pruned_table) + NUM_SPEC_CODES - 1;
					last_entry_prefix = actual_prefix;
					last_entry_char = ent->final_char;
				}

				// //if encode: delete the last entry
				// if (encode)
				// {
				// 	struct entry* last_entry = hashCodeLookup(pruned_table, last_entry_code);
				// 	assert(last_entry);


				// }

		}

	free(code_translate);

	hashDestroy(h);

	return pruned_table;
}



int
hashGetNumBits(hash_table *h)
{
	int expon = 1;

	//grow exponent until it can a bitfield of that length
	//can represent that number of codes
	while (POW_OF_2(expon) < h->n + NUM_SPEC_CODES)
		expon++;  

	return expon;
}

int
decodeHashGetNumBits(hash_table *h)
{
	assert(0);

	int expon = 1;	

	//grow exponent until it can a bitfield of that length
	//can represent that number of codes
	while (POW_OF_2(expon) <= h->n)
		expon++;  

	if (hashFull(h)) //when full, don't need
		expon--;	//to preemptively add a bit:
					//no more codes are coming.

	return expon;
}

/*malloc space for a new entry, then return it.*/
//NOTE: Does not set pointer to next
entry *create_entry(int prefix, int final_char, int code, int times_used)
{
	struct entry *new_entry = calloc(1, sizeof(*new_entry));

	new_entry->prefix = prefix;
	new_entry->final_char = final_char;
	new_entry->code = code;
	new_entry->times_used = times_used; //initialized to never used NOTE: Change
	new_entry->next_entry = 0;

	return new_entry; 
}


int
hashInsert(hash_table *h, int prefix, int final_char, int code, int times_used) {

	assert(code != EMPTYCODE);
	assert(code != INC_BIT_CODE);
	assert(code <= h->size);

	int hash_key; 
	struct entry *e;
	struct entry *e_for_array;

	//table full
	// if (h->n >= h->size)
	// 	h = hashGrow(h, POW_OF_2((hashGetNumBits(h) + 1)) );
	assert(h->n <= h->size);
										//need one more: still need to add.

	//if not already in table!, assert it! 
	if (hashLookup(h, prefix, final_char) == NULL)
	{
		// if (h->n >= h->size)
		// {
		// 	printf("%s", "Hash Table is full!");
		// 	return 0; // failure
		// }

		// if(h->n >= h->size) 
		// 	h = hashGrow(h);
		e = create_entry(prefix, final_char, code, times_used);

		hash_key = hashFunction(h, prefix, final_char);

		// chaining: insert element at front of that index's chain
		e->next_entry = h->table[hash_key];
		h->table[hash_key] = e;

		// insert in code array
		e_for_array = create_entry(prefix, final_char, code, times_used);
		h->codeArray[h->n] = *e_for_array;

		// free(e_for_array);

		h->n++; //one more entry in table
	}

	assert(hashLookup(h, prefix, final_char)); //ensure in table
	assert(hashCodeLookup(h, code)); //ensure in array //skip zeroth index!


	return 1; //success
}


// NOTE: Doesn't delete from array
int 
hashDelete(hash_table *h, int prefix, int final_char) {

	int hash_key = hashFunction(h, prefix,final_char);
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


// Return a pointer to the entry found the given code. 
// If not in the code array, return NULL.
entry * 
hashCodeLookup(hash_table *h, int code) {	

	assert(code < h->size); //valid codes // the last NUM_SPEC_CODE spaces are unfilled
	assert(code != EMPTYCODE);
	assert(code != INC_BIT_CODE);
	assert(code != PRUNE_CODE);
	assert(code >= NUM_SPEC_CODES);

	if (code < h->n + NUM_SPEC_CODES) return &(h->codeArray[code- NUM_SPEC_CODES]); //one less, b/c indexed by zero
	else return NULL; // not


	// assert(code < h->n);
	// return &(h->codeArray[code]);
}


// Return a pointer to the entry found with (prefix, final_char) pair. 
// If not in table, return NULL.
entry * 
hashLookup(hash_table *h, int prefix, int final_char) {

	int hash_key = hashFunction(h, prefix, final_char);
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

bool
hashFull(hash_table *h) {

	return h->n + NUM_SPEC_CODES >= h->size; 
}

bool
hashOneFromFull(hash_table *h)
{
	return h->n + NUM_SPEC_CODES + 1 >= h->size;
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

		// if(i < h->n) free(&(h->codeArray[i]));

	}

	free(h->table);
	free(h->codeArray);

	free(h);
}





int
hashGetN(hash_table *h) {

	return h->n;
}

int
hashGetCode(entry *e)
{
	return e->code;
}




void
hashPrintTable(hash_table *h, bool print_array)
{

	int i;
	struct entry * e;

	printf(" (Prefix | Char) | Code  |  Times USED | String \n");
    printf("--------   ------  -------   -------     ------ \n");

	for(i = 0; i < h->size; i++)
		{
			for ( e = h->table[i]; e != 0; e = e->next_entry)
				{	
					if (!print_array)
						{
							printf("%8d | %5d | %8d | %5d | ", 
								e->prefix, e->final_char, e->code, e->times_used);
							string_print(h, e->code);
							printf("\n");
						}
				
					assert(0 <= e->prefix && e->prefix < h->size); //prefix is a valid code
					assert(0 <= e->final_char && e->final_char < 256); //char is a valid char
				}

			// make sure code_array is proper
			e = &(h->codeArray[i]);	

				//don't print zero-intialized values
			if (print_array && (e->prefix != 0 || e->final_char != 0)) 
				{
					printf("%8d | %5d | %8d | %5d | ", 
						e->prefix, e->final_char, e->code, e->times_used);
					string_print(h, e->code);
					printf("\n");
				}

			assert(0 <= e->prefix && e->prefix < h->size); //prefix is a valid code
			assert(0 <= e->final_char && e->final_char < 256); //char is a valid char
		}

}

// Return the prefix code for a given table and code
int 
hashGetPrefix(hash_table *h, int code)
{
	entry *e = hashCodeLookup(h, code);
	assert(e);

	return e->prefix;
}

// Return the final character for a given given table and code
int 
hashGetChar(hash_table *h, int code)
{
	assert(code != EMPTYCODE);
	assert(code <= h->size);

	entry *e = hashCodeLookup(h, code);
	assert(e);

	return e->final_char;

}

void
hashIncrUse(hash_table *h, int code)
{
	assert(code != EMPTYCODE);
	assert(code <= h->size);

	entry *e = hashCodeLookup(h, code);
	assert(e);

	e->times_used++; //increment use count
}



void string_print(hash_table *h, int code)
{
		entry *e = hashCodeLookup(h, code);
		
		if (e->prefix != EMPTYCODE)
			string_print(h, e->prefix);

		if(e->final_char >= 32 && e->final_char <= 127)
			printf("%c ", (char)e->final_char); //print the character
		else
			printf("%d ", e->final_char);

}











