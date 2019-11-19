#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "smshsignals.h"

#ifndef DEBUG
#define DEBUG   0
#endif 

int STOPPED;

void set_smsh_mask()
{
  sigset_t not_tstp;                                      
  sigfillset(&not_tstp);                                  
  //remove SIGTSTP since we want to catch and act on it   
  sigdelset(&not_tstp, SIGTSTP);                          
  //remove SIGCHLD
  sigdelset(&not_tstp, SIGCHLD);                          
  if(sigprocmask(SIG_SETMASK, &not_tstp, NULL) == -1)
      perror("Setting proc. mask for smsh failed");
}

void set_fg_mask()
{
  sigset_t just_tstp;                                      
  //remove all signals except SIGTSTP; we don't want fg processes stopped
  sigemptyset(&just_tstp);                                  
  sigaddset(&just_tstp, SIGTSTP);                          
  if(sigprocmask(SIG_SETMASK, &just_tstp, NULL) == -1)
      perror("Setting proc. mask for fg proc failed");
}

void set_bg_mask()
{
  sigset_t intntstp;                                      
  //remove all signals except SIGTSTP and SIGINT
  sigemptyset(&intntstp);                                  
  sigaddset(&intntstp, SIGTSTP);                          
  sigaddset(&intntstp, SIGINT);                          
  if(sigprocmask(SIG_SETMASK, &intntstp, NULL) == -1)
      perror("Setting proc. mask for bg proc failed");
}


void smsh_chld_hndlr(int sig)
{
  //Kerrisk, pp556-559
  int cpid = 0;
  int em = INT_MAX; 
  int status = INT_MAX; 
  if(DEBUG){fprintf(stderr, "In SIGCHLD hanlder!");}
  while((cpid = waitpid(-1, &em, WNOHANG)) > 0)
  {
      if (WIFEXITED(em))
      {
        if(DEBUG){fprintf(stderr, "The process exited normally\n");}
        status = WEXITSTATUS(em);
        fprintf(stderr, "background pid %i is done: exit value %i\n", cpid, status);
      }
      else if(WIFSIGNALED(em))
      {
        status = WTERMSIG(em);
        fprintf(stderr, "terminated by signal %i\n", status);
      }
  }
}


void smsh_tstp_hndlr(int sig)
{
  if(!STOPPED)
  {
    char *msg1 = "Entering foreground-only mode (& is now ignored)\0";
    write(STDOUT_FILENO, msg1, 50);
    STOPPED=1;
  }
  else
  {
    char *msg2 = "Exiting foreground-only mode";
    write(STDOUT_FILENO, msg2, 30);
    STOPPED=0;
  }
}


void reg_smsh_handlers()
{
  //create and assign handler to SIGTSTP
  struct sigaction smsh_act = {0};
  
  sigset_t hndl_tstp;
  sigfillset(&hndl_tstp);
  //we want to not block SIGCHLD sigdelset(&hndl_tstp, SIGTSTP);
  sigdelset(&hndl_tstp, SIGCHLD);
  smsh_act.sa_mask = hndl_tstp;
  
  smsh_act.sa_handler = smsh_tstp_hndlr;
  smsh_act.sa_flags = 0;

  sigaction(SIGTSTP, &smsh_act, NULL);

  //create and assign handler to SIGCHLD
  struct sigaction smsh_actchld = {0};
  
  sigset_t hndl_chld;
  sigfillset(&hndl_chld);
  sigdelset(&hndl_chld, SIGTSTP);
  sigdelset(&hndl_chld, SIGCHLD);
  smsh_actchld.sa_mask = hndl_chld;
  
  smsh_actchld.sa_handler = smsh_chld_hndlr;
  smsh_actchld.sa_flags = 0;

  if(sigaction(SIGCHLD, &smsh_actchld, NULL) == -1)
    perror("Call to sigaction failed to register SIGCHLD handler");

}
