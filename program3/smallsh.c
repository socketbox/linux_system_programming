#include <unistd.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef CMDSTRUCT_H
#include "cmdstruct.h"
#define CMDSTRUCT_H
#endif 

#ifndef DEBUG
#define DEBUG             0
#endif 

#define CASE_BG           38  
#define CASE_BUILTIN      129
#define CASE_PID          164 
#define CASE_RDSTDOUT     62
#define CASE_RDSTDIN      60
#define CASE_COMMENT      35  
#define CMD_LINE_MAX      2048


int _get_case(char *cmdstr)
{
  int val = 0;
  char c = '\0';
  
  //check for builtin
  if(strlen(cmdstr) > 1)
  {
    if(strcmp(cmdstr, "exit") * strcmp(cmdstr, "status") * strcmp(cmdstr, "cd") == 0)
      val = CASE_BUILTIN;
    else if(strcmp(cmdstr, "$$") == 0) 
      val = CASE_PID;
  }
  else
  {
    c = cmdstr[0];
    if(c == '#')
      val = CASE_COMMENT;
    else if(c == '<')
      val = CASE_RDSTDIN;
    else if(c == '>')
      val = CASE_RDSTDOUT;
    else if(c == '&')
      val = CASE_BG;
  }
  return val;
}


/*
 * pre:   an array of ccommand arguments in Cmd cs argument is partially populated
 * in:    a Cmd structure pointer; the index of an argument to check
 * out:   n/a
 * post:  n/a
 */
void _check_arg(struct cmd *cs, int idx)
{
  int c =_get_case(cs->cmd_args[idx]);
  
  switch(c)
  {
    case CASE_PID:
      cs->pidarg = getpid();
      cs->pidarg_idx = idx;
      break;
    //we know file args for redirection come one arg after the operator
    case CASE_RDSTDOUT:
      cs->redir_out = idx + 1;
      break; 
    case CASE_RDSTDIN:
      cs->redir_in = idx + 1;
      break;
    case CASE_BG:
      cs->bg = 1;
      break;
    case CASE_COMMENT:
      cs->comment = 1;
    default:
      fprintf(stderr, "arg: %s\n", cs->cmd_args[idx]);  
  }
}


void parse_cmdline(Cmd *cs)
{
  char cmdline[CMD_LINE_MAX];
  char *delims = " \n"; 
  char *tkstart = NULL;
  char *tkstate = NULL;
  char **tks = &tkstate; 
  int argcnt = 0; 
  int len = -1;

  if( fgets(cmdline, sizeof cmdline, stdin) )
  {
    //get the command
    //tkstart = strtok_s(cmdline, tksz, delims, tks);
    tkstart = strtok_r(cmdline, delims, tks);
    //if tkstart is NULL, we've got a blank line 
    if(tkstart)
    {
      len = strlen(tkstart);
      cs->the_cmd = malloc(len+1);
      //check malloc success 
      if(cs->the_cmd)
      {
        memset(cs->the_cmd, '\0', len+1);
        strcpy(cs->the_cmd, tkstart);
      }
    }
    while( ( (tkstart = strtok_r(NULL, delims, tks)) != NULL ) && argcnt < ARGS_MAX)
    { 
      len = strlen(tkstart);
      cs->cmd_args[argcnt] = malloc(len+1);
      if(cs->cmd_args[argcnt])
      {
        memset(cs->cmd_args[argcnt], '\0', len+1 );
        strcpy(cs->cmd_args[argcnt], tkstart);
      }
      argcnt++;   
      check_arg(cs);
    }
  }
}


void prompt_user()
{
  fprintf(stdout, "%s", ": ");
  fflush(stdout);
}

void free_cmd(Cmd cs)
{
  free(cs.the_cmd);
  for(int i=0; i < ARGS_MAX; i++)
  {
    free(cs.cmd_args[i]);
  }
}


int main(int argc, char *argv[])
{
  Cmd cmdstruct;

  do 
  {
    if(false)
    {
      //display child background process termination
    }
    prompt_user();
    parse_cmdline( &cmdstruct ); 
    if(cmdstruct.the_cmd)
    {
      if(DEBUG) fprintf(stderr, "Command: %s\n", cmdstruct.the_cmd);
    }

    /*accomodates display requirements for prompt after time command 
    if(u.cmd != 't')
    {
      memset(&u, 0, sizeof(Usrin));
      display_node(curr_node);
    }
    parse_user_input(&u, curr_node); 
    switch(u.cmd)
    {
      case 't':
        show_time(&tf_mutex);
        break;
      case 'n':
        route_user(&u, &curr_node, nodes, route, &steps);
        break;
      case 'h':
        fprintf(stdout, "\n%s\n", HUH);
        fflush(stdout);
        break; 
    }*/
  }
  while(true);

  free_cmd(cmdstruct);
  return 0;
}

