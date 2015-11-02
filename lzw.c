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

// #define COMPARE_TABLES 1

//when did you mark bits


void encode(int max_bits)
{
	int old_num_bits = 9; //starting with 256 + 2 codes 

	hash_table *table = hashCreate(POW_OF_2(max_bits));

	int k; //char just read
	int code = EMPTYCODE; //code to prefix, or code of newly inserted
	entry *ent; 
	int new_num_bits;

	//signal the max bits
	printf("%d:", max_bits);


	// #define SIGNAL 1

	#ifdef SIGNAL
		int currByte = 0;
	#endif


	while((k = getchar()) != EOF) {
		#ifdef SIGNAL
			currByte++;
		#endif


		if ((ent = hashLookup(table, code, k)) != NULL) //it's in table
			code = hashGetCode(ent); //it's in table, so get code
		else //new code!
		{	
			//write the prefix 
			#ifndef COMPARE_TABLES

				new_num_bits = hashGetNumBits(table);
				
				if (new_num_bits != old_num_bits) //signal an bit increase
					{
						putBits(old_num_bits, INC_BIT_CODE);

						#ifdef SIGNAL
						fprintf(stderr, "INC BIT signal at byte %d", currByte);
						#endif
					}

				putBits(new_num_bits, code);
			#endif

			#ifdef COMPARE_TABLES
				printf("code: -%d-\n", code);
			#endif


			//if room: put the new prefix, char pair in table
			if (!hashFull(table))
				hashInsert(table, code, k, hashGetN(table) + NUM_SPEC_CODES, 0);
			
			//make that final character the new prefix code
			ent = hashLookup(table, EMPTYCODE, k);
			code = hashGetCode(ent);

			old_num_bits = new_num_bits; //what was new is now old.
		}


	}
	#ifndef COMPARE_TABLES
		if (code != EMPTYCODE)
		{	
			new_num_bits = hashGetNumBits(table);
				
			if (new_num_bits != old_num_bits) //signal an bit increase
				putBits(old_num_bits, INC_BIT_CODE);

			putBits(new_num_bits, code);
			flushBits();
		}
	#endif

	#ifdef COMPARE_TABLES
		//printf("code: -%d-\n", code);
		hashPrintTable(table, true);
	#endif

}


/********************************************************
					DECODE FUNCTIONS
*********************************************************/

  //#define TEST 1
 //#define COMPARE_TABLES 1
 
 // #define COMPARE_TABLES 1
 // #define DEBUG 1					
void decode()
{	
	int currByte = 1;
	FILE *cmp_file = fopen("docs/sylvie.1", "r");

	int oldCode = EMPTYCODE, newCode, code, final_char;
	int max_bits, status; 

	int numBits = 9; // table full of 256 default one char codes + 2 spec char


	//read max bits signal!
	status = scanf("%d:", &max_bits);
	if (status < 1)
		DIE("%s", "scanf of max_bits failed");

	hash_table *table = hashCreate(POW_OF_2(max_bits));
	Stack kstack = stackCreate(POW_OF_2(max_bits)); 



			//read the next code in input
	while( (newCode = code = getBits(numBits)) != EOF)
	{	



		if (code == EMPTYCODE)
		{	
			DIE("%s", "this shouldnt happen!");
			//bit overflow! w/ 4096
			code = POW_OF_2(max_bits);
		}

		if (code == INC_BIT_CODE)
		{
			numBits++;
			continue;
		}

		assert(numBits <= max_bits);

		//#define TEST (1)
		//#define DEBUG (1)

		#ifdef TEST
			//printf("Byte: %d, Current code: %d\n", currByte, code);
			if (code > 4096)
				printf("Byte: %d, Current code: %d\n", currByte, code);

			if (currByte > 55658 && currByte < 55662)
			{
				int jew = 3;
				jew *= 2;
			}

		#endif

		entry *e;

		if (code == EMPTYCODE)
			WARN("Empty code found at byte %d", currByte);

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
		
		#ifdef DEBUG
			int real_char = (int)fgetc(cmp_file);
			if (real_char != final_char)
				DIE("sylvie.1 ERROR: In byte %d: actual file had: %d (ascii: %c)\ndecode gave: %d (ascii: %c)",
									currByte, real_char, (char)real_char, final_char, (char)final_char);
			//currByte++;
		#endif
			currByte++;

		#ifndef COMPARE_TABLES
		putchar(final_char);
		#endif


		while(!stackEmpty(kstack)) //write the string just read
		{						   //from prefix-dive
			int k = stackPop(kstack); 
			#ifdef DEBUG
				int real_char = (int)fgetc(cmp_file);
				if (real_char != k)
					DIE("ERROR: In byte %d: actual file had: %d (ascii: %c)\ndecode gave: %d (ascii: %c)",
										currByte, real_char, (char)real_char, k, (char)k);
				//currByte++;
			#endif
				currByte++;
		
			#ifndef COMPARE_TABLES
			putchar(k);
			#endif
		}

								              //insert the prefix (oldCode), 
		if (oldCode != EMPTYCODE && !hashFull(table))             //char (final_char) pair in table
			hashInsert(table, oldCode, final_char, hashGetN(table) + NUM_SPEC_CODES, 0);
		
		oldCode = newCode;


	}

	#ifdef COMPARE_TABLES

		#ifndef TEST
		hashPrintTable(table, true);
		#endif 

	#endif

	#ifdef TEST
	printf("end Byte: %d\n", currByte-1);
	#endif 

	// #ifdef DEBUG
	// 	flushBits();

	// 	int c;
	// 	while((c = (int)fgetc(cmp_file)) != EOF)
	// 	{
	// 		printf("Real sylvie file still had: byte %d, (ascii %c)\n", c, (char)c);
	// 	}
	// #endif

	

}




void test()
{

}

