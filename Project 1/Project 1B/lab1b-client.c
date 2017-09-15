/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 111111111
   Project 1B - Summer 2017 - CS 111
   lab1b.client*/
   
#include <poll.h>		          // for poll
#include <stdio.h>                       // I/O services 
#include <errno.h>                      // for error descritptions
#include <fcntl.h>                     // for STDOUT_FILENO and STDI_FILENO
#include <netdb.h>                    // for AF_INET
#include <stdlib.h>                  // for std library fucntions
#include <string.h>    		    // for strerror
#include <getopt.h>    	   	   // for getopt
#include <signal.h>               // for kill
#include <unistd.h>              // for fork, pipe, and exec
#include <mcrypt.h>             // for encrytption and decryption
#include <termios.h>           // for termios, tcgetattr, and tcsetattr
#include <pthread.h>          // creating a thread
#include <sys/wait.h>        // for waitpid
#include <sys/stat.h>       // server
#include <sys/types.h>     // for waitpid
#include <arpa/inet.h>    // for AF_INET
#include <sys/socket.h>  // for socket
#include <netinet/in.h> //  for sockaddr_in

struct termios orig_Tios;          // for original copy of the attributes
struct termios new_Tios;          // for new copy of the attributes
int fd_Log, fd_Key;
MCRYPT td;
int log_Flag = 0;
int encrp_Flag = 0;
char cr_Lf[2] = {'\r','\n'};
int fd_Socket;
int flag_C = 0;
int flag_D = 0;
void exit_func() {
  tcsetattr(STDIN_FILENO, TCSANOW, &orig_Tios );
  mcrypt_generic_deinit(td);
  mcrypt_module_close(td);
  exit(0);
}
void handle_cntrlD(){
  close(fd_Socket);
  exit_func();
}

int main(int argc, char* argv[]){
  // local variables
  int ch, opt_Index;
  opt_Index = 0;
  char* log_Name = NULL;
  char* encrypt_Name = NULL;
  int port_No = -1;

  // for key file
  int key_Size = 8;
  char key_Buff[256];

  // for the terminal
  int read_Buff_size = 1024;
  char read_Buff[read_Buff_size];
  struct option long_options[] = {
    {"port",    required_argument, NULL, 'p'},
    {"log",     required_argument, NULL, 'l'},
    {"encrypt", required_argument, NULL, 'e'},
    {0,         0,                 0,     0 }
  };
  while((ch = getopt_long(argc, argv, "p:l:e", long_options, &opt_Index)) > 0){
    switch(ch){
    case 'p':
      port_No = atoi(optarg);
      break;
    case 'l':
      log_Flag = 1;
      log_Name = optarg;
      if(log_Name == NULL) {
	fprintf(stderr, "Error: Log File name missing.%s", strerror(errno));
	exit(1);
      }
      break;
    case 'e':
      encrp_Flag = 1;
      encrypt_Name = optarg;
      if(encrypt_Name == NULL) {
	fprintf(stderr, "Error: Log File aname missing.%s", strerror(errno));
	exit(1);
      }
      break;
    default :
      fprintf(stderr, "?? getopt returned character code 0%o ??. %s\n", ch, strerror(errno));
      exit(1);
    }
  }
    // create the log file to write to and get a file descriptor
  if(log_Flag == 1) {
    if((fd_Log = creat(log_Name, 0666)) < 0 ){
      fprintf(stderr, "Error in creating log file. %s", strerror(errno));
      exit(1);
    }
  }
    // encrytion falg is on
  if(encrp_Flag == 1) {
    // fd_Key becomes the file desciptor
    if ((fd_Key = open(encrypt_Name, O_RDONLY)) < 0) {
      fprintf(stderr, "Error in opening the key file. %s", strerror(errno));
      exit(1);
    }
    if((read(fd_Key, key_Buff, key_Size)) < 0) {
      fprintf(stderr, "Error while reading from key file. %s", strerror(errno));
      exit(1);
    }
    td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    char * IV = malloc(mcrypt_enc_get_iv_size(td));
    memset(IV, 0,  sizeof(char)* mcrypt_enc_get_iv_size(td));
    if((int)sizeof(key_Size) < mcrypt_enc_get_key_size(td)) {
      mcrypt_generic_init(td, (void *) key_Buff, key_Size, (void *) IV);
    }
  }
    // set the terminal to the non-canonical (character at a time, no echo
  tcgetattr(STDIN_FILENO, &orig_Tios);
  new_Tios = orig_Tios;
  new_Tios.c_iflag = ISTRIP;       // only lower 7 bits
  new_Tios.c_oflag = 0;             // no processing
  new_Tios.c_lflag = 0;          // no processing
  tcsetattr(STDIN_FILENO, TCSANOW, &new_Tios );
    // create a socket and set its parameter
  fd_Socket = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in serv_addr;
  struct hostent *server;
  server = gethostbyname("localhost");
  memset((char*) &serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  memcpy(&serv_addr.sin_addr.s_addr,server->h_addr, server->h_length);
  serv_addr.sin_port = htons(port_No);    // to adjust the endianess
  if(connect(fd_Socket,(struct sockaddr*)& serv_addr, sizeof(serv_addr)) < 0){
    fprintf(stderr, "connect() failed. %s", strerror(errno));
  }
  atexit(exit_func);
  struct pollfd ufds[2];       // for polling
  ufds[0].fd = 0;
  ufds[1].fd = fd_Socket;
  ufds[0].events = POLLIN | POLLHUP | POLLERR;   // check for normal or out-of-band
  ufds[1].events = POLLIN | POLLHUP | POLLERR;  // check for just normal data
  while(1) {
    int nchars = 0;
    char temp[1];
    int i;
    int rv = poll(ufds, 2, 0);
    if(rv == -1) {
      fprintf(stderr,"Error: poll() failed \n", strerror(errno));
      exit(1);
    }
    else {
      if (ufds[1].revents & (POLLHUP | POLLERR)) {
        close(fd_Socket);
        exit_func();
        exit(0);
      }
      if(ufds[1].revents & POLLIN) {
	int lbuffer_Size = 1025;
	char l_Buffer[lbuffer_Size];
       	    if((nchars = read(fd_Socket, l_Buffer, lbuffer_Size-1))> 0){
	      // write to the log file if --log option provided
	      if(log_Flag == 1) {
                char temp[1200];
                int data_Size = nchars * sizeof(char);
                // int sprintf ( char * str, const char * format, ... );
                int size = sprintf(temp, "RECEIVED %d bytes: %s \n", data_Size, l_Buffer);
                write(fd_Log, temp, size );
              }

	      // if we have encryption flag then decrypt before sending to shell
	      if(encrp_Flag == 1) {
		mdecrypt_generic(td, l_Buffer, nchars);
	      }
	      // write the std output --- check for crlf then write to stdout.
	      //write(1, l_Buffer, nchars);
	      for( i = 0; i < nchars; i++) {
		temp[0] = l_Buffer[i];
		if(temp[0] == '\n') {
		  write(1, &cr_Lf, 2);
		}
		else 
		  write(1, &temp, 1);
	      }
	    }
      }
      if(ufds[0].revents & POLLIN) {
	if((nchars = read(STDIN_FILENO, read_Buff, read_Buff_size-1)) > 0) {
	  for( i = 0; i < nchars; i++) {
	    temp[0] = read_Buff[i];
	    if(temp[0] == 0x04) 
	      flag_D = 1;
	    if(temp[0] == 0x03)
	      flag_C = 1;
	    // map received <cr> or <lf> into <cr><lf>
	    if(temp[0] == '\r' || temp[0] == '\n') 
	      write(STDOUT_FILENO, &cr_Lf, 2);
	    else
	      write(STDOUT_FILENO,temp, 1);
	  }
	  if(encrp_Flag == 1) {
	    mcrypt_generic(td, read_Buff, nchars);
	  }
	  write(fd_Socket, read_Buff,nchars);
	  // log all the read data into a log file
	  if(log_Flag == 1){
	    read_Buff[nchars] = '\0';
	    char temp_Buff[1200]; 
	    int data_Size = nchars * sizeof(char);
	    int size = sprintf(temp_Buff, "SENT %d bytes: %s \n", data_Size, read_Buff);
	    write(fd_Log, temp_Buff, size);
	  }
	}
	if(flag_D == 1) exit(0);
	if(flag_C == 1) exit(0);
      }						
    }
  }
}


