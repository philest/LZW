// lzw.c -- Implementation of the LZW algorithim. 
// Phil Esterman 

#include "lzw.h"
#include "hash.h"


//return TRUE if same string, FALSE otherwise
// #define my_strcmp(str1, str2) (strcmp(str1,str2) == 0)

void test(void);

hash_table * encode(void);

void encode_test();

//print the string associated with code
void string_print(hash_table *h, int code);


int
main(int argc, const char* argv[])
{		
		//check whether called by encode (last six characters)
	if (strcmp((*argv+(strlen(argv[0]) - 6)), "encode") == 0)
		encode();

}



hash_table *encode(void)
{
	hash_table *table = hashCreate();

	int k; //char just read
	int code = EMPTYCODE; //code to prefix, or code of newly inserted
	entry *ent; 

	while((k = getchar()) != EOF) {
		if ((ent = hashLookup(table, code, k)))
			code = hashGetCode(ent); //it's in table, so get code
		else
		{	
			//put it in table
			printf("%8d\n", code);
			hashInsert(table, code, k, hashGetN(table));
			
			//then get the code
			ent = hashLookup(table, EMPTYCODE, k);
			code = hashGetCode(ent);
		}
	}

	if (code != EMPTYCODE) printf("%8d\n", code);

	hashPrintTable(table, true);
	return table;

}



void test()
{
	hash_table *h = hashCreate(); 
}

