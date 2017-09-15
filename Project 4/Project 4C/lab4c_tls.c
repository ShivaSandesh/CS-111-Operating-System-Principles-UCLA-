/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 111111111
   Project 4c -Part b - Summer 2017 - CS 111 */

#include <stdio.h>          // for stadard in and out
#include <errno.h>         // for error number
#include <stdlib.h>       // for Standard in
#include <string.h>      // for strerror
#include <getopt.h>     // for getopt
#include <time.h>
#include <sys/poll.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <math.h>                  // for the log function
#include <unistd.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#define TEMP_ADC_PIN 0              // ADC pin is 0
#define GPIO_PIN 3
const int B  = 4275;                    // B value for thermistor
mraa_aio_context adc_Pin;              // used as AIO context

float R, temp_Val;
struct pollfd ufds[1];
time_t rawtime;
struct tm* timeInfo;
int log_Fd = 0;
int log_Flag = 0;
char* log_Server = "lever.cs.ucla.edu";
char* host_Id;
int id = 123454321;
int uid;
int port;
int sock_Fd;
SSL_CTX* new_Context;
SSL *SSLClient;
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
  SSL_write(SSLClient,towrite, len);
  if(log_Flag) {
    write(log_Fd,towrite, len);
  }
  mraa_aio_close(adc_Pin);
  if(log_Flag) close(log_Fd);
  
  exit(0);
}
int main(int argc, char* argv[]) {
  if(argc < 2) {
    fprintf(stderr, "Missing mandatory port number parameter.\n");
    exit(1);
  }
  int ch;
  char* log_Name = NULL;
  int stop_Flag, opt_Index;
  log_Flag = opt_Index = stop_Flag = 0;
  int period = 1;
  char temp_Scale = 'F';
  char towrite[17];
  uid = id;
  host_Id = log_Server;
  struct option long_options[] = {
    {"log",    required_argument, NULL, 'l'},
    {"host",  required_argument,  NULL, 'h'},
    {"id", required_argument,     NULL, 'i'},
    {0,        0,                 0,     0 }
  };
  char* val;
  // extract all the options supplied using the getopt_long
  while((ch = getopt_long(argc, argv, "l:h:i:", long_options, &opt_Index)) > 0){
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
    case 'h': host_Id = optarg; break;
    case 'i': 
      val = optarg;
      if(strlen(val) != 9) {
	fprintf(stderr, "--id length of argument must be 9.\n");
	exit(1);
      }
      uid = atoi(val);
      break;
    default:
      fprintf(stderr, "?? getopt returned character code 0%o ??. %s \n", ch, strerror(errno));
      exit(1);
      break;
    }
  }
  port = atoi(argv[optind]);
  // Server initialization 
  sock_Fd = socket(AF_INET, SOCK_STREAM, 0);
  if(sock_Fd < 0) {
    fprintf(stderr, " socket() failed to file descriptor. \n");
    exit(2);
  }
  struct sockaddr_in serv_addr;
  struct hostent *server;
  server = gethostbyname(host_Id);
  if(server == NULL) {
    fprintf(stderr, "Error in host name: No such host\n",strerror(errno));
  }
  memset((char*) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port =  htons(port);
  memcpy(&serv_addr.sin_addr.s_addr,server->h_addr, server->h_length);  
  if(connect(sock_Fd,(struct sockaddr*)& serv_addr, sizeof(serv_addr)) < 0){
    fprintf(stderr, "connect() failed. %s", strerror(errno));
    exit(2);
  }
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  new_Context = SSL_CTX_new(TLSv1_client_method());
  if(new_Context == NULL){
    fprintf(stderr, " Error: Failed to create a new SSL_CTX object for a connection.\n");
    exit(2);
  }
  SSLClient = SSL_new(new_Context);
  if(SSLClient == NULL) {
    fprintf(stderr, " Error: Failed to create a new SSL structure for a connection.\n");
    exit(2);
  }
  if(SSL_set_fd(SSLClient, sock_Fd) != 1) {
    fprintf(stderr, " Error in set_fd for SSL. \n");
    exit(2);
  }
  if(SSL_connect(SSLClient) != 1) {
    fprintf(stderr, " Error in connect() for SSL. \n");
    exit(2);
  }
  char buff[15];
  int len1 = sprintf(buff,"ID=%d\n",uid);
  SSL_write(SSLClient, buff, len1);
  if(log_Flag == 1) {
    write(log_Fd, buff, len1);
  }
  rawtime = time(NULL);
  time(&rawtime);
  timeInfo = localtime(&rawtime);
  // initializing the sensor
  mraa_init();
  adc_Pin = mraa_aio_init(TEMP_ADC_PIN);
  ufds[0].fd = sock_Fd;                               // Standard Input
  ufds[0].events = POLLIN;      // check for normal 
  time_t t1 = time(0);
  while(1) {
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
      time_t t2 = time(0);
      int time_Diff = t2 - t1;
      if(time_Diff >= period) {
	int len = sprintf(towrite, "%.2d:%.2d:%.2d %0.1f\n", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, temp_Val);
	SSL_write(SSLClient, towrite, len);
	if(log_Flag) {
	  write(log_Fd, towrite, len);
	}
	t1 = time(0);
      }
    }
    int rv = poll(ufds, 1, 1);
    if(rv == -1) {
      fprintf(stderr,"Error: poll() failed \n", strerror(errno));
      exit(1);
    }
    else {
      if(ufds[0].revents & POLLIN) {
        char buffer[1024];
        int nchars = SSL_read(SSLClient, buffer, 1022);
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
		  fprintf(stderr,"Argument to PERIOD unacceptable !!\n");
		  exit(0);
		}
	      }

	      else {
		fprintf(stderr, "Wrong command to the program !! \n");
		exit(2);
	      }
	      free(command);
	      char *command = malloc(sizeof(char)*15);
	      k = 0;
	    }
	  }
	}
      }
    }
  }
  return 0;
}  
