#include "lzw.h"
#include "array.h"

//an entry in the array code table 
struct entry {
	struct entry *next_entry; //pointer used for chaining
	int prefix; //code for prefix
	int final_char; // last character
	int code; // this entry's code.
	int times_used; // for pruning: the times accessed
};


/*malloc space for a new entry, then return it.*/
//NOTE: Does not set pointer to next
entry *create_entry(int prefix, int final_char, int code)
{
	struct entry *new_entry = malloc(sizeof(*new_entry));

	new_entry->prefix = prefix;
	new_entry->final_char = final_char;
	new_entry->code = code;
	new_entry->times_used = 0; //initialized to never used NOTE: Change
	new_entry->next_entry = 0;

	return new_entry; 
}

void list_add(list *my_list, char *name, char *value)
{
	entry *new_entry; 
	new_entry = create_entry(name, value);


	//if it's the first
	if (list_empty(my_list))
		new_entry->next = 0;
	else 
		new_entry->next = *my_list; 	//make the new entry the first,
									// thus pointing to the former first.

	*my_list = new_entry; //set the list's head to the new entry
}


list *list_create()
{
	struct entry **head; 
	head = malloc(sizeof(*head));

	*head = 0; //initialize

	return head;
}

//delete the entry with this name from the my_list
//return -1 on error, 1 on success; and assumes only one instance. 
int list_delete(list *head, char *name)
{	
	struct entry *elt;

	struct entry *entry_to_kill; 

	if (list_empty(head))
		return -1;

	//check case of one elt: it's a match, and has no next
	// if ( strcmp(name, (*head)->name) == 0 &&
	// 		 (*head)->next == 0)
	// {
	// 	//destroy it all
	// 	list_destroy(head);
	// 	return 1;

	// }	//check case that the elt to remove is the head!
	if ( strcmp(name, (*head)->name) == 0 )
	{
			entry_to_kill = *head; //a pointer to the former head
			*head = entry_to_kill->next; // head should point to a POINTER to the 2nd elt.
			free(entry_to_kill->value);
			free(entry_to_kill->name);
			free(entry_to_kill);

			return 1;
	}
	else
	{
		for(elt = *head; elt != 0; elt = elt->next)
		{	
			//look one ahead, so you can tweak pointers
			if( strcmp(elt->next->name, name) == 0 )
			{
				entry_to_kill = elt->next; 
				elt->next = entry_to_kill->next; //rewire past it
				free(entry_to_kill->name);
				free(entry_to_kill->value);
				free(entry_to_kill);

				return 1;
			}

		}

		return -1; 

	}

}


int list_empty(list *head)
{

	if (*head == 0)
		return TRUE;
	else
		return FALSE;
} 

								//adapted from
								/*Break Interview*/
void list_destroy(list *head)	/*source:  breakinterview.com/write
								-a-function-to-delete-a-linked-list*/	
{
	struct entry *current = *head;
	struct entry *next; 

	while(current != NULL)
		{
			next = current->next;
			free(current->name);
			free(current->value);
			free(current);
			current = next;
		}
		*head = NULL;

		free(head);
}


void list_print(list *head)
{	 
	entry *elt;

	for(elt = *head; elt != 0; elt = elt->next)
		printf("Name: %s | Value: %s\n", elt->name, elt->value);
} 


int list_include(list *head, char *name)
{
	entry *elt;

	for(elt = *head; elt != 0; elt = elt->next)
	{
		if ( strcmp(elt->name, name) == 0)
			return TRUE;
	}

	//it wasn't found after searching all
	return FALSE;
}


char *list_get_value(list *head, char *name)
{
	entry *elt;

	for(elt = *head; elt != 0; elt = elt->next)
	{
		if ( strcmp(elt->name, name) == 0 )
			{
				char *value = calloc(PATH_MAX, 1);
				strcpy(value, elt->value);
				return value;
			}
	}

	//it wasn't found after searching all
	return NULL;

}


