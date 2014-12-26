#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

/*********************************
 *	File: linked_list.h
 *
 *	Purpose: A basic LinkedList with a few adjustments
 *	for window size. I coded this last year while in 2160.
 *
 *********************************/

struct Segment{
	uint32_t sequence;			//the sequence number
	uint32_t datalen;		//length of data
	int32_t data;	//actual data
};
 
struct Node
{
    int val;
	struct Segment *segment;
    struct Node *next;
};

struct Node *head = NULL;
struct Node *curr = NULL;
int size;
int window_size;

//returns the list size
int list_size(){
	return size;
}

/*********************************
 *	Function: create_list
 *
 *	Purpose: Creates the LinkedList and sets val&segment as the
 *	head node
 *
 *********************************/

struct Node* create_list(struct Segment *seg, int val)
{
    //printf("\n creating list with headnode as [%d]\n",val);
    struct Node *ptr = (struct Node*)malloc(sizeof(struct Node));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->val = val;
	ptr->segment = seg;
    ptr->next = NULL;

    head = curr = ptr;
    return ptr;
}

struct Node* get_first(){
	struct Node *temp = (struct Node*)malloc(sizeof(struct Node));
	temp = head;
	head = head->next;
	return temp;
}

/*********************************
 *	Function: add_to_list
 *
 *	Purpose: Adds a value to the LinkedList, first checks
 *	if the head is NULL, if it is calls create_list. Otherwise just
 *	adds it. Can add to the front or the back, indicated by add_to_end passed value
 *
 *********************************/
struct Node* add_to_list(struct Segment *seg, int val, bool add_to_end)
{
	if(list_size() == window_size){
		printf("\nThe window is full: [%d] elements\n", list_size());
		return NULL;
	}
	
    if(NULL == head)
    {
		size++;
		printf("\nAdded value [%d] to the window\n", val);
        return ( create_list(seg, val));
    }

    struct Node *ptr = (struct Node*)malloc(sizeof(struct Node));
    if(NULL == ptr)
    {
        printf("\n Node creation failed \n");
        return NULL;
    }
    ptr->val = val;
	ptr->segment = seg;
    ptr->next = NULL;

    if(add_to_end)
    {
        curr->next = ptr;
        curr = ptr;
    }
    else
    {
        ptr->next = head;
        head = ptr;
    }
	size++;
	printf("\nAdded value [%d] to the window\n",val);
    return ptr;
}

/*********************************
 *	Function: search_in_list
 *
 *	Purpose: Searches the list for the specified value val
 *
 *********************************/
struct Node* search_in_list(int val, struct Node **prev)
{
    struct Node *ptr = head;
    struct Node *tmp = NULL;
    bool found = false;

    //printf("\n Searching the list for value [%d] \n",val);

    while(ptr != NULL)
    {
        if(ptr->val == val)
        {
            found = true;
            break;
        }
        else
        {
            tmp = ptr;
            ptr = ptr->next;
        }
    }

    if(true == found)
    {
        if(prev)
            *prev = tmp;
        return ptr;
    }
    else
    {
        return NULL;
    }
}

/*********************************
 *	Function: contains
 *
 *	Purpose: Searches the list for the given value val, and returns 1 if found, 0 otherwise
 *
 *********************************/
int contains(int val)
{
    struct Node *ptr = head;
    struct Node *tmp = NULL;

    //printf("\n Searching the list for value [%d] \n",val);

    while(ptr != NULL)
    {
        if(ptr->val == val)
        {
            return 1;
        }
        else
        {
            tmp = ptr;
            ptr = ptr->next;
        }
    }
	return 0;
}

/*********************************
 *	Function: create_list
 *
 *	Purpose: Searches the list  for the specified value val, if it finds it
 *	returns the pointer to the value and deletes it, setting the head, curr, next Nodes
 *	appropiately
 *
 *********************************/
int delete_from_list(int val)
{
    struct Node *prev = NULL;
    struct Node *del = NULL;

    printf("\nDeleting value [%d] from window\n",val);

    del = search_in_list(val,&prev);
    if(del == NULL)
    {
        return -1;
    }
    else
    {
        if(prev != NULL)
            prev->next = del->next;

        if(del == curr)
        {
            curr = prev;
        }
        else if(del == head)
        {
            head = del->next;
        }
    }

    free(del);
    del = NULL;
	size--;
    return 0;
}

//Traverses the list and prints all the node values
void print_list(void)
{
    struct Node *ptr = head;
    while(ptr != NULL)
    {
        printf("Seq#:%d \t Datalen:%d \t Data:'%s' \n", (uint32_t)ptr->segment->sequence, (uint32_t)ptr->segment->datalen, (unsigned char*)ptr->segment->data);
        ptr = ptr->next;
    }
	
    return;
}

void win_size(int l){
	window_size = l;
}