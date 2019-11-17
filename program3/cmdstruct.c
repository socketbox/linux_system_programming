#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "cmdstruct.h"


/* pre:   n/a
 * in:    an instance of Cmd struct
 * out:   n/a
 * post:  memory freed
 */
void free_cmd_struct(Cmd *cs)
{
  if(cs->the_cmd != NULL)
  {  
    free(cs->the_cmd);
  }
  for(int i=0; i < cs->cmd_argc; i++)
  {
    if(cs->cmd_args[i] != NULL);
      free(cs->cmd_args[i]);
  }
}


void init_cmd_struct(Cmd *cs)
{
	cs->comment = INT_MIN;
  cs->builtin = INT_MIN;
  cs->bg = INT_MIN;
  cs->redir_in = INT_MIN;
  cs->redir_out = INT_MIN;
  cs->pidarg = INT_MIN;
  cs->pidarg_idx = INT_MIN;
  cs->cmd_argc = INT_MIN;
  cs->the_cmd = NULL;
  for(int x=0; x<ARGS_MAX; x++){ cs->cmd_args[x] = NULL;}
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


