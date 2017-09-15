/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 111111111
   Project 2A - Summer 2017 - CS 111 */
   
#include <stdio.h>           // for stadard in and out 
#include <stdlib.h>         // for Standard in 
#include <string.h>        // for strerror
#include <pthread.h>           
#include "SortedList.h"
#include <sched.h>
/*
struct SortedListElement {
  struct SortedListElement *prev;
  struct SortedListElement *next;
  const char *key;
};
*/
void SortedList_insert(SortedList_t *list, SortedListElement_t *element){   
  if(element == NULL) return;
  SortedListElement_t *ptr = list;
  while(ptr->next != NULL ){
    if(strcmp(element->key,ptr->next->key) > 0) {
      ptr = ptr -> next;
    }
    //if(strcmp(element->key,ptr->key) <= 0)
    else break;
  }
  if (opt_yield & INSERT_YIELD)
    pthread_yield();
  
  if(ptr->next != NULL)
    ptr->next->prev = element;
  
  element->next = ptr->next;
  element->prev = ptr;
  //ptr->prev->next = element;
  ptr->next = element;
}
int SortedList_delete( SortedListElement_t *element){
  if(element == NULL) return 1;
  SortedListElement_t *prev_Node = element->prev;
  SortedListElement_t *next_Node = element->next;
  if(next_Node != NULL){
      if(next_Node->prev != element ) return 1;
  }
  if(prev_Node->next != element) return 1;
  if(next_Node != NULL) 
    element->next->prev = prev_Node;

  if(opt_yield & DELETE_YIELD)
    pthread_yield();
  
  element->prev->next = next_Node;  
  // free(element); //do it in the client. Client should be responsible for memmory deallocation.
  //if(next != NULL) element->next->prev = elemetn->prev; <---- this is problem gdb pointed here.
  // can't do this pointer set incorrectly
  return 0;
}
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key){
  if(key == NULL ) return NULL;
  SortedListElement_t *curr_Node = list->next;
  while(curr_Node !=  NULL){
    if(strcmp(curr_Node->key, key) == 0)
      return curr_Node;
    if (opt_yield & LOOKUP_YIELD)
      pthread_yield();

    curr_Node = curr_Node->next;
  }
  return NULL;
}

int SortedList_length(SortedList_t *list){
  // empty list
  if(list == NULL) return -1;
  SortedListElement_t *prev_Node = NULL;
  SortedListElement_t *curr_Node = list;
  SortedListElement_t *next_Node = curr_Node->next;
  int count = 0;

  while(curr_Node->next != NULL) {
    if(next_Node->prev != curr_Node) return -1;
    // we will be in the beginnig of the string 
    if(prev_Node != NULL){
      if(prev_Node->next != curr_Node) return -1;
    }
    count++;
    if(opt_yield & LOOKUP_YIELD)
      pthread_yield();
    // update the pointers in the fashion that you always 
    // traverse the list in incremental fashion one step
    // at a time. 
    curr_Node = curr_Node->next;
    prev_Node = curr_Node->prev;
    next_Node = curr_Node->next;
  }
  return count;
  }
