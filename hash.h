
#define EMPTYCODE (0)

typedef struct hash_table hash_table;
typedef struct entry entry; 

/* Return a newly created hash_table,
   initialized with every one character string */
hash_table *
hashCreate(void); 

/* Insert pair (prefix, char) into the table.
   Return 0 on failure, 1 on success. */
int
hashInsert(hash_table *h, int prefix, int final_char, int code);

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


/* Print all entries within the hash table */
void
hashPrintTable(hash_table *h);




