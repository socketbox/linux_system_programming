#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "cmdstruct.h"

void init_cmd_struct(struct cmd *cs)
{
	cs->comment = -1;
  cs->builtin = -1;
  cs->bg = -1;
  cs->redir_in = -1;
  cs->redir_out = -1;
  cs->pidarg = -1;
  cs->pidarg_idx = -1;
  cs->cmd_argc = -1;
  cs->the_cmd = NULL;
  memset(cs->cmd_args, '\0', ARGS_MAX);
}

void print_cmd_struct(struct cmd *cs)
{
  fprintf(stderr, "%s", "Printing cmd struct:\n"); 
  if(cs->the_cmd)
  {
    fprintf(stderr, "Command: %s\n", cs->the_cmd); 
  }
  fprintf(stderr, "To be backgrounded? %i\n", cs->bg);
  fprintf(stderr, "Is a builtin? %i\n", cs->builtin);
  fprintf(stderr, "Is a comment? %i\n", cs->comment);
  fprintf(stderr, "Redirect stdin index: %i\n", cs->redir_in);
  fprintf(stderr, "Redirect stdout index: %i\n", cs->redir_out);
  fprintf(stderr, "PID of shell arg: %i\n", cs->pidarg); 
  fprintf(stderr, "PID arg index: %i\n", cs->pidarg_idx);
  fprintf(stderr, "Arg count: %i\n", cs->cmd_argc);
  for(int i=0; i<cs->cmd_argc; i++)
  {
    fprintf(stderr, "arg%i: %s\n", i, cs->cmd_args[i]);
  }
}



