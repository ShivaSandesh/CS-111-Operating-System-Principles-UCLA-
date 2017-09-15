/* NAME : FNU SHIVA SANDESH
   Email : sandesh.shiva362@gmail.com
   UID : 111111111
   Project 1A - Summer 2017 - CS 111 */

#include <termios.h>            // for termios, tcgetattr, and tcsetattr
#include <unistd.h>            // for fork, pipe, and exec
#include <stdio.h>            // for STDIN and STDOUT
#include <pthread.h>         // creating a thread
#include <sys/types.h>      // for waitpid
#include <sys/wait.h>      // for waitpid
#include <string.h>       // for strerror
#include <getopt.h>      // for getopt
#include <signal.h>     // for kill
#include <poll.h>      // for poll
#include <stdlib.h>
#include <errno.h>

// Global constants
char cr_Lf[2] = {'\r','\n'};  
struct termios orig_Tios;       // for original copy of the attributes
struct termios new_Tios;       // for new copy of the attributes
pid_t  sid;                   // Shell id
struct pollfd ufds[2];       // for polling
 
void find_exitStatus()
{
  int status;
  int low_Bits, high_Bits;
  if(waitpid(sid, &status, 0) == -1 ) {
    fprintf(stderr, "waitpid() failed. %s", strerror(errno));
      exit(1);
  }
    low_Bits = status & 0x007f;
    high_Bits = status & 0xff00;
    printf("SHELL EXIT SIGNAL= %d STATUS= %d\n", low_Bits, high_Bits);
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_Tios );
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
  int buff_Size = 256;
  char char_Buff[buff_Size];
  int nchar, option_Index, ch;
  nchar = option_Index = ch = 0;
  
  int is_Shell = 0;           // flag for --shell options
  int s1, s2, rv;            // for polling
  
  static struct option long_Option[] = {
    {"shell", no_argument, 0, 's'},
    {0      , 0          , 0,  0}
  };
  while((ch = getopt_long(argc, argv, "s", long_Option, &option_Index)) > 0) {
    switch(ch) {
    case 's':
	is_Shell = 1; break;

    default: 
      printf("?? getopt returned character code 0%o ??\n", ch);
      exit(1);
    }
  }
  // int tcgetattr(int fileDescriptor, struct termios *terminal);
  tcgetattr(STDIN_FILENO, &orig_Tios);
  new_Tios = orig_Tios;
  new_Tios.c_iflag = ISTRIP;         // only lower 7 bits
  new_Tios.c_oflag = 0;             // no processing
  new_Tios.c_lflag = 0;            // no processing
  
 // int tcsetattr(int fildes, int opt_actions, struct termios *termios_p);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_Tios );
  
  // We do have shell option
  int fd_Toshell[2];
  int fd_Fromshell[2];
  if(is_Shell == 1) {
      
    // create pipes for communicating to shell -- return 1 if system call fails
    if(pipe(fd_Toshell) < 0 ) {
      fprintf(stderr, "Failed to create file desciptors. %s\n", strerror(errno));
      exit(1);    
    }
    if(pipe(fd_Fromshell) < 0 ) {
      fprintf(stderr, "Failed to create file desciptors. %s\n", strerror(errno));
      exit(1);    
    }
    
    sid = fork();
    if(sid < 0) {
      fprintf(stderr, "Failed to create Child process . %s", strerror(errno));
      exit(1);    // retunr 1 if system call failure
  }
  /* fork() returns a zero to the newly created child process.
     In child process do the I/O redirections and then do execlp */
  if(sid == 0){
    // close the un-used file descriptior
    if(close(fd_Toshell[1]) < 0) {
      fprintf(stderr, "Error: close() failed. %s", strerror(errno)); 
      exit(1);
    }
    if(close(fd_Fromshell[0]) < 0) {
      fprintf(stderr, "Error: close() failed. %s", strerror(errno));
      exit(1);
    }

    if(dup2(fd_Toshell[0],0) == -1){ // input to shell
      fprintf(stderr, "Error: dup2() failed. %s", strerror(errno));
      exit(1);
    }
    if(dup2(fd_Fromshell[1],1) == -1){     // output from shell
      fprintf(stderr, "Error: dup2() failed. %s", strerror(errno));
      exit(1);
    }

    if(dup2(fd_Fromshell[1],2) == -1){    // stderr frm shell
      fprintf(stderr, "Error: dup2() failed. %s", strerror(errno));
      exit(1);
    }
    execlp("/bin/bash", "/bin/bash", NULL);
  }
  // In parent process
  // reads from STDIN forward to STDOUT and shell and then process it in shell and terminal
  else { 
    if( close(fd_Toshell[0]) < 0) {
        fprintf(stderr, "Error: close() failed. %s", strerror(errno));
	exit(1);
   }

    if(close(fd_Fromshell[1]) < 0) {
       fprintf(stderr, "Error: close() failed. %s", strerror(errno));
       exit(1);
   }

    atexit(find_exitStatus);
    signal(SIGPIPE, signal_handler);
    int s1 = 0;                   // stdin 
    int s2 = fd_Fromshell[0];    // pipe that returns output from the shell
    ufds[0].fd = s1;
    ufds[1].fd = s2;
    ufds[0].events = POLLIN | POLLHUP | POLLERR;   // check for normal or out-of-band
    ufds[1].events = POLLIN | POLLHUP | POLLERR;  // check for just normal data
    char buff[1];
    while(1) {
      rv = poll(ufds, 2, 0);
      if(rv == -1) {
	fprintf(stderr,"Error: poll() failed", strerror(errno));
	exit(1);
      }
      else {
	if(ufds[0].revents & POLLIN) {
	  if(read(STDIN_FILENO, buff, 1) > 0 ) {
	    if(buff[0] == '\r' ||buff[0] == '\n') {
	      write(fd_Toshell[1], &cr_Lf[1], 1);
	      write(STDOUT_FILENO, &cr_Lf, 2);
	    }
	    // chech for the speciffic characters
	    else if(buff[0] == 0x04) { 
	      close(fd_Toshell[0]);
	      close(fd_Toshell[1]);
	      //find_exitStatus();
	      // kill(sid, SIGHUP);   // ask TA for this line.
	    }
	    else if(buff[0] == 0x03) { 
	      kill(sid, SIGINT);        // default behaviour
	      //find_exitStatus(); 
	      exit(1);
	    }
	    else  {
	      write(fd_Toshell[1], &buff, 1);
	      write(STDOUT_FILENO, &buff, 1);
	    }
	  }
	}
	if(ufds[1].revents & POLLIN) {
          if((nchar = read(fd_Fromshell[0], char_Buff, buff_Size)) > 0) {
            int i;
            char temp[1];
            for( i = 0; i < nchar; i++) {
              temp[0] = char_Buff[i];
              if(temp[0] == '\n') {
                write(0, &cr_Lf, 2);
              }
              else
                write(0, &temp, 1);
            }
          }
        }

	if (ufds[1].revents & (POLLHUP | POLLERR)) {
	  break;
	}
      }
    }
  }
} 
  //If we are not creating a duplex terminal
 else {
   while((nchar = read(STDIN_FILENO, char_Buff, buff_Size)) > 0) {
     int i;
     char temp[1];
     for( i = 0; i < nchar; i++) {
       temp[0] = char_Buff[i];
       // map received <cr> or <lf> into <cr><lf>
       if(temp[0] == '\r' || temp[0] == '\n')
	 write(STDOUT_FILENO, cr_Lf, 2);
       /* detected a pre-defined escape-sequence(^D),
	  restore normal terminal modes and exit */
       else if(temp[0] == 0x04) {
	 tcsetattr(STDIN_FILENO, TCSANOW, &orig_Tios );
	 exit(0);
       }
       else
	 write(1,temp, 1);
     }
   }
 }
}
