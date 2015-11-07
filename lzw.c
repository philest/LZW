// lzw.c -- Implementation of the LZW algorithim. 
// Phil Esterman 

#include "lzw.h"
#include "hash.h"
#include "/c/cs323/Hwk4/code.h"
#include "stack.h"

#define LZW (1) // for stack.h modules

#define DONT_PRUNE (-1)
#define NONE "none"

//return TRUE if same string, FALSE otherwise
// #define my_strcmp(str1, str2) (strcmp(str1,str2) == 0)

void test(char *file);

void encode(int max_bits, char*output_file, char*input_file, int prune_bar);

void decode(char*output_file);

void encode_test();

int getPrefix(hash_table *h, int code);

// Return the proper argument values in an array, 
// given the CL arguments args, 
// void readArgs(const char *argv[], int arg_vals[]);

int
main(int argc, const char* argv[])
{		

	// char *none = calloc(100, sizeof(char));
	// none = "none";


	//set default args!
	int max_bits = DEFAULT_MAX_BITS; //-m arg
	char input_file[PATH_MAX] = "none"; //-i arg 
	char output_file[PATH_MAX] = "none"; //-o arg
	int prune_bar = DONT_PRUNE; //-p arg

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
			prune_bar = atoi(argv[i+1]);

			if(strcmp(argv[i+1], "0") != 0 && prune_bar == 0)
				DIE("%s", "improper -p format: must be integer!");
		}	
			// anything not preceded by a flag!  
		else if ((i == 1 ) || !(strcmp(argv[i - 1], "-m") == 0 || strcmp(argv[i - 1], "-i") == 0 
				|| strcmp(argv[i - 1], "-o") == 0 || strcmp(argv[i - 1], "-p") == 0))
			DIE("%s", "Invalid argument! Not preceded by flag");

	}


		//check whether called by encode (last six characters)
	if (strcmp((*argv+(strlen(argv[0]) - 6)), "encode") == 0)
		encode(max_bits, output_file, input_file, prune_bar);
	else if (strcmp((*argv+(strlen(argv[0]) - 6)), "decode") == 0)
		decode(output_file);
	else
		test(input_file);
		// DIE("%s", "not designed to be accessed this way!");
}




/********************************************************
						TABLE IO
*********************************************************/



void dump_table(hash_table *table, char*fname)
{
	FILE *fout = fopen(fname, "w+");

	if (!fout)
		DIE("%s", "file already existed or open failed!");

	//for each element in table, put prefix, char, code
	// in PREF-CHAR-CODE
	//	  PREF-CHAR-CODE etc format


	for(int code = NUM_SPEC_CODES; code < (hashGetN(table) + NUM_SPEC_CODES); code++) //codes start at 1
	{
		int prefix = hashGetPrefix(table, code);
		int final_char = hashGetChar(table, code);

		fwrite(&prefix, sizeof(prefix), 1, fout);
		fwrite(&final_char, sizeof(final_char), 1, fout);

	}

	fclose(fout);

}

hash_table *read_table(char*fname, int max_bits)
{
	// int prefix;
	// int final_char;
	int status, code;

	int pref_finalchar[2] = {0,0};

	hash_table *new_table = hashCreate(POW_OF_2(max_bits));

	FILE *fin = fopen(fname, "r");

	if (!fin)
		DIE("%s", "file already existed or open failed!");


	code = NUM_SPEC_CODES; //codes start after special codes

	while((status = fread(pref_finalchar, sizeof(int), 2, fin)) == 2)
	{

		assert(code != EMPTYCODE);
		assert(code != INC_BIT_CODE);

		assert(pref_finalchar[1] < 256);

		hashInsert(new_table, pref_finalchar[0], pref_finalchar[1], code, 0);
		code++;
	}

	return new_table;
}







/********************************************************
						ENCODE
*********************************************************/

 // #define COMPARE_TABLES 1

//when did you mark bits
	// #define PTABLE 1
// #define COMPARE_TABLES 1

// #define B_TABLE 1


void encode(int max_bits, char*output_file, char *input_file, int prune_bar)
{
	int old_num_bits = 9; //starting with 256 + 2 codes 
	int new_num_bits;

	hash_table *table;

	if (strcmp(input_file, NONE) != 0) //read table from file -i!
		{
			table = read_table(input_file, max_bits);
			old_num_bits = hashGetNumBits(table);

			if (old_num_bits > max_bits)
				DIE("%s", "Table read in was bigger than max_bits allowed.");

		}
	else 
		table = hashCreate(POW_OF_2(max_bits));

	new_num_bits = old_num_bits; //sync to start

	int k; //char just read
	int code = EMPTYCODE; //code to prefix, or code of newly inserted
	entry *ent; 

	//in encoded output header:
	//signal the max bits and input file's name
	printf("%d %s %d\n", max_bits, input_file, prune_bar);

	//#define SIGNAL 1

	#ifdef SIGNAL
		int currByte = 0;
	#endif


	while((k = getchar()) != EOF) {
		#ifdef SIGNAL
			currByte++;
		#endif


		if ((ent = hashLookup(table, code, k)) != NULL) //it's in table
			{
				code = hashGetCode(ent); //it's in table, so get code
				hashIncrUse(table, code); //increment use count
			}
		else //new code!
		{	
			if(new_num_bits < max_bits || hashGetNumBits(table) < new_num_bits) // don't increase if reached maxbits
				new_num_bits = hashGetNumBits(table);

			//write the prefix 
			#ifndef COMPARE_TABLES
				
				if (new_num_bits > old_num_bits) //signal an bit increase
					{
						putBits(old_num_bits, INC_BIT_CODE);

						#ifdef SIGNAL
						fprintf(stderr, "INC BIT signal at byte %d", currByte);
						#endif
					}

				while(new_num_bits < old_num_bits)
				{
					putBits(old_num_bits, DEC_BIT_CODE);
					old_num_bits--;
				}

				putBits(new_num_bits, code);

			#endif

			// #ifdef COMPARE_TABLES
			// 	printf("code: -%d-\n", code);
			// #endif


			//if room: put the new prefix, char pair in table
			if (!hashFull(table))
				{
					hashInsert(table, code, k, hashGetN(table) + NUM_SPEC_CODES, 0);	
				}	

			//make that final character the new prefix code
			ent = hashLookup(table, EMPTYCODE, k);
			hashIncrUse(table, ent->code);

			code = hashGetCode(ent);

			// //increment that char's use count
			// hashIncrUse(table, code); //increment use count

			if (hashFull(table) && prune_bar != DONT_PRUNE) //JUST became full: prune now.
			{

					#ifdef B_TABLE
						hashPrintTable(table, true);
						exit(0);
					#endif

					table = hashPrune(table, prune_bar, true); //replace table with pruned table
					putBits(new_num_bits, PRUNE_CODE);

					#ifdef PTABLE
						hashPrintTable(table, true);
						exit(0);
					#endif

					// find it's new code in table?
			}


			old_num_bits = new_num_bits; //what was new is now old.
		}


	}

	#ifndef COMPARE_TABLES
		if (code != EMPTYCODE)
		{	
			new_num_bits = hashGetNumBits(table);
				
			if (new_num_bits > old_num_bits) //signal an bit increase
				putBits(old_num_bits, INC_BIT_CODE);

			putBits(new_num_bits, code);
			flushBits();
		}
	#endif

	#ifdef COMPARE_TABLES
		//printf("code: -%d-\n", code);
		hashPrintTable(table, true);
	#endif

	if(strcmp(output_file, NONE) != 0)
		dump_table(table, output_file);

}


/********************************************************
					DECODE FUNCTIONS
*********************************************************/

  //#define TEST 1
 // #define COMPARE_TABLES 1
  // #define COMPARE_TABLES 1
   // #define PTABLE 1
 // #define DEBUG 1		

 // #define WHERE 1 


void decode(char* output_file)
{	
	int currByte = 1;

	bool just_pruned = false;

	//bool just_pruned = false; //was pruned last pass?
//	#define DEBUG 1 

	#ifdef DEBUG
		char *comp_file = "C-Delaware";
		FILE *cmp_file = fopen(comp_file, "r");
	#endif

	int oldCode = EMPTYCODE, newCode, code, final_char;
	int max_bits, status, prune_bar; 

	char *input_file_name = calloc(PATH_MAX, sizeof(char));

	int numBits = 9; // table full of 256 default one char codes + 2 spec char

	int kwk[2] = {0,0}; // no kwk code to add

	//read max bits signal!
	status = scanf("%d %s %d\n", &max_bits, input_file_name, &prune_bar);
	if (status < 2)
		DIE("%s", "scanf of max_bits, input_file_name failed");
	
	hash_table *table;

	if (strcmp(input_file_name, NONE) != 0)
		{
			table = read_table(input_file_name, max_bits);
			numBits = hashGetNumBits(table);

			if (numBits > max_bits)
				DIE("%s", "table was bigger than max_bits allowed.");
		}
	else
		table = hashCreate(POW_OF_2(max_bits));

	Stack kstack = stackCreate(POW_OF_2(max_bits)); 


			//read the next code in input
	while( (newCode = code = getBits(numBits)) != EOF)
	{	

		#ifdef WHERE
			printf("%d-%d\n", numBits, code);
		#endif


		if (code == PRUNE_CODE && prune_bar != DONT_PRUNE)
			{	

				#ifdef B_TABLE
					hashPrintTable(table, true);
					exit(0);
				#endif

				table = hashPrune(table, prune_bar, false);
				#ifdef PTABLE
					hashPrintTable(table, true);
					exit(0);
				#endif

				just_pruned = true;

				continue;
			}

		if (code == EMPTYCODE)
		{	
			DIE("%s", "this shouldnt happen!");
			//bit overflow! w/ 4096
			code = POW_OF_2(max_bits);
		}

		if (code == INC_BIT_CODE)
		{
			if (numBits < max_bits) 
				numBits++;

			continue;
		}

		if (code == DEC_BIT_CODE)
		{
			if (numBits > 9)
				numBits--;
			
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




			// unknown code found! kwk case
		if ((e = hashCodeLookup(table, code)) == NULL)
		{
			stackPush(kstack, final_char); //add the last char to stack
			code = oldCode; //output the string rep'd by prefix code

			// remember the kwk signature
			kwk[0] = oldCode;
			kwk[1] = final_char;
		}
		// else // a valid code, update use count. 
		// 	hashIncrUse(table, code);


		e = hashCodeLookup(table, code); //find pair in table
	  	if(e == NULL)
		{
			DIE("%s", "An unknown code-- and not kwkwk case-- was found\n");
		}

		// if (e->prefix == EMPTYCODE)	//for single char strs
		// 	hashIncrUse(table, e->code);


		   //until the prefix is empty, accumulate chars
		while (e->prefix != EMPTYCODE)
		{	
			//update use count (efficient alg?)
			hashIncrUse(table, e->code);

			stackPush(kstack, e->final_char);
			e = hashCodeLookup(table, e->prefix); 
					// dive into prefix codes
		} 

		final_char = hashGetChar(table, e->code); //get the first char!
		
		#ifdef DEBUG
			int real_char = (int)fgetc(cmp_file);
			if (real_char != final_char)
				DIE("%s ERROR: In byte %d: actual file had: %d (ascii: %c)\ndecode gave: %d (ascii: %c)",
									comp_file, currByte, real_char, (char)real_char, final_char, (char)final_char);
			//currByte++;
		#endif
			currByte++;

		#ifndef COMPARE_TABLES
		putchar(final_char);	//increment code for the final k! 
		#endif
		hashIncrUse(table, final_char + NUM_SPEC_CODES); 

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
		if (oldCode != EMPTYCODE && !hashFull(table) && !just_pruned)             //char (final_char) pair in table
			{
				hashInsert(table, oldCode, final_char, hashGetN(table) + NUM_SPEC_CODES, 0);
				
				if (oldCode == kwk[0] && final_char == kwk[1])
					{
						hashIncrUse(table, hashGetN(table) + NUM_SPEC_CODES - 1);
						 //kwk just inserted
						
						//reset the values
						kwk[0] = 0;
						kwk[1] = 0;
					}

			}

    	if (just_pruned == true) //reset it.
    		just_pruned = false;

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

	if(strcmp(output_file, NONE) != 0)
		dump_table(table, output_file);


	free(input_file_name);

}




void test(char *file)
{
	if (strcmp(file, NONE) == 0)
		printf("im a jew!");

	printf("%s", file);
}

