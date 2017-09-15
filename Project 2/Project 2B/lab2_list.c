/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 111111111
   Project 2A - Summer 2017 - CS 111 */
   
#include <stdio.h>          // for stadard in and out 
#include <errno.h>         // for error number 
#include <stdlib.h>       // for Standard in 
#include <string.h>      // for strerror
#include <pthread.h>    // creating a thread
#include <unistd.h>
#include <getopt.h>   // for getopt
#include <time.h>
#include "SortedList.h"
#define BILLION 1E9

int num_Threads = 1;
int num_Iterates = 1;
int syn_Flag = 0;
char sync_Opt = 'X';
int opt_yield  = 0;
pthread_mutex_t* mutex_Array;
long long *spinlock_Array;
SortedListElement_t **elems;
SortedList_t* sub_Lists;
static const char alphanum[] = "0123456789!@#$%^&*ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
int string_Length = sizeof(alphanum) - 1;
int key_Length = 5;
int num_Lists = 1;
long long* lock_Timer;

// initialize all the locks that program might need
void initialize_Locks(){
  int i = 0;
  // initialize the locks for number of lists user wants 
  spinlock_Array = malloc(sizeof(long long )*num_Lists);
  if(spinlock_Array == NULL) {
    fprintf(stderr, " malloc failed while initializing locks.%s \n",strerror(errno));
    exit(1);
  }
  mutex_Array = malloc(sizeof(pthread_mutex_t)*num_Lists);
  if(mutex_Array == NULL) {
    fprintf(stderr, " malloc failed while initializing locks.%s \n",strerror(errno));
    exit(1);
  }
  for(i = 0; i < num_Lists;i++){
    mutex_Array[i] = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
    // pthread_mutex_init(&mutex_Array[i], NULL);
    spinlock_Array[i] = 0;
  }
  return;
} 
void initialize_Elems(){
  elems = malloc(sizeof(SortedListElement_t*)*num_Threads*num_Iterates);
  if(elems == NULL) {
    fprintf(stderr, " malloc failed while initializing list elements.%s \n",strerror(errno));
    exit(1);
  }
  srand((unsigned int)time(NULL));
  int i;
  for(i = 0; i < num_Iterates*num_Threads; i++)
    {
      int j = 0;
      char* randkey = malloc(sizeof(char)*(key_Length + 1));
      if(randkey == NULL) {
	i--; // try again 
	continue;
      }
      for(j = 0; j< key_Length; j++){
	randkey[j] = alphanum[rand() % string_Length];
      }
      randkey[j] = '\0';
      SortedListElement_t *node = malloc(sizeof(SortedListElement_t)+ (key_Length + 1));
      node->prev = NULL;
      node->next = NULL;
      node->key = randkey;
      elems[i] = node;
    }
  return;
}
void initialize_List(){
  // allocate enough memory for all the lists 
  sub_Lists = malloc(sizeof(SortedList_t)*num_Lists);
  if(sub_Lists == NULL) {
    fprintf(stderr, "Error: Failed to allocate memmory for sub lists. %s \n", strerror(errno));
    exit(1); 
  }
  return;
}
void * busy_Worker(void * val){
  long long tid = (*((long long *) val));
  long long begin  = (*(long long *)val) * num_Iterates;
  struct timespec start_time;
  struct timespec end_time;
  long long lock_Gettime;
  long long i;
  // INSERT
  for( i = begin; i < begin + num_Iterates; i++){
    SortedListElement_t * node = elems[i];
    long long nlist = ((long long) (elems[i]->key))%num_Lists;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    if(sync_Opt == 'm') 
      pthread_mutex_lock(&(mutex_Array[nlist]));
    if(sync_Opt == 's') 
      while(__sync_lock_test_and_set(&(spinlock_Array[nlist]), 1));
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    SortedList_insert(&(sub_Lists[nlist]), node);

    if(sync_Opt == 's') 
      __sync_lock_release(&(spinlock_Array[nlist]));
    if(sync_Opt == 'm')
      pthread_mutex_unlock(&(mutex_Array[nlist]));
    
    lock_Gettime  = end_time.tv_nsec - start_time.tv_nsec;
    lock_Gettime +=  (end_time.tv_sec - start_time.tv_sec)*BILLION;  
    lock_Timer[tid] += lock_Gettime;
  }
  
  for(i = 0; i < num_Lists; i++){
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    if(sync_Opt == 'm')
      pthread_mutex_lock(&(mutex_Array[i]));
    if(sync_Opt == 's') 
      while(__sync_lock_test_and_set(&(spinlock_Array[i]), 1));
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    if(SortedList_length(&(sub_Lists[i])) < 0){
      fprintf(stderr, "List is Corrupted: Length mismatch.\n");
      exit(2);
    }

    if(sync_Opt == 's')
      __sync_lock_release(&(spinlock_Array[i]));
    if(sync_Opt == 'm') 
      pthread_mutex_unlock(&(mutex_Array[i]));
    
    lock_Gettime  = end_time.tv_nsec - start_time.tv_nsec;
    lock_Gettime += BILLION * (end_time.tv_sec - start_time.tv_sec);  
    lock_Timer[tid] += lock_Gettime;
  }
  // DELETE
  SortedListElement_t * to_Remove;
  for( i = begin ; i < num_Iterates + begin; i++){
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    long long nlist = ((long long) (elems[i]->key))%num_Lists;
    if(sync_Opt == 'm') 
      pthread_mutex_lock(&(mutex_Array[nlist]));
    if(sync_Opt == 's') 
      while(__sync_lock_test_and_set(&(spinlock_Array[nlist]), 1));
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    to_Remove = SortedList_lookup(&(sub_Lists[nlist]), elems[i]->key);
    if(to_Remove == NULL){
      fprintf(stderr, "List is Corrupted. Missing Element.\n");
      exit(2);
    }
    if(SortedList_delete(to_Remove)){
      fprintf(stderr, " Function SortedList_delete() Failed !\n");
      exit(2);
    }
    
    if(sync_Opt == 's')
      __sync_lock_release(&(spinlock_Array[nlist]));
    if(sync_Opt == 'm') 
      pthread_mutex_unlock(&(mutex_Array[nlist]));
    
    lock_Gettime += BILLION * (end_time.tv_sec - start_time.tv_sec); 
    lock_Gettime  = end_time.tv_nsec - start_time.tv_nsec;
    lock_Timer[tid] += lock_Gettime;
  }
  return NULL;
}

int main(int argc, char* argv[]) {
  int ch;
  int opt_Index = 0;
  char* yield_Opts;
  long long total_Time = 0;
  long long avrg_Time = 0;
  long long num_Operations = 0;
  char name[30]="list";  
  struct timespec request_Start, request_End;
  struct option long_options[] = {
    {"threads",    required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield",      required_argument, NULL, 'y'},
    {"sync",       required_argument, NULL, 's'},
    {"lists",      required_argument, NULL, 'l'},
    {0,            0,                 0,     0 }
  };
  // extract all the options supplied using the getopt_long
  while((ch = getopt_long(argc, argv, "t:i:y:s:l:", long_options, &opt_Index)) > 0){
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
    case 'l':
      num_Lists =  atoi(optarg);
      if( num_Lists < 0) {
	fprintf(stderr, "Number of sub lists must greater than Zero !! \n");
	exit(2);
      }
      break;
    default: 
      fprintf(stderr, "?? getopt returned character code 0%o ??. %s \n", ch, strerror(errno));
      exit(1);
      break;
    }
  }
  // initialize all the mutex as well and spin locks 
  initialize_Locks();
  // allocate, generate and initialize all the elements of the  the list 
  initialize_Elems();
  // itialize all sublist with head
  initialize_List();
  lock_Timer = malloc(sizeof(long long) * num_Threads);
  if(lock_Timer == NULL) {
    fprintf(stderr, "Error: Failed to allocate memmory. %s \n", strerror(errno));
    exit(1); 
  }
  int i;
  for(i = 0; i < num_Threads;i++){
    lock_Timer[i] = 0;
  }
  
  pthread_t *thread_Id = malloc(sizeof(pthread_t)*num_Threads);
  if(thread_Id == NULL)
    {
      fprintf(stderr, "Error: Failed to allocate memmory for threads. %s \n", strerror(errno));
      exit(1);
    }
  long long* threadpos = malloc(sizeof(long long)* num_Threads);
  int k, l, checker;
  clock_gettime(CLOCK_MONOTONIC, &request_Start);
  for(k = 0; k < num_Threads; k++)
    {    
      threadpos[k] =  k;
      checker = pthread_create(&thread_Id[k], NULL, &busy_Worker, &(threadpos[k]));
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
  long size = 0;
  for(i = 0; i < num_Lists; i++){
    size = size + SortedList_length(&sub_Lists[i]);
  }
  if(size < 0 ) {
    fprintf(stderr, "List is corrupted. Some elements not deleted. \n");
    exit(2);
  }
  total_Time = (request_End.tv_sec - request_Start.tv_sec)*BILLION; 
  total_Time += (request_End.tv_nsec - request_Start.tv_nsec);
  num_Operations = num_Iterates*num_Threads*3;
  avrg_Time = total_Time/num_Operations;

  // name of the test
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
  long long sum = 0;
  for(i = 0; i < num_Threads; i++) sum = sum + lock_Timer[i];

  sum = sum /(num_Threads * (2*num_Iterates + num_Lists));
  if(syn_Flag != 1) sum = 0;
  printf("%s,%d,%d,%d,%lld,%lld,%lld,%lld\n",name,num_Threads,num_Iterates,num_Lists,num_Operations,total_Time,avrg_Time,sum/num_Threads);
  free(thread_Id);
  free(elems);
  //free(lock_Timer);
  //free(sub_Lists);
  exit(0);
}



