// lzw.c -- Implementation of the LZW algorithim. 
// Phil Esterman 

#include "lzw.h"
#include "hash.h"
#include "code.h"
#include "stack.h"

#define LZW (1) // for stack.h modules

//return TRUE if same string, FALSE otherwise
// #define my_strcmp(str1, str2) (strcmp(str1,str2) == 0)

void test(void);

void encode(int max_bits);

void decode(void);

void encode_test();

int getPrefix(hash_table *h, int code);

int
main(int argc, const char* argv[])
{		

	int max_bits = DEFAULT_MAX_BITS;

		//check whether called by encode (last six characters)
	if (strcmp((*argv+(strlen(argv[0]) - 6)), "encode") == 0)
		encode(max_bits);
	else if (strcmp((*argv+(strlen(argv[0]) - 6)), "decode") == 0)
		decode();
	else
		test();
		//DIE("%s", "Must call by encode or decode");
}



/********************************************************
						ENCODE
*********************************************************/


void encode(int max_bits)
{
	hash_table *table = hashCreate(POW_OF_2(max_bits));

	int k; //char just read
	int code = EMPTYCODE; //code to prefix, or code of newly inserted
	entry *ent; 

	while((k = getchar()) != EOF) {
		if ((ent = hashLookup(table, code, k)) != NULL) //it's in table
			code = hashGetCode(ent); //it's in table, so get code
		
		else //new code!
		{	
			//write the prefix 
			putBits(hashGetNumBits(table), code);
			
			//if room: put the new prefix, char pair in table
			if (!hashFull(table))
				hashInsert(table, code, k, hashGetN(table), 0);
			
			//make that final character the new prefix code
			ent = hashLookup(table, EMPTYCODE, k);
			code = hashGetCode(ent);
		}


	}

	if (code != EMPTYCODE) putBits(hashGetNumBits(table), code);

	// hashPrintTable(table, true);

}


/********************************************************
					DECODE FUNCTIONS
*********************************************************/

void decode()
{	
	int max_bits; 

	int oldCode = EMPTYCODE, newCode, code, final_char; 

	hash_table *table = hashCreate(POW_OF_2(DEFAULT_MAX_BITS));
	Stack kstack = stackCreate(POW_OF_2(DEFAULT_MAX_BITS)); 

	int numBits = 8; // table full of 256 default one char codes

			//read the next code in input
	while( (newCode = code = getBits(numBits)) != EOF)
	{

		//#define TEST (1)

		#ifdef TEST
			printf("Current code: %d\n", code);
		#endif

		entry *e;

			// unknown code found!
		if ((e = hashCodeLookup(table, code)) == NULL)
		{
			stackPush(kstack, final_char);
			code = oldCode;
		}

		e = hashCodeLookup(table, code);
		if(e == NULL)
			 DIE("%s", "An unknown code-- and not kwkwk case-- was found\n");

		   //until the prefix is empty, accumulate chars
		while (e->prefix != EMPTYCODE)
		{
			stackPush(kstack, e->final_char);
			e = hashCodeLookup(table, e->prefix);
		} 


		final_char = hashGetChar(table, e->code);
		putchar(final_char);



		while(!stackEmpty(kstack))
		{
			int k = stackPop(kstack); 
			putchar(k);
		}

		if (oldCode != EMPTYCODE)
			hashInsert(table, oldCode, final_char, hashGetN(table), 0);
		
		oldCode = newCode;
		numBits = decodeHashGetNumBits(table);

	}

	// hashPrintTable(table, true);

}




void test()
{

}

