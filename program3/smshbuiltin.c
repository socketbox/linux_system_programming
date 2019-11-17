#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include "cmdstruct.h"
#include "smshbuiltin.h"


#ifndef DEBUG
#define DEBUG   0
#endif


void run_exit(int pid_arr[])
{
  if(DEBUG){fprintf(stderr, "%s\n", "In run_exit...");}
  kill(0,SIGKILL);
}

void run_cd(Cmd *cs)
{
  char buff[PATH_MAX] = {'\0'};
  int retval = -2;
  //if the user did supply a directory arg...
  if(cs->cmd_argc > 0)
  {
    char *cd_arg = cs->cmd_args[0];
    //check to see if it's not absolute 
    if(cd_arg[0] != '/')
    {    
      realpath(cd_arg, buff);
      retval = chdir(buff);
    }
    else
      retval = chdir(cd_arg);
  }
  else 
  {
    //going to user's $HOME; check that cwd isn't $HOME
    char envbuff[PATH_MAX] = {'\0'};
    strcpy(envbuff, getenv("HOME"));
    getcwd(buff, PATH_MAX);
    if( strcmp(envbuff, buff) != 0 )
    {
      retval = chdir(envbuff);
    }
  }
  if(DEBUG)
  {
    fprintf(stderr, "chdir in cd returned: %i\n", retval);   
    fprintf(stderr, "cwd now: %s\n", getcwd(buff, PATH_MAX));
    if(retval == -1)
    { 
      perror("chdir perror");
    }
  }
}

void run_status(Fgexit *fge, State *st)
{
  //no foreground process has been run, so we return 0
  if(st->fg_init == 0)
    fprintf(stderr, "exit value %i\n", st->fg_init);
  if(st->fg_init && fge->signal > INT_MIN)
    fprintf(stderr, "terminated by signal %i\n", fge->signal);
  if(st->fg_init && fge->status > INT_MIN)
    fprintf(stderr, "exit value %i\n", fge->status);
}

