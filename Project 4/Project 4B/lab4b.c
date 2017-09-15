/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 111111111
   Project 4b - Summer 2017 - CS 111 */

#include <stdio.h>          // for stadard in and out
#include <errno.h>         // for error number
#include <stdlib.h>       // for Standard in
#include <string.h>      // for strerror
#include <getopt.h>     // for getopt
#include<time.h>
#include <sys/poll.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <math.h>                  // for the log function
#include <unistd.h>
#define TEMP_ADC_PIN 0              // ADC pin is 0
#define GPIO_PIN 3
const int B  = 4275;                    // B value for thermistor
//const int R0 = 100;                  // R0 is 100k ohms
mraa_aio_context adc_Pin;              // used as AIO context
mraa_gpio_context button;
//float adc_Val;                     // ADC value read into this variable
float R, temp_Val;
struct pollfd ufds[1];
time_t rawtime;
struct tm* timeInfo;
int log_Fd = 0;
int log_Flag = 0;
int check_Opt_Period(char* command) {
  char* str_Period = "PERIOD=";
  int i = 0;
  for(i = 0; i < (int)strlen(str_Period); i++)
    {
      if((str_Period[i] != command[i]) || (command[i] == '\0'))
        return 0;
    }
  return 1;
}
void handle_Off() {
  rawtime = time(NULL);
  timeInfo = localtime(&rawtime);
  char towrite[20];
  char* shut = "SHUTDOWN";
  int len = sprintf(towrite, "%.2d:%.2d:%.2d %s\n", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, shut);
  write(1,towrite, len);
  if(log_Flag) {
    write(log_Fd,towrite, len);
  }
  mraa_aio_close(adc_Pin);
  mraa_gpio_close(button);
  close(log_Fd);
  exit(0);
}
void button_Pushed(){
  int read_Val = mraa_gpio_read(button);
  if(read_Val != 0 ){
    handle_Off();
  }
  return;
}
int main(int argc, char* argv[]) {
  int ch;
  char* log_Name = NULL;
  int stop_Flag, opt_Index;
  log_Flag = opt_Index = stop_Flag = 0;
  int period = 1;
  char temp_Scale = 'F';
  char towrite[17];
  struct option long_options[] = {
    {"log",    required_argument, NULL, 'l'},
    {"scale",  required_argument, NULL, 's'},
    {"period", required_argument, NULL, 'p'},
    {0,        0,                 0,     0 }
  };
  // extract all the options supplied using the getopt_long
  while((ch = getopt_long(argc, argv, "l:s:p:", long_options, &opt_Index)) > 0){
    switch(ch){
    case 'l':
      log_Flag = 1;
      log_Name = optarg;
      if(log_Name == NULL) {
        fprintf(stderr, "Error: Log File name missing.%s", strerror(errno));
        exit(1);
      }
      // create the log file to write to and get a file descriptor
      if((log_Fd = creat(log_Name, 0666)) < 0 ){
        fprintf(stderr, "Error in creating log file. %s", strerror(errno));
        exit(1);
      }
      break;
    case 's':  temp_Scale = optarg[0];  break;
    case 'p':  period = atoi(optarg);  break;
    default:
      fprintf(stderr, "?? getopt returned character code 0%o ??. %s \n", ch, strerror(errno));
      exit(1);
      break;
    }
  }
  rawtime = time(NULL);
  time(&rawtime);
  timeInfo = localtime(&rawtime);
  // initializing the sensor
  mraa_init();
  adc_Pin = mraa_aio_init(TEMP_ADC_PIN);
  button = mraa_gpio_init(3);
  mraa_gpio_dir(button , MRAA_GPIO_IN);
  ufds[0].fd = 0;                               // Standard Input
  ufds[0].events = POLLIN;      // check for normal o

  while(1) {
    button_Pushed();            // first check if button is pushed or not
    if(stop_Flag != 1)  {
      rawtime = time(NULL);
      time(&rawtime);
      timeInfo = localtime(&rawtime);
      uint16_t adc_Val = mraa_aio_read(adc_Pin);
      R = (1023.0/((float)adc_Val)-1.0)*100000.0;
      temp_Val = 1.0/(log(R/100000.0)/B+1/298.15)-273.15;
      if(temp_Scale == 'F') {
        temp_Val = temp_Val*1.8 + 32.0;
      }
      //int i = 0;
      int len = sprintf(towrite, "%.2d:%.2d:%.2d %0.1f\n", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, temp_Val);
      write(1, towrite, len);
      if(log_Flag) {
        write(log_Fd, towrite, len);
      }
    }
    int rv = poll(ufds, 1, 0);
    if(rv == -1) {
      fprintf(stderr,"Error: poll() failed \n", strerror(errno));
      exit(1);
    }
    else {
      if(ufds[0].revents & POLLIN) {
        char buffer[1024];
        int nchars = read(0, buffer, 1022);
	if(nchars > 0) {
	  int i = 0;
	  int k = 0;
	  char* command = malloc(sizeof(char)*15);
	  for (i = 0; i< nchars; i++) {
	    if (buffer[i] != '\n') {
	      command[k] = buffer[i];
	      k++;
	    }
	    else {
	      command[k] = '\0';
	      if(log_Fd) {
		int len = strlen(command);
		write(log_Fd, command, len);
		write(log_Fd, "\n", 1);
	      }
	      if(strcmp(command, "OFF") == 0){
		//write(log_Fd, "SHUTDOWN",8);
		handle_Off();
	      }
	      else if(strcmp(command, "STOP") == 0) {
		stop_Flag = 1;
	      }
	      else if(strcmp(command, "START") == 0) {
		stop_Flag  = 0;
	      }
	        
	      else if(strcmp(command, "SCALE=F") == 0) {
		temp_Scale = command[6];
	      }
	      else if(strcmp(command, "SCALE=C") == 0) {
		temp_Scale = command[6];
	      }
	      else if(check_Opt_Period(command) == 1) {
		int left = strlen(command) - strlen("PERIOD=");
		if(left <= 0){
		  fprintf(stderr,"Incomplete argument for PERIOD \n");
		  exit(0);
		}
		char accumulator[left];
		int i = 0;
		while(command[i+7] != '\0') {
		  accumulator[i] = command[i+7];
		  i++;
		}
		period = atoi(accumulator);
		if((period < 0) || (period > 3600)) {
		  fprintf(stderr,"Argument to  PERIOD unacceptable !!\n");
		  exit(0);
		}
	      }

	      else {
		fprintf(stderr, "Wrong command to the program !! \n");
		exit(0);
	      }
	      free(command);
	      char *command = malloc(sizeof(char)*15);
	      k = 0;
	    }
	  }
	}
      }
    }
    int i = 0;
    while(i < period) {
      button_Pushed();
      sleep(1);
      i++;
    }
  }
  return 0;
}  
