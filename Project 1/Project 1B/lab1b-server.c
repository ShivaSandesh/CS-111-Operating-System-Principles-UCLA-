/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 111111111
   Project 1A - Summer 2017 - CS 111 */

#include <poll.h>                          // for poll
#include <fcntl.h>                        // for macros STDIN_FILENO etc.
#include <stdio.h>                       // for STDIN and STDOUT
#include <errno.h>                      // error numbers and description
#include <unistd.h>                    // for fork, pipe, and exec
#include <string.h>                   // for strerror
#include <getopt.h>                  // for getopt
#include <signal.h>                 // for kill
#include <stdlib.h>                // for standard I/O
#include <mcrypt.h>               // for encryption
#include <termios.h>             // for termios, tcgetattr, and tcsetattr
#include <sys/wait.h>           // for waitpid
#include <sys/stat.h>          // for stats of server
#include <sys/types.h>        // for waitpid
#include <sys/socket.h>      // for socket usage
#include <netinet/in.h>     // for AF_NET
#include <linux/netlink.h>

char cr_Lf[2] = {'\r','\n'};
pid_t  sid;                   // Shell id
struct pollfd ufds[2];       // for polling
MCRYPT td;

void find_exitStatus()
{
  int status;
  int low_Bits, high_Bits;
  if(waitpid(sid, &status, 0) == -1 ) {
    fprintf(stderr, "waitpid() failed. %s \n", strerror(errno));
    exit(1);
  }
  low_Bits = status & 0x007f;
  high_Bits = (status & 0xff00)>>8;
  printf("SHELL EXIT SIGNAL= %d STATUS= %d \n", low_Bits, high_Bits);
  exit(0);
}
// SIGPIPE - signal handler method
void signal_handler(int signum) {
  if(signum == SIGPIPE) {
    find_exitStatus();
    exit(1);
  }
}
int main(int argc, char* argv[]) {
  int ch,encrp_Flag, opt_Index;
  encrp_Flag = opt_Index = 0;
  char* encrypt_Fname = NULL;
  int port_No = -1;

  // for encryption-key file
  int key_Size = 8;
  char key_Buff[256];

  // for the read buffer
  int read_Buff_size = 128;
  char read_Buff[read_Buff_size];

  // file descriptors for the log file, client and the encrytption Key file.
  int fd_LogFile, fd_Client, fd_Key;

  // pipes to and from the shell
  int fd_Toshell[2], fd_Fromshell[2];

  struct option long_options[] = {
    {"port",    required_argument, NULL, 'p'},
    {"encrypt", required_argument, NULL, 'e'},
    {0,         0,                 0,     0 }
  };
  // extract all the options supplied using the getopt_long
  while((ch = getopt_long(argc, argv, "p:e", long_options, &opt_Index)) > 0){
    switch(ch){
    case 'p':
      port_No = atoi(optarg);
      break;
    case 'e':
      encrp_Flag = 1;
      encrypt_Fname = optarg;
      if(encrypt_Fname == NULL) {
	fprintf(stderr, "Error: Log File aname missing.%s \n", strerror(errno));
	exit(1);
      }
      break;
    default :
      fprintf(stderr, "?? getopt returned character code 0%o ??. %s \n", ch, strerror(errno));
      exit(1);
    }
  }
  if(encrp_Flag == 1) {
    if ((fd_Key = open(encrypt_Fname, O_RDONLY)) < 0) {
      fprintf(stderr, "Error in opening the key file. %s \n", strerror(errno));
      exit(1);
    }

    if((read(fd_Key, key_Buff, key_Size)) < 0) {
      fprintf(stderr, "Error while reading from key file. %s \n", strerror(errno));
      exit(1);
    }

    td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    char * IV = malloc(mcrypt_enc_get_iv_size(td));
    memset(IV, 0,  sizeof(char)* mcrypt_enc_get_iv_size(td));
    if((int)sizeof(key_Size) < mcrypt_enc_get_key_size(td)) {
      mcrypt_generic_init(td, (void *) key_Buff, key_Size, (void *) IV);
      // mcrypt_generic(td, key_Buff, sizeof(key_Buff));
    }
  }
  int fd_Socket = socket(AF_INET, SOCK_STREAM, 0);
  if(fd_Socket < 0) {
    fprintf(stderr, "Error while creating socket. %s \n", strerror);
    exit(1);
  }
  struct sockaddr_in serv_addr, client_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port_No);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  if(bind(fd_Socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "Error while using bind(). %s \n", strerror(errno));
    exit(1);
  }
  if(listen(fd_Socket, 5) < 0){
    fprintf(stderr, "Error while using listen(). %s \n", strerror(errno));
    exit(1);
  }
  socklen_t client_Size =  sizeof(client_addr);
  fd_Client = accept(fd_Socket, (struct sockaddr *) &client_addr, &client_Size);
  if(fd_Client < 0) {
    fprintf(stderr, "Error while using accept(). %s \n", strerror(errno));
    exit(1);
  }
  if(pipe(fd_Toshell) < 0 ) {
    fprintf(stderr, "Failed to create file desciptors. %s \n", strerror(errno));
    exit(1);
  }
  if(pipe(fd_Fromshell) < 0 ) {
    fprintf(stderr, "Failed to create file desciptors. %s \n", strerror(errno));
    exit(1);
  }
  
  // forking the child process.
  sid = fork();
  if(sid < 0) {
    fprintf(stderr, "Failed to create Child process . %s \n", strerror(errno));
    exit(1);    // retunr 1 if system call failure
  }
  // child thread
  if(sid == 0){
    // close the un-used file descriptior
    if(close(fd_Toshell[1]) < 0) {
      fprintf(stderr, "Error: close() failed. %s \n", strerror(errno));
      exit(1);
    }
    if(close(fd_Fromshell[0]) < 0) {
      fprintf(stderr, "Error: close() failed. %s \n", strerror(errno));
      exit(1);
    }

    if(dup2(fd_Toshell[0],0) == -1){ // input to shell
      fprintf(stderr, "Error: dup2() failed. %s \n", strerror(errno));
      exit(1);
    }
    if(dup2(fd_Fromshell[1],1) == -1){     // output from shell
      fprintf(stderr, "Error: dup2() failed. %s \n", strerror(errno));
      exit(1);
    }
    if(dup2(fd_Fromshell[1],2) == -1){    // stderr frm shell
      fprintf(stderr, "Error: dup2() failed. %s \n", strerror(errno));
      exit(1);
    }
    execlp("/bin/bash", "/bin/bash", NULL);
  }

  // parent thread
  else {
    if( close(fd_Toshell[0]) < 0) {
      fprintf(stderr, "Error: close() failed. %s \n", strerror(errno));
      exit(1);
    }
    if(close(fd_Fromshell[1]) < 0) {
      fprintf(stderr, "Error: close() failed. %s \n", strerror(errno));
      exit(1);
    }
    atexit(find_exitStatus);
    signal(SIGPIPE, signal_handler);
    int s1 = fd_Client;                   // stdin
    int s2 = fd_Fromshell[0];    // pipe that returns output from the shell
    ufds[0].fd = s1;
    ufds[1].fd = s2;
    ufds[0].events = POLLIN | POLLHUP | POLLERR;   // check for normal or out-of-band
    ufds[1].events = POLLIN | POLLHUP | POLLERR;  // check for just normal data
    while(1){
      int nchars;
      char temp[1];
      int i;
      int rv = poll(ufds, 2, 0);
      if(rv == -1) {
	fprintf(stderr,"Error: poll() failed \n", strerror(errno));
	exit(1);
      }
      else {
	// client is sending a command
	if(ufds[0].revents & POLLIN) {
	  if((nchars = read(fd_Client, read_Buff, read_Buff_size)) > 0 ) {
	    // decrypt message
	    if(encrp_Flag == 1 )
	      mdecrypt_generic(td, read_Buff, nchars);

	    for(i = 0; i < nchars; i++) {
	      temp[0] = read_Buff[i];
	      // carriage return or newline character entered.
	      if(temp[0] == '\r' || temp[0] == '\n') {
		write(fd_Toshell[1], &cr_Lf[1], 1);
	      }
	      // ^D entered at the client side
	      else if(temp[0] == 0x04) {
		close(fd_Toshell[0]);
		close(fd_Toshell[1]);
		close(fd_Client);
	      }
	      // ^C entered at the client side
	      else if(temp[0] == 0x03) {
		kill(sid, SIGINT);
	      }
	      else
		write(fd_Toshell[1], &temp, 1);
	    }
	  }
	}
	// server trying to write to the client through the fd_client
	if(ufds[1].revents & POLLIN) {
	  if((nchars = read(fd_Fromshell[0], read_Buff, read_Buff_size)) > 0){
	    if(encrp_Flag == 1)
	      mcrypt_generic(td, read_Buff, nchars);
	    
	    write(fd_Client,read_Buff, nchars);
	  }
	}
	if (ufds[1].revents & (POLLHUP | POLLERR)) {
	  break;
	}
	// this added 
	if (ufds[0].revents & (POLLHUP | POLLERR)) {
	  close(fd_Fromshell[1]);
	}
      }
    }
  }
}




