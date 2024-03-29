#ifndef MEMNODE_H
#define MEMNODE_H


//node used for all processes
struct processes
{
	int pid;
	int size;
	int count;

	struct processes *next;
};
 

//node used for all memory units
struct memory_lists
{
	int start;
	int size;
	struct processes *pid;

	struct memory_lists *next;
	struct memory_lists *prev; //added to make my life easier, more space i know but pls
	// if prev is NULL means first item in list
};

//node used for the memory unit manager
struct memory_list_info
{
	int mem;
	int mem_usage;
	int holes;
	int num_processes;

	struct memory_lists *free_list;
	struct memory_lists *used_list;
};

void initialize_process(struct processes *node );
struct memory_list_info *create_memory_list( int memory_size);
struct memory_lists* create_memory_list_node( struct processes* pid, int start, int memory_size);
//splits a memory node - returns the node that has been reduce. places the other 'hole' in memory
struct memory_lists* split_memory_node(struct memory_list_info *info_node, struct memory_lists *memory_list, int reduced_size);
//removes a memory node from a list
struct memory_lists *remove_memory_node(struct memory_list_info *info_node,struct processes *process);
//uses a memory node from the free list with a process
void use_memory_node(struct memory_list_info *info_node,struct memory_lists *node, struct processes *process);
//takes a memory node from used list and frees it sending it back to the free list
struct memory_lists *free_memory_node(struct memory_list_info *info_node,struct memory_lists *memory_node);
//merges nodes in free list - does a double merge if needed (it being in the middle of two nodes (that make it adjacent to both))
struct memory_lists *merge_memory_node(struct memory_lists *node1,struct memory_lists *node2, struct memory_list_info *info_node);
//detaches from a list changes their pointers to point elsewherE!
struct memory_lists *detach_memory_node(struct memory_list_info* info_node,struct memory_lists *node);
#endif
