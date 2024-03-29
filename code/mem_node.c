#include "headerfiles/mem_node.h"
#include "headerfiles/utilities.h"
#include <stdlib.h>
#include <stdio.h>

void initialize_process(struct processes *node ){
    node->pid   =0;
    node->size  =0;
    node->count =0;
    node->next  = NULL;
}

struct memory_list_info* create_memory_list_info_node(int memory_size)
{
    struct memory_list_info *info_node;
    info_node = (struct memory_list_info *)malloc(sizeof(struct memory_list_info));

    info_node->mem           = memory_size;
    info_node->mem_usage     = 0;
    info_node->holes         = 1;
    info_node->num_processes = 0;
    info_node->free_list     = NULL;
    info_node->used_list     = NULL;

    return info_node;
}
 
//creates a memory structure (info node at head, with two lists used mem and free)
struct memory_list_info* create_memory_list(int memory_size)
{
    struct memory_list_info *info_node;
    info_node            = create_memory_list_info_node(memory_size);
    
    struct memory_lists *first_free_node;
    first_free_node      = create_memory_list_node( NULL, 0, memory_size);
    
    info_node->free_list = first_free_node;
    return info_node;
}

//creates a new memory node
// takes a process * , start location, size. Returns *node
struct memory_lists* create_memory_list_node( struct processes* pid, int start, int memory_size){
    struct memory_lists* node;
    node = (struct memory_lists *)salloc(sizeof(struct memory_lists));

    if(pid)
    {
        node->pid = pid;
    }
    else
    {
        node->pid = NULL;
    }
    node->start = start;
    node->size  = memory_size;
    node->next  = NULL;
    node->prev  = NULL;
 
    return node;
}

//function that splits a memory segment up
// pass node you want to split & the size you want to reduce it to
// returns first memory node (resized block) *
struct memory_lists* split_memory_node(struct memory_list_info *info_node, 
        struct memory_lists *current_node, int reduced_size)
{
    if(current_node == NULL)
    {
        printf("\nbad memory unit being passed\n");
        exit(1);
    }

    if(reduced_size == current_node->size)
    {
        return current_node;
    }

    struct memory_lists* new_hole;
    struct memory_lists* temp;
    //initialise the new hole
    new_hole = create_memory_list_node(NULL, 
        current_node->start+reduced_size, current_node->size-reduced_size);

    //assign points & adjust node
    current_node->size = reduced_size;
    temp               = current_node->next;
    current_node->next = new_hole;
    new_hole->next     = temp;
    new_hole->prev     = current_node;
    //if we can assign previous to last hole
    if(new_hole->next)
    {
        new_hole->next->prev = new_hole;
    }
    //for every split there is a new hole
    info_node->holes++;

    return current_node;
}

//uses a vacant memory node
//places it into used memory list
//memory list is sorted in descending order of SIZE (largest on top)
void use_memory_node(struct memory_list_info *info_node,
    struct memory_lists *node, struct processes *process)
{
    node = detach_memory_node(info_node, node);
    node->pid  = process;
    node->pid->next = NULL;
    //update info node
    info_node->holes--;
    info_node->num_processes++;
    info_node->mem_usage    += node->pid->size;

    //new used list adding here
    struct memory_lists *current_node = info_node->used_list;

    //if nothing is in the used lists just add it
    if(info_node->used_list == NULL)
    {
        info_node->used_list = node;
    }
    else
    {
        while(current_node != NULL)
        {
            if( node->pid->size > current_node->pid->size)
            {
                //if the current node is greater we want it infront
                if(current_node->prev == NULL)
                {
                    //if its the first node just slip it in
                    current_node->prev   = node;
                    node->next           = current_node;
                    info_node->used_list = node;
                    return;
                }
                else
                {
                    //if in middle
                    //not the first node
                    node->prev         = current_node->prev;
                    node->next         = current_node;
                    current_node->prev = node;
                    node->prev->next   = node;
                    return;
                }
            }

            if(current_node->next)
            {
                current_node = current_node->next;
            }else
            {
                break;
            }
        }
        //if here must be last used memory
        current_node->next = node;
        node->prev         = current_node;
        node->next         = NULL;
        return;
    }
}


//function finds largest & oldest memory unit being used
// returns it detached from list
struct memory_lists *remove_memory_node(struct memory_list_info *info_node,
    struct processes *process)
{
    struct processes *tmp_proc;
    struct memory_lists *tmp = NULL;
    if(info_node->used_list != NULL)
    {        
        if( info_node->used_list->pid->count < 2)
        {
            info_node->used_list->pid->count++;
            //we want to add its pid to end of process list
            tmp_proc = process;
            while(tmp_proc)
            {
                if(tmp_proc->next)
                {
                    tmp_proc = tmp_proc->next;
                }
                else
                {
                    break;
                }
            }
            tmp_proc->next = info_node->used_list->pid;
        }

        //detach node from list
        tmp = detach_memory_node(info_node, info_node->used_list);
        info_node->mem_usage -= tmp->size;
        info_node->num_processes--;
        
        return  tmp;
    }
    return tmp;
}

struct memory_lists *detach_memory_node(struct memory_list_info* info_node,
        struct memory_lists *node){
        //remove from linked list
        if(node->prev ==NULL)
        {
            //we are case 1 front of the list
            if(node->pid)
            {
                //we are on used list
                info_node->used_list = node->next;
            }
            else
            {
                //we are on free list
                info_node->free_list = node->next;
            }
            if(node->next)
            {
                // at the front and list is not empty
                node->next->prev = NULL;
                //move the next thing up in the list
            }
        }
        else if (node->next == NULL)
        {
            //we are at the end of the list easy
            node->prev->next = NULL;
        }
        else
        {
            
            //we are in the middle of the queue
            //update node before hand
            node->prev->next = node->next;
            //update node after
            node->next->prev = node->prev;
        }
        //remove process
        node->next = NULL;
        node->prev = NULL;
        node->pid = NULL;
        return node;
}


//function takes a used node and frees it - joining it with free_list
struct memory_lists *free_memory_node(struct memory_list_info *info_node,
    struct memory_lists *memory_node)
{
    struct memory_lists *current_node = info_node->free_list;

    if(info_node->free_list == NULL)
    {
        //nothing there just add
        info_node->free_list = memory_node;
    }
    else
    {
        while(current_node != NULL)
        {
            //cycle through list and add
            if(memory_node->start == current_node->start + current_node->size
                || memory_node->start +memory_node->size == current_node->start)
            {
                
                //we have found adjacent nodes pls merge
                memory_node = merge_memory_node(memory_node , current_node, info_node);
                break;  
            }
            else if(memory_node->start < current_node->start )
            {
                
                //put the node in
                //could be first node so
                if(current_node->prev)
                {
                    current_node->prev->next = memory_node;
                    memory_node->prev = current_node->prev;
                }
                else
                {
                    info_node->free_list = memory_node;
                }
                memory_node->next = current_node;
                current_node->prev = memory_node;
                break;
            }
            else{
                
                //memory_list_testing(info_node,1);
                //else its behind the node
                if(current_node->next == NULL)
                {
                    //now the last item in the list just add it
                    current_node->next = memory_node;
                    memory_node->prev = current_node;
                    
                    break;
                }
                if( current_node->next != NULL)
                {
                    current_node = current_node->next;
                }
                else
                {
                    break;
                }
            }            
        }
    }
    info_node->holes++;
    return memory_node;
}

//function takes two nodes and merges them. Stable node has to be the one
//already in the list
//returns merged node
struct memory_lists *merge_memory_node(struct memory_lists *node1,
    struct memory_lists *stable_node, struct memory_list_info *info_node)
{
    struct memory_lists *tmp;
    if(node1->start < stable_node->start)
    {
        //placing node 1 in front of stable node
        //so node 1 is base
        node1->size = node1->size + stable_node->size;

        if(stable_node->prev)
        {
            node1->prev             = stable_node->prev;
            stable_node->prev->next = node1;
        }
        else
        {
            //was first in free list
            info_node->free_list = node1;
        }

        if(stable_node->next)
        {
            node1->next             = stable_node->next;
            stable_node->next->prev = node1;
        }

        //free(stable_node);
        info_node->holes--;

        //node1 now could be adjacent to the node above it?
        if( (node1->prev) && (node1->start == node1->prev->start + node1->prev->size))
        {
            //we will call function again
            node1->prev->size += node1->size;
            node1->prev->next = node1->next;
            node1->next->prev = node1->prev;
            tmp               = node1->prev;
            free(node1);
            info_node->holes--;
            return tmp;
        }

        return node1;
    }
    else
    {
        //we are placing node behind stable is at front
        //just increase size and free other node
        stable_node->size = stable_node->size +node1->size;

        if(node1->next)
        {
            node1->next->prev = stable_node;
        }

        free(node1);
        info_node->holes--;

        //this back node could be touching another
        if( stable_node->next && stable_node->start+stable_node->size == stable_node->next->start )
        {
            //we need to merge this back node onto aswell
            stable_node->size += stable_node->next->size;
            tmp               = stable_node->next;
            stable_node->next = stable_node->next->next;

            if(stable_node->next)
            {
                stable_node->next->prev = stable_node;
            }

            free(tmp);
            info_node->holes--;
        }
        
        return stable_node;
    }
}

