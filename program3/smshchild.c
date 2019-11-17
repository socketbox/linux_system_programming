#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "cmdstruct.h"
#include "smshchild.h"

//fork, then call exec..(), this will allow the exec'd process to catch the SIGINT you block

/*
 * Most of the code in this file is derived from Lecture 3.4
 */
int redir_stdin(Cmd *cs)
{
  int fd = -2;
  //get path from arg array in Cmd struct
  char *path = cs->cmd_args[cs->redir_in];
  fd = open(path, O_RDONLY);
  if(fd == -1)
  { 
    perror("Failed to open file for stdout redirect"); 
    exit(1); 
  }
  int dupres = dup2(fd, 1);
  if(dupres == -1)
  { 
    perror("Failed to duplicate stdout to fd"); 
    exit(1); 
  }
  return fd;
}

int redir_stdout(Cmd *cs)
{
  //get path from arg array in Cmd struct
  char *path = cs->cmd_args[cs->redir_out];
  int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if(fd == -1)
  { 
    perror("Failed to open file for stdout redirect"); 
    exit(1); 
  }
  int dupres = dup2(fd, 1);
  if(dupres == -1)
  { 
    perror("Failed to duplicate stdout to fd"); 
    exit(1); 
  }
  return fd;
}

/*
 * create an array of arguments that can be passed to exec
 */
void prep_args(Cmd *cs, char *arg_arr[])
{
  if(DEBUG){fprintf(stderr, "%s\n", "In prep_args...");}
 
  //execvp requires the cmd to be first element in arg array
  int len = strlen(cs->the_cmd);
  arg_arr[0] = malloc(len+1);
  memset(arg_arr[0], '\0', len+1);
  strncpy(arg_arr[0], cs->the_cmd, len+1);
 
  //probably should have thought this through a little bit more...
  int aaidx = 1; 
  for(int i = 0; i < cs->cmd_argc; i++)
  {
    //we skip over arguments related to i/o redirection, hence the additional counter aaidx
    //stepping through this in gdb looks like it's a performance hit
    if(!(i == cs->redir_in || i == (cs->redir_in - 1) || 
      i == cs->redir_out || i == (cs->redir_out - 1) ) )
    {
      len = strlen(cs->cmd_args[i]);
      arg_arr[aaidx] = malloc(len+1);
      memset(arg_arr[aaidx], '\0', len+1);
      strncpy(arg_arr[aaidx], cs->cmd_args[i], len+1);
      aaidx++;
    }
  }
  //to mark the end of args, as spec'd in execvp
  arg_arr[aaidx] = NULL;
  if(DEBUG)
  {
    for(int x=0; x < cs->cmd_argc; x++)
    {
      fprintf(stderr, "arg_arr[%i]: %s\n", x+1, arg_arr[x+1]);
      fprintf(stderr, "cmd_args[%i]: %s\n", x, cs->cmd_args[x]);
    }
  }
}

/*
 * Finally run the_cmd
 */
void exec_fg_cmd(Cmd *cs, char *arg_arr[])
{
  execvp(cs->the_cmd, arg_arr);
  perror("execvp failed in exec_fg_cmd");
  //fflush(stdout);
}

pid_t run_fg_child(Cmd *cs, Fgexit *fge)
{
  //TODO: probably should be passed in from the caller
  int em;

  //prepare the arguments for call to exec 
  char *arg_arr[cs->cmd_argc];
  prep_args(cs, arg_arr);
  
  pid_t spawnpid = -2;
  spawnpid = fork();

  switch(spawnpid)
  {
    case -1:
      perror("fork in run_fg_proc returned error");
      exit(1);
      break;
    case 0:
      if(cs->redir_out > -1)
        redir_stdout(cs);
      if(cs->redir_in > -1)
        redir_stdin(cs);
      exec_fg_cmd(cs, arg_arr);
      break;
    default:
      if(DEBUG){fprintf(stderr, "In parent's case; spawnpid: %i\n", spawnpid);}
      spawnpid = waitpid(spawnpid, &em, 0); 
      if (WIFEXITED(em))
      {
        if(DEBUG){fprintf(stderr, "The process exited normally\n");}
        fge->status = WEXITSTATUS(em);
      }
      else if(WIFSIGNALED(em))
      {
        if(DEBUG){fprintf(stderr, "The process was signalled.\n");}
        fge->signal = WTERMSIG(em);
      }
      break;
  }
  //TODO: free the args array 
  return spawnpid;  
}


