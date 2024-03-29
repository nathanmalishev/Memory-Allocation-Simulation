//best fit algorithm

#include <stdio.h>
#include "headerfiles/algorithms.h"
#include "headerfiles/utilities.h"

struct memory_lists *find_best(struct memory_list_info *info_node, 
	struct processes *process)
{

	struct memory_lists *current_memory = info_node->free_list;
	struct memory_lists *smallest ;
	//initialise smallest to be largest
	smallest = (struct memory_lists*)salloc(sizeof(struct memory_lists));
	
	if( !info_node->free_list)
	{
		//means list is 100% full
		return NULL;
	}

	smallest->size = info_node->mem*2;

	while(current_memory != NULL)
	{	
		//we want smallest possible sized memory
		if( current_memory->size < smallest->size && current_memory->size >= process->size)
		{
			smallest = current_memory;
		}
		current_memory = current_memory->next;
	}
	if(smallest->size == info_node->mem*2)
	{
		return NULL;
	}

	return smallest;


}