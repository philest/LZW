// stack.c --  a stack for storing characters
// Phil Esterman

#include "stack.h"

struct stack {
	int n; // number of characters stored
	int size; // space for how many total characters
	int *data; // the chars stored 
};

Stack stackCreate(int size)
{
	Stack s = calloc(1, sizeof(*s));
	s->n = 0; // storing nothing
	s->size = size;
	s->data = calloc(size, sizeof(int)); // the character array

	return s;
}

void stackPush(Stack s, int k)
{
	if (s->n >= s->size) // the stack's full
		{	
			/** double the space! **/
			s->data = realloc(s->data, s->size * 2 * sizeof(int));
			s->size *= 2;
		}

	s->data[s->n] = k; // store k after all filled characters 
					   // (end of filled array). 
	s->n++;		
}

// Return the most recent character pushed onto Stack.
// Else, return -1 on failure.
int stackPop(Stack s)
{
	if (stackEmpty(s))
		return -1;
	else
	{
		int toRet = s->data[s->n - 1];
		s->data[s->n - 1] = 0; //remove it;
		s->n--; 
		return toRet;
	}

}

// Return true if empty, false if not
bool stackEmpty(Stack s)
{
	return s->n == 0;
}

void stackDelete(Stack s)
{
	free(s->data);
	free(s);
}

