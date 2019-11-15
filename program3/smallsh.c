#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "smshregex.h"
#include "smshbuiltin.h"
#include "cmdstruct.h"
#include "smshsignals.h"

#ifndef DEBUG
#define DEBUG             0
#endif 

//#define CASE_BG           38  
#define CASE_EXIT         1
#define CASE_CD           2
#define CASE_STATUS       3
#define CASE_PID          164 
#define CASE_RDSTDOUT     62
#define CASE_RDSTDIN      60
#define CASE_COMMENT      35  
#define CMD_LINE_MAX      2048

//https://stackoverflow.com/a/17811492/148680
#define RLIMIT_NPROC      4096


/* pre:   n/a 
 * in:    a string to be characterized
 * out:   an integer 
 * post:  n/a
 */
int get_case(char *cmdstr)
{
  int val = 0;
  char c = '\0';
  
  if(DEBUG){ fprintf(stderr, "Arg to get_case: %s\n", cmdstr);}

  //check for builtin
  if(strlen(cmdstr) > 1)
  {
    if(strcmp(cmdstr, "$$") == 0) 
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
    /*else if(c == '&')
      val = CASE_BG;*/
  }
  return val;
}


void check_builtin(Cmd *cs)
{
  if(strcmp(cs->the_cmd, "exit") == 0)
    cs->builtin = CASE_EXIT;
  if(strcmp(cs->the_cmd, "status") == 0)
    cs->builtin = CASE_STATUS;
  if(strcmp(cs->the_cmd, "cd") == 0)
    cs->builtin = CASE_CD;
}


/*
 * pre:   an array of command arguments in cmd struct argument is partially populated
 * in:    a cmd struct pointer; index of the argument to check; the entire string of user input
 * out:   n/a
 * post:  n/a
 */
void check_arg(struct cmd *cs, int idx)
{
  int c = get_case(cs->cmd_args[idx]);
  
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
    /*case CASE_BG:
      cs->bg = 1;
      break;*/
    case CASE_COMMENT:
      cs->comment = 1;
    default:
      if(DEBUG){fprintf(stderr, "arg %i: %s\n", idx, cs->cmd_args[idx]);}  
  }
}


/* pre:   user has been prompted
 * in:    pointer to a command struct
 * out:   n/a
 * post:  command line fully parsed and cmd struct populated and flagged accordingly
 */
void parse_cmdline(Cmd *cs)
{
  char cmdline[CMD_LINE_MAX] = "\0";
  char *delims = " \n"; 
  char *tkstart = NULL;
  char *tkstate = NULL;
  char **tks = &tkstate; 
  int len = -1;

  //nb: fgets captures through newline
  if( fgets(cmdline, sizeof cmdline, stdin) )
  {
    //before anything, check if we will run in bg
    check_bg(cs, cmdline);

    //get the command
    tkstart = strtok_r(cmdline, delims, tks);
    //if tkstart is NULL, we've got a blank line 
    if(tkstart)
    {
      len = strlen(tkstart);
      
      //check if we've got a comment
      check_comment(cs, tkstart, len);

      if(!cs->comment)
      {
        //to be freed in free_cmd_struct() 
        cs->the_cmd = malloc(len+1);
        //check malloc success 
        if(cs->the_cmd)
        {
          memset(cs->the_cmd, '\0', len+1);
          strcpy(cs->the_cmd, tkstart);
        }

        //check for builtin
        check_builtin(cs);

        int argcnt = 0; 
        while( ( (tkstart = strtok_r(NULL, delims, tks)) != NULL ) && argcnt < ARGS_MAX)
        { 
          len = strlen(tkstart);
          //to be freed in free_cmd_struct() 
          cs->cmd_args[argcnt] = malloc(len+1);
          if(cs->cmd_args[argcnt])
          {
            memset(cs->cmd_args[argcnt], '\0', len+1 );
            strcpy(cs->cmd_args[argcnt], tkstart);
          }
          check_arg(cs, argcnt);
          argcnt++;   
        }
        cs->cmd_argc = argcnt;
      }
    }
  }
}


/*
 * pre:   n/a
 * in:    n/a
 * out:   n/a
 * post:  colon prompt displayed to user
 */
void prompt_user()
{
  fprintf(stdout, "%s", ": ");
  //fflush(stdout);
}


/* pre:   n/a
 * in:    an instance of Cmd struct
 * out:   n/a
 * post:  memory freed
 */
void free_cmd_struct(Cmd cs)
{
  if(cs.the_cmd)
    free(cs.the_cmd);
  for(int i=0; i < cs.cmd_argc; i++)
  {
    if(cs.cmd_args[i]);
      free(cs.cmd_args[i]);
  }
}


int main(int argc, char *argv[])
{
  //why not do this instead of repeated calls to fflush()?
  setbuf(stdout, NULL);

  //register singal handlers
  reg_handlers();

  Cmd cs;
  init_cmd_struct(&cs);

  //pass this to fg process handler and status builtin
  Fgexit fge = {-1, -1};

  int sigtstp = 0;
  int pid_arr[RLIMIT_NPROC] = {0};

  do 
  {
    if(false)
    {
      //display child background process termination
    }
    prompt_user();
    parse_cmdline( &cs ); 
    if(DEBUG){ print_cmd_struct(&cs);}

    if(cs.the_cmd != NULL)
    {
      //must come before check for bg flag
      if(cs.builtin > 0)
      {
        switch(cs.builtin)
        {
          case CASE_EXIT:
            run_exit(pid_arr);
            break;
          case CASE_CD:
            run_cd(&cs);
            break;
          case CASE_STATUS:
            run_status(&fge);
            break;
        }
      }
      //cannot run in bg if flag for SIGTSTP was toggled on
      else if(cs.bg && !sigtstp)
        ;//run_bg_proc(
      else
        run_fg_proc(&cs, &fge);
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
    free_cmd_struct(cs);
  }
  while(true);

  return 0;
}

