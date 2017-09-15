/*  NAME: FNU SHIVA SANDESH
    EMAIL: sandesh.shiva362@gmail.com
    ID: 111-111-111
    Project 0 - CS 111 - Summer 2017 */

/* A program that copies its standard input to its standard output by read(2)-ing
from file descriptor 0 (until encountering an end of file) and write(2)-ing to
file descriptor 1. If no errors (other than EOF) are encountered, your program
should exit(2) with a return code of 0. */

#include<stdlib.h>           // for exit
#include<signal.h>          // for SIGSEGV
#include<fcntl.h>          // for O_RDONLY
#include<string.h>        // for strerror()
#include<getopt.h>       // for getopt_long()
#include<unistd.h>      // for getopt_long()
#include<errno.h>      // for Errro Hanndling
#include<stdio.h>     // for Standard-In and Standard-out, printf
 
/* logs an error message(on stderr, file descriptor 2) and
   exit with a return code of 4. */
void sighandler(int signum)
{   
  if(signum == SIGSEGV) {
    fprintf(stderr, "Process encountered a SEGMENTATION FAULT. \n");
    exit(4);
  }
}

int main(int argc, char* argv[]){
  // Variable 'optind': system initializes this value to 1.
  int ch, optind;
  char *inFile_Name = NULL;
  char *outFile_Name = NULL;
  // flags for different options 
  int in_Flag, out_Flag, fault_flag;
  char* ptr = NULL;
  int fd0, fd1;
  int buffer_Size = 80;
  char buffer[buffer_Size];
  int temp;
  
  struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"segfault", no_argument, 0, 's' },
    {"catch", no_argument, 0, 'c' },
    {0,0,0,0}
  };
  
  in_Flag = out_Flag = fault_flag = 0;
  
  // STEP 1: Process all arguments and store the results in variables.
  ch = getopt_long(argc,argv,"i:o:sc", long_options, &optind);
  while(ch > 0 ) {
    switch(ch) {
    case 'i': 
      // get the pointer to input file from the 'optarg'
      inFile_Name = optarg;
      in_Flag = 1;
      break;
      
    case 'o':
      // get the pointer to output file from the 'optarg'
      outFile_Name = optarg;
      out_Flag = 1; 
      break;
      
    case 's': fault_flag = 1;
      break;
      
    case 'c': 
      signal(SIGSEGV, sighandler);
      break;
      /* If encountered an unrecognized argument so print out an error message
	 including a correct usage line, and exit(2) with a return code of 1. */
   default: fprintf(stderr,"?? getopt returned character code 0%o ??\n",ch);
      exit(1);
      break;
    }
    ch = getopt_long(argc, argv, "i:o:sc", long_options, &optind);
  }
  
  // STEP : Register the signal handler and cause the segfault
  if(fault_flag == 1) {
    *ptr = 'X'; 
  }
  // STEP : Do any file redirection
  if(in_Flag == 1){ 
    fd0 = open(inFile_Name, O_RDONLY);
    if(fd0 < 0) {
      fprintf(stderr,"--input caused the error. Could not open %s file. %s. \n", inFile_Name, strerror(errno));
      exit(2);
    }
    else {
      //close(0);
      if(dup2(fd0,0) < 0) { 
	fprintf(stderr, "--input caused the errror. Failed to use dup2.%s", strerror(errno)); 
      }
      close(fd0);
    }
  }
  if(out_Flag) {
    fd1 = creat(outFile_Name, 0666);
    if (fd1 < 0) {
      fprintf(stderr,"--output caused error. Could not create %s file. %s. \n", outFile_Name, strerror(errno));
      exit(3);
    }
    else {
      //close(1);
      if(dup2(fd1,1) <0) { 
	fprintf(stderr, "--output caused the errror. Failed to use dup2.%s", strerror(errno));
      }
      close(fd1);
    }
  }
  
  // STEP : If no segfault was caused, copy stdin to stdout
  temp = read(0, buffer, buffer_Size);
  while(temp > 0) {
    if (write(1, buffer,temp) < 0) {
      fprintf(stderr,"--output caused error. Failed to write to %s. %s \n",outFile_Name, strerror(errno));
      exit(3);
    }
    temp = read(0, buffer, buffer_Size);
  }
  exit(0);
}
