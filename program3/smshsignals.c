#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include "smshsignals.h"
#include "cmdstruct.h"

#define PID_MAX_LEN   5

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


void smsh_chld_hndlr(int sig, siginfo_t *sigi, void *ucontext)
{
  //Kerrisk, pp556-559
  int cpid = 0;
  int em = INT_MAX; 
  int status = INT_MAX; 
  if(SIGDEBUG)
  {
    fprintf(stderr, "smsh_chld_hndlr, before cpid eval: cpid=%i; exit status %i from %i\n", \
        cpid, sigi->si_pid, sigi->si_status);
    perror("waitpid in amsh_chld_hndlr returned error");
  }
  while((cpid = waitpid(-1, &em, WNOHANG)) > 0)
  {
    if(SIGDEBUG)
      fprintf(stderr, "smsh_chld_hndlr, after waitpid: cpid=%i; exit status %i from %i\n", \
          cpid, sigi->si_pid, sigi->si_status);

    if (WIFEXITED(em))
    {
      if(DEBUG){fprintf(stderr, "The process exited normally\n");}
      status = WEXITSTATUS(em);
      //fprintf(stderr, "background pid %i is done: exit value %i\n", cpid, status);
      char *msg1 = "background pid ";
      //pid + spaces + null
      char msg2[PID_MAX_LEN+1] = {'\0'};
      char *msg3 = " is done: exit value ";
   
      write(STDOUT_FILENO, msg1, 15);
      write(STDOUT_FILENO, msg2, 5);
      write(STDOUT_FILENO, msg3, 22);
      write(STDOUT_FILENO, "msg3, 22);
    }
    else if(WIFSIGNALED(em))
    {
      status = WTERMSIG(em);
      fprintf(stderr, "terminated by signal %i\n", status);
    }
  }
}


void smsh_tstp_hndlr(int sig, siginfo_t *sigi, void *ucontext)
{
  if(SIGDEBUG)
    fprintf(stderr, "shsh_tstp_hndlr: from %i\n, exit status: %i", \
        sigi->si_pid, sigi->si_status);

  if(!STOPPED)
  {
    char *msg1 = "Entering foreground-only mode (& is now ignored)\n";
    write(STDOUT_FILENO, msg1, 50);
    STOPPED=1;
  }
  else
  {
    char *msg2 = "Exiting foreground-only mode\n";
    write(STDOUT_FILENO, msg2, 30);
    STOPPED=0;
  }
}


void reg_smsh_handlers()
{
  //create and assign handler to SIGTSTP
  struct sigaction smsh_act = {{0}};
  
  sigset_t hndl_tstp;
  sigfillset(&hndl_tstp);
  //we want to not block SIGCHLD sigdelset(&hndl_tstp, SIGTSTP);
  //sigdelset(&hndl_tstp, SIGCHLD);
  smsh_act.sa_mask = hndl_tstp;
  
  smsh_act.sa_sigaction = smsh_tstp_hndlr;
  smsh_act.sa_flags = SA_SIGINFO;

  sigaction(SIGTSTP, &smsh_act, NULL);

  //create and assign handler to SIGCHLD
  struct sigaction smsh_actchld = {{0}};
  
  sigset_t hndl_chld;
  sigfillset(&hndl_chld);
  //sigdelset(&hndl_chld, SIGTSTP);
  sigdelset(&hndl_chld, SIGCHLD);
  smsh_actchld.sa_mask = hndl_chld;
  
  smsh_actchld.sa_sigaction = smsh_chld_hndlr;
  smsh_actchld.sa_flags = SA_SIGINFO;

  if(sigaction(SIGCHLD, &smsh_actchld, NULL) == -1)
    perror("Call to sigaction failed to register SIGCHLD handler");

}
