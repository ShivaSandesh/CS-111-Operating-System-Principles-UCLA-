/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 1111111111
   Project 2A - Summer 2017 - CS 111 */
   
#include <stdio.h>          // for stadard in and out 
#include <errno.h>         // for error number 
#include <stdlib.h>       // for Standard in 
#include <string.h>      // for strerror
#include <pthread.h>    // creating a thread
#include <unistd.h>
#include <getopt.h>   // for getopt

#include "SortedList.h"
#include<time.h>
#define BILLION 1E9

int num_Threads = 1;
int num_Iterates = 1;
int syn_Flag = 0;
char sync_Opt = 'X';
int opt_yield  = 0;
pthread_mutex_t lock;
SortedList_t sorted_List;
SortedListElement_t **elems;
static const char alphanum[] = "0123456789!@#$%^&*ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int string_Length = sizeof(alphanum) - 1;
int key_Length = 5;
long long spinlock = 0;

// Helper function to favilitate looking up an element and removing it. 
void lookup_Remove(long long i){
  SortedListElement_t * to_Remove = SortedList_lookup(&sorted_List, elems[i]->key);
  if( to_Remove == NULL) {
    fprintf(stderr,"Element in list missing \n. ");
    exit(2);
  }
  int rv = SortedList_delete(to_Remove);
  if(rv !=  0){
    fprintf(stderr, " Error while removing the element from list. \n");
    exit(2);
  }  
}
void remove_Helper(long long begin) {
  long end = begin + num_Iterates;
  long long i;
  for(i = begin; i < end; i++){
    lookup_Remove(i);
  }
  //sanity check 
    long long size = -1;
    size = SortedList_length(&sorted_List);
    if(size != 0) {
      fprintf(stderr, "List is corrupted. Not all elements deleted.\n");
      exit(2);
    }
}

void add_Helper(long long begin){
  long end = begin + num_Iterates;
  long long i;
  for(i = begin; i < end; i++){
    SortedList_insert(&sorted_List,elems[i]);
  }
  //sanity check 
  long long size = -1;
  size = SortedList_length(&sorted_List);
  if(size < num_Iterates) {
    fprintf(stderr, "List size mismatch.Corrupted list.\n");
    exit(2);
  }
}
void* busy_Worker(void* val){
  long begin = *(long*)val;
  if(sync_Opt == 's') {
    while (__sync_lock_test_and_set(&spinlock, 1));
  }
  if(sync_Opt == 'm'){
    pthread_mutex_lock(&lock);
  }
  add_Helper(begin);
  remove_Helper(begin);
  if(sync_Opt == 's') {
    __sync_lock_release(&spinlock);
  }
  if(sync_Opt == 'm'){
    pthread_mutex_unlock(&lock);
  }

}
int main(int argc, char* argv[]) {
  int ch;
  int opt_Index = 0;
  char* yield_Opts;
  long long total_Time = 0;
  long long avrg_Time = 0;
  long long num_Operations = 0;
  char name[25]="list";
  
  struct timespec request_Start, request_End;
  struct option long_options[] = {
    {"threads",    required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield",      required_argument, NULL, 'y'},
    {"sync",       required_argument, NULL, 's'},
    {0,         0,                    0,     0 }
  };
  // extract all the options supplied using the getopt_long
  while((ch = getopt_long(argc, argv, "t:i:y:s", long_options, &opt_Index)) > 0){
    switch(ch){
    case 't':
      num_Threads = atoi(optarg);
      break;
    case 'i':
      num_Iterates = atoi(optarg);
      break;
    case 'y':  
      yield_Opts = optarg;
      unsigned int i;
      for(i = 0; i < strlen(yield_Opts); i++ )
	{
	  switch(yield_Opts[i])
	    {   
	      // do a bitwise OR to set the flags if option provided
	    case 'i': opt_yield = opt_yield | INSERT_YIELD; break;
	    case 'd': opt_yield = opt_yield | DELETE_YIELD; break;
	    case 'l': opt_yield = opt_yield | LOOKUP_YIELD; break;
	    default: 
	      fprintf(stderr, "Wrong argument to --yield. %d not supported.", yield_Opts[i]);
	      exit(1);
	      break;
	    }
	}
      break;
    case 's': 
      syn_Flag = 1;
      // optarg will return you a string so convert it to char before using
      sync_Opt = optarg[0];
      if((sync_Opt != 's')&& (sync_Opt != 'm')) {
	fprintf(stderr, " Wrong parameter to --sync. %s", strerror(errno));
	exit(1);
      }
      break;
    default:printf("in the default %d", ch); 
      fprintf(stderr, "?? getopt returned character code 0%o ??. %s \n", ch, strerror(errno));
      exit(1);
      break;
    }
  }
  // Initailizing the locking variable 
  if (sync_Opt == 'm') {
    int rv = pthread_mutex_init(&lock, NULL);
    if(rv != 0 ) {
      fprintf(stderr, " Error with thread_mutex_init().%s \n",strerror(errno));
      exit(1);
    }
  }
  // allocate enough space for all the elements in the list 
  elems = malloc(sizeof(SortedListElement_t*)*num_Threads*num_Iterates);
  sorted_List.key = NULL;
  sorted_List.next = NULL;
  sorted_List.prev = NULL;
  // circular list because assignement only allows for one list
  /*SortedList_t* head = &sorted_List;
  sorted_List.key = NULL;
  sorted_List.next = head;
  sorted_List.prev = head;
  long long int i;*/
  srand((unsigned int)time(NULL));
  //  printf("Size of string is, %d", string_Length);
  int i;
  for(i = 0; i < num_Iterates*num_Threads; i++)
    {
      //srand(time(NULL));
      int j = 0;
      char* randkey = malloc(sizeof(char)*(key_Length + 1));
      if(randkey == NULL) {
	i--;
	continue;
      }
      for(j = 0; j< key_Length; j++){
	randkey[j] = alphanum[rand() % string_Length];
      }
      randkey[j] = '\0';
      //printf("%s \n",key);
      elems[i] = malloc(sizeof(SortedListElement_t));
      elems[i]->prev = NULL;
      elems[i]->next = NULL;
      elems[i]->key = randkey;
    }
  
  pthread_t *thread_Id = malloc(num_Threads * sizeof(pthread_t));
  if(thread_Id == NULL)
    {
      fprintf(stderr, "Error: Failed to allocate memmory for threads. %s \n", strerror(errno));
      exit(1);
    }
  int k, l, checker;
  clock_gettime(CLOCK_MONOTONIC, &request_Start);
  for(k = 0; k < num_Threads; k++)
    {    
      long l = k*num_Iterates;
      checker = pthread_create(&thread_Id[k], NULL, &busy_Worker, &l);
      if (checker != 0) {
	fprintf(stderr, "Error while creating a thread. %s \n", strerror(errno));
	exit(1);
      }    
    }
  for(k = 0; k < num_Threads; k++)
    {
      checker = pthread_join(thread_Id[k],NULL);
      if (checker != 0) {
	fprintf(stderr, "Error while joining thread. %s \n", strerror(errno));
	exit(1);
      }   
    }
  clock_gettime(CLOCK_MONOTONIC, &request_End);
  
  // sanity check point
  int size = SortedList_length(&sorted_List);
  if(size != 0 ) {
    fprintf(stderr, "List is corrupted. Some elements not deleted. \n");
    exit(2);
  }
  total_Time = (request_End.tv_sec - request_Start.tv_sec)*BILLION; 
  total_Time += (request_End.tv_nsec - request_Start.tv_nsec);
  num_Operations = num_Iterates*num_Threads*3;
  avrg_Time = total_Time/num_Operations;
  free(thread_Id);
  free(elems);
  
  // name of the test
  //  strcat(name, "list-");
  if(opt_yield != 0) {
    strcat(name, "-");
    strcat(name, yield_Opts);
  }
  else strcat(name, "-none");

  if(syn_Flag == 1)
    { 
      if(sync_Opt == 's') strcat(name, "-s");
      else strcat(name, "-m");
    }
  if(syn_Flag != 1) strcat(name, "-none");

  int numlist = 1;
  printf("%s,%d,%d,%d,%lld,%lld,%lld\n",name,num_Threads,num_Iterates,numlist,num_Operations,total_Time, avrg_Time);
  exit(0);
}
