// lzw.c -- Implementation of the LZW algorithim. 
// Phil Esterman 

#include "lzw.h"
#include "hash.h"
#include "code.h"
#include "stack.h"

#define LZW (1) // for stack.h modules

#define DONT_PRUNE (-1)

//return TRUE if same string, FALSE otherwise
// #define my_strcmp(str1, str2) (strcmp(str1,str2) == 0)

void test(void);

void encode(int max_bits);

void decode(void);

void encode_test();

int getPrefix(hash_table *h, int code);

// Return the proper argument values in an array, 
// given the CL arguments args, 
// void readArgs(const char *argv[], int arg_vals[]);

int
main(int argc, const char* argv[])
{		

	//set default args!
	int max_bits = DEFAULT_MAX_BITS; //-m arg
	char input_file[PATH_MAX] = "none"; //-i arg 
	char output_file[PATH_MAX] = "none"; //-o arg
	int prune = DONT_PRUNE; //-p arg

	/***** READ ARGS ******/

	for(int i = 1; i < argc; i++)
	{
			//it's the last arg, and a flag. 
		if 	((i + 1 == argc) &&
			(strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "-i") == 0 
				|| strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "-p") == 0))
				DIE("%s", "improper arg format!: ending with flag!");


		if (strcmp(argv[i], "-m") == 0)
		{
			
			max_bits = atoi(argv[i+1]);

			if(strcmp(argv[i+1], "0") != 0 && max_bits == 0)
				DIE("%s", "improper -m format: must be integer!");

			if (max_bits <= 8 || max_bits > 20)
				max_bits = DEFAULT_MAX_BITS;
		}

		else if (strcmp(argv[i], "-i") == 0)
		{
			strcpy(input_file, argv[i+1]);
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			strcpy(output_file, argv[i+1]);
		}
		else if (strcmp(argv[i], "-p") == 0)
		{
			prune = atoi(argv[i+1]);

			if(strcmp(argv[i+1], "0") != 0 && prune == 0)
				DIE("%s", "improper -p format: must be integer!");
		}	
			// anything not preceded by a flag!  
		else if ((i == 1 ) || !(strcmp(argv[i - 1], "-m") == 0 || strcmp(argv[i - 1], "-i") == 0 
				|| strcmp(argv[i - 1], "-o") == 0 || strcmp(argv[i - 1], "-p") == 0))
			DIE("%s", "Invalid argument! Not preceded by flag");

	}


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

	//signal the max bits
	printf("%d", max_bits);

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

	int oldCode = EMPTYCODE, newCode, code, final_char;
	int max_bits, status; 

	int numBits = 8; // table full of 256 default one char codes

	hash_table *table = hashCreate(POW_OF_2(DEFAULT_MAX_BITS));
	Stack kstack = stackCreate(POW_OF_2(DEFAULT_MAX_BITS)); 

	//read max bits signal!
	status = scanf("%d", &max_bits);
	if (status < 1)
		DIE("%s", "scanf of max_bits failed");

			//read the next code in input
	while( (newCode = code = getBits(numBits)) != EOF)
	{

		assert(numBits <= max_bits);

		// #define TEST (1)

		#ifdef TEST
			printf("Current code: %d\n", code);
		#endif

		entry *e;

			// unknown code found!
		if ((e = hashCodeLookup(table, code)) == NULL)
		{
			stackPush(kstack, final_char); //add the last char to stack
			code = oldCode; //output the string rep'd by prefix code
		}

		e = hashCodeLookup(table, code); //find pair in table
		if(e == NULL)
		{
			DIE("%s", "An unknown code-- and not kwkwk case-- was found\n");
		}

		   //until the prefix is empty, accumulate chars
		while (e->prefix != EMPTYCODE)
		{
			stackPush(kstack, e->final_char);
			e = hashCodeLookup(table, e->prefix); 
					// dive into prefix codes
		} 

		final_char = hashGetChar(table, e->code); //get the first char!
		putchar(final_char);


		while(!stackEmpty(kstack)) //write the string just read
		{						   //from prefix-dive
			int k = stackPop(kstack); 
			putchar(k);
		}

								              //insert the prefix (oldCode), 
		if (oldCode != EMPTYCODE && !hashFull(table))             //char (final_char) pair in table
			hashInsert(table, oldCode, final_char, hashGetN(table), 0);
		
		oldCode = newCode;
		numBits = decodeHashGetNumBits(table);

	}

}




void test()
{

}

