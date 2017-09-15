/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 1111111111
   Project 2A - Summer 2017 - CS 111 */
  
#include <stdio.h>            // for stadard in and out 
#include <errno.h>           // for error number 
#include <stdlib.h>         // for Standard in 
#include <string.h>        // for strerror
#include <getopt.h>       // for getopt
#include <pthread.h>     // creating a thread
#include <sys/stat.h> 
#include <sched.h>
#include <fcntl.h>  // for Macros STDIN_FILENO
#define BILLION 1E9

long long counter = 0;
int num_Threads = 1;
int num_Iterates = 1;
int opt_yield  = 0;
char sync_Opt = 'X';
int syn_Flag = 0;
static pthread_mutex_t lock;
long long spinlock = 0;
char name[20] = "add";

void add(long long *pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield)
    sched_yield();
  *pointer = sum;
}
void* busy_Worker() {
  int nums[2];
  nums[0] = 1;
  nums[1] = -1;
  int i, j;
  long long old_Val, new_Val;
  old_Val = new_Val = 0;
  for(i = 0; i < 2; i++) {
    for(j = 0; j < num_Iterates; j++ ) {
      if(syn_Flag != 1) add(&counter, nums[i]);
      else {
	// Now sync_Opt can be 's', 'm', or 'c' if sync == 1
	switch(sync_Opt) {
	  // spin lock
	case 's': 
	  while (__sync_lock_test_and_set(&spinlock, 1));
	  add(&counter, nums[i]);
	  __sync_lock_release(&spinlock);
	  break;
	  
	  // pthread_mutex
	case 'm':
	  pthread_mutex_lock(&lock);
	  add(&counter, nums[i]);
	  pthread_mutex_unlock(&lock);
	  break;
	  
	  //CSV 
	case 'c':
	  do{
	    old_Val = counter;
	    new_Val = old_Val + nums[i];
	    if(opt_yield)
	      sched_yield();
	  } while(__sync_val_compare_and_swap(&counter, old_Val, new_Val) != old_Val);
	  break;
	  
	default :  break;
	}
      }
    }
  }
  return NULL;
}
int main(int argc, char* argv []) {
  int opt_Index = 0;
  int checker = 0;
  long long total_time = 0;
  long avrg_Time = 0;
  int num_Operations = 0;
  int i = 0;
  int ch;
  long long total_Time;
  struct timespec request_Start, request_End;
  struct option long_options[] = {
    {"threads",    required_argument, NULL, 't'},
    {"iterations", required_argument, NULL, 'i'},
    {"yield ",     no_argument,       NULL, 'y'},
    {"sync",      required_argument,  NULL, 's'},
    {0,         0,                    0,     0 }
  };
  // extract all the options supplied using the getopt_long
  while((ch = getopt_long(argc, argv, "t:i:ys", long_options, &opt_Index)) > 0){
    switch(ch){
    case 't':
      num_Threads = atoi(optarg);
      break;
    case 'i':
      num_Iterates = atoi(optarg);
      break;
    case 'y':  
      opt_yield = 1;
      break;
    case 's': 
      syn_Flag = 1;
      // optarg will return you a string so convert it to char before using
      sync_Opt = optarg[0];
      if((sync_Opt != 's') &&(sync_Opt !=  'c') && (sync_Opt != 'm')) {
	fprintf(stderr, " Wrong parameter to --sync.", strerror(errno));
	exit(1);
      }
      break;
    default :
      fprintf(stderr, "?? getopt returned character code 0%o ??. %s \n", ch, strerror(errno));
      exit(1);
      break;
    }
  }
  // Initailizing the locking variable for 'counter' 
  if (sync_Opt == 'm') {
    int rv = pthread_mutex_init(&lock, NULL);
    if(rv != 0 ) {
      fprintf(stderr, " Error with thread_mutex_init().%s \n",strerror(errno));
      exit(1);
    }
  }
  counter = 0;
  num_Operations = 2*num_Iterates*num_Threads;
  pthread_t* thread_Id;
  thread_Id = (pthread_t*)malloc(sizeof(pthread_t)*num_Threads);
  if(thread_Id == NULL)
    {
      fprintf(stderr, "Error: Failed to allocate memmory for threads. %s\n", strerror(errno));
      exit(1);
    }
  clock_gettime(CLOCK_MONOTONIC, &request_Start);
  for(i = 0; i < num_Threads; i++)
    {
      checker = pthread_create(&thread_Id[i], NULL, &busy_Worker, NULL);
      if (checker != 0) {
	fprintf(stderr, "Error while creating a thread. %s \n", strerror(errno));
	free(thread_Id);
	exit(1);
      }    
    }
  for(i = 0; i < num_Threads; i++)
    {
      checker = pthread_join(thread_Id[i],NULL);
      if (checker != 0) {
	fprintf(stderr, "Error while joining thread. %s \n", strerror(errno));
	//free(thread_Id);
	exit(1);
      }   
    }
  clock_gettime(CLOCK_MONOTONIC, &request_End);
  
  // extracting the name of the test.
  if(opt_yield == 1) strcat(name, "-yield");
  if(syn_Flag == 0) strcat(name, "-none");
  else {
    switch (sync_Opt) {
      case 's': strcat(name, "-s"); break;
      case 'c': strcat(name, "-c"); break;
      case 'm': strcat(name, "-m"); break;
      default : break; 
    }
  }
  
  free(thread_Id);
  total_Time = (request_End.tv_sec - request_Start.tv_sec)*BILLION + (request_End.tv_nsec - request_Start.tv_nsec);
  avrg_Time = total_Time/num_Operations;
  printf("%s,%d,%d,%lld,%lld,%lld,%lld\n",name,num_Threads,num_Iterates,num_Operations,total_Time,avrg_Time,counter);
  if(counter == 0) exit(0);
  exit(1);
}
