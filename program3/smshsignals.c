#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "smshsignals.h"


#ifndef DEBUG
#define DEBUG   0
#endif 


/*void reg_handler(int sig)
{
  struct sigaction sa; 
  build_action(sig, &sa);

  sigaction(sig, , NULL);
}
*/


void handle_int(int sig)
{
  if(DEBUG){write(STDOUT_FILENO, "In handle_int...\n", 17);}
}


void handle_chld(int sig)
{
  if(DEBUG){write(STDOUT_FILENO, "In handle_chld...\n", 18);}
  /*
WIFEXITED is a special macro used to evaluate the child exit status returned from wait() and waitpid(). If it returns a non-zero value then the child process exited normally. The WEXITSTATUS macro indicates the actual exit cause, but only if the child exited normally (see p546 of Kerrisk).
However, WIFSIGNALED and WTERMSIG are used if the process exited due to a signal.
   */
  //Kerrisk, p556
  while(waitpid(-1, NULL, WNOHANG) > 0)
    continue;
}


void reg_handlers()
{
  struct sigaction SIGINT_action = {0};
  struct sigaction SIGCHLD_action = {0};
  struct sigaction SIGTSTP_action = {0};
  struct sigaction ignore_action = {0};

  //SIGCHLD
  SIGCHLD_action.sa_handler = handle_chld;
  sigfillset(&SIGCHLD_action.sa_mask);
  SIGCHLD_action.sa_flags = 0;
  sigaction(SIGCHLD, &SIGCHLD_action, NULL);

  //SIGINT
  SIGINT_action.sa_handler = handle_int;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = 0;
  sigaction(SIGINT, &SIGINT_action, NULL);

  //ignore
  ignore_action.sa_handler = SIG_IGN;
  sigaction(SIGTERM, &ignore_action, NULL);
  sigaction(SIGHUP, &ignore_action, NULL);
  sigaction(SIGSEGV, &ignore_action, NULL);
  sigaction(SIGQUIT, &ignore_action, NULL);

}
/*
SIGUSR2_action.sa_handler = catchSIGUSR2;
sigfillset(&SIGUSR2_action.sa_mask);
SIGUSR2_action.sa_flags = 0;
printf("SIGTERM, SIGHUP, and SIGQUIT are disabled.\n");
printf("Send a SIGUSR2 signal to kill this program.\n");
printf("Send a SIGINT signal to sleep 5 seconds, then kill this program.\n");
while(1)
pause();*/
