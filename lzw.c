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

void encode(void);

void decode(void);

void encode_test();

int getPrefix(hash_table *h, int code);

int mygetBits(void);

int
main(int argc, const char* argv[])
{		
		//check whether called by encode (last six characters)
	if (strcmp((*argv+(strlen(argv[0]) - 6)), "encode") == 0)
		encode();
	else if (strcmp((*argv+(strlen(argv[0]) - 6)), "decode") == 0)
		decode();
	else
		test();
		//DIE("%s", "Must call by encode or decode");
}



/********************************************************
						ENCODE
*********************************************************/


void encode(void)
{
	hash_table *table = hashCreate(POW_OF_2(DEFAULT_MAX_BITS));

	int k; //char just read
	int code = EMPTYCODE; //code to prefix, or code of newly inserted
	entry *ent; 

	while((k = getchar()) != EOF) {
		if ((ent = hashLookup(table, code, k)))
			code = hashGetCode(ent); //it's in table, so get code
		else
		{	
			//put it in table
			//printf("%d\n", code);
			putBits(hashGetNumBits(table), code);
			hashInsert(table, code, k, hashGetN(table), 0);
			
			//then get the code
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

	int oldCode = EMPTYCODE, newCode, code, final_char = 666; 

	hash_table *table = hashCreate(POW_OF_2(DEFAULT_MAX_BITS));
	Stack kstack = stackCreate(POW_OF_2(DEFAULT_MAX_BITS)); 

	int numBits = 8;
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
			 DIE("%s", "And unknown code-- and not kwkwk case-- was found\n");

		

		   //until the prefix is empty, accumulate chars
		while (e->prefix != EMPTYCODE)
		{
			stackPush(kstack, e->final_char);
			e = hashCodeLookup(table, e->prefix);
		} 


		final_char = hashGetChar(table, e->code);
		putchar(final_char);


		// #ifdef TEST
		// 	printf("%d, ascii: %c\n", final_char, (char)final_char); 
		// #else
		// 	if( (final_char >= 32 && final_char <= 127) || (final_char == 10 || final_char==9) )
		// 		printf("%c", (char)final_char); //print the character
		// 	else
		// 		printf("%d", final_char);
		// #endif

		while(!stackEmpty(kstack))
		{
			int k = stackPop(kstack); 
			putchar(k);
			// #ifdef TEST
			// 	printf("%d, ascii: %c\n", k, (char)k); 
			// #else
			// 	if( (k >= 32 && k <= 127) || (k== 10 || k==9))
			// 		printf("%c", (char)k); //print the character
			// 	else
			// 		printf("%d", k);
			// #endif
		}

		if (oldCode != EMPTYCODE)
			hashInsert(table, oldCode, final_char, hashGetN(table), 0);
		
		oldCode = newCode;
		numBits = decodeHashGetNumBits(table);

	}

	// hashPrintTable(table, true);

}


// int mygetBits(void)
// {
// 	int code; 

// 	if (scanf ("%d\n", &code) < 1)              // Demand code or assume no more
// 		return EOF;
// 	else
// 		return code;
// }

void test()
{

}

