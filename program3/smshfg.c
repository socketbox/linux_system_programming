#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "cmdstruct.h"

//fork, then call exec..(), this will allow the exec'd process to catch the SIGINT you block

void run_fg_proc(Cmd *cs, Fgexit *fg)
{
  pid_t spawnpid = -5;
  int ten = 10;
  spawnpid = fork();
  switch (spawnpid)
  {
    case -1:
      perror("fork in run_fg_proc returned error");
      exit(1);
      break;
    case 0:
      if(cs->redir_out > -1)
        redirec_stdout(cs);
      if(cd->redir_in > -1)
        redirec_stdout(cs);
      char *arg_arr[cs->cmd_argc] = {'\0'};
      prep_args(cs, arg_arr);
      exec_cmd(cs, arg_arr);
      break;
    default:
      printf("I am the parent! ten = %d\n", ten);
      break;
  }
  
}


