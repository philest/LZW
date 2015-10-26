// stack.h --  Interface for a stack that stores characters
// Phil Esterman

#ifndef LZW
	#include <stdbool.h>
	#include <stdlib.h>
#endif

typedef struct stack * Stack;

// Return a newly created Stack with space for size chars
Stack stackCreate(int size);

// Push the character k onto the Stack
void stackPush(Stack s, int k);

// Return the most recent character pushed onto Stack.
// Else, return -1 on failure.
int stackPop(Stack s); 

// Return true if empty, false if not
bool stackEmpty(Stack s);

// Free all memory associated with the stack
void stackDelete(Stack s);

