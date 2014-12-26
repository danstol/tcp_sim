#include "linked_list.h"

/*****************************************
 *	File: window.c
 *	
 *	Purpose: Serves as an interface for a linked_list
 *	implements linked_list.h
 *
 *****************************************/

struct Node *ptr = NULL;

//sets maximum window size
void set_window_size(int l){
	win_size(l);
}

//adds a segment to the LL
void add(struct Segment *segment){
	ptr = add_to_list(segment, ntohl(segment->sequence), false);
}

//adds a segment to the LL at the back, not used but could still be handy
void add_back(struct Segment *segment){
	ptr = add_to_list(segment, segment->sequence, true);
}

//prints the current contents of the LL
void print(){
	print_list();
}

//removes a segment with the given sequence(element)
void remove_element(int element){
	delete_from_list(element);
}

//returns the first segment in the list
struct Segment* first(){
	struct Node* node = get_first();
	return node->segment;
}

//returns the current size of the list
int get_size(){
	return list_size();
}

//returns the segment with the given sequence(element)
struct Segment* get_from_list(int element){
	if(contains(element)){
		struct Node* node = search_in_list(element, NULL);
		return node->segment;
	}
	return NULL;
}