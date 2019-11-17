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
#include "smshchild.h"

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
    fprintf(stderr, "c in cmdstr: %c\n", c);
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
      if(DEBUG){fprintf(stderr, "wrong case: %c\n", c);}
      cs->redir_out = idx + 1;
      break; 
    case CASE_RDSTDIN:
      if(DEBUG){fprintf(stderr, "wrong case: %c\n", c);}
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
    /*eat remains in buffer                              
    if (strchr(cmdline, '\n') == NULL)                    
    {                                                    
      int ch;                                            
      while ((ch = fgetc(stdin)) != '\n' && ch != EOF);  
    } */                                                   

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

      if(cs->comment > -1)
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
  
        //status and exit don't take args, so we can skip the arg loop
        if(cs->builtin == -1 || cs->builtin == CASE_CD)
        {
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
  fflush(stdout);
}


int main(int argc, char *argv[])
{
  //why not do this instead of repeated calls to fflush()?
  //setbuf(stdin, NULL);
  //setbuf(stdout, NULL);

  //register signal handlers
  //reg_handlers();
  
  Cmd cs = {0};

  //pass this to fg process handler and status builtin
  Fgexit fge = {0};

  //use a bitfield to track state
  State st = {0};

  int sigtstp = 0;
  //intended to hold pids if I'm incapable of coding SIGCHLD listener 
  int pid_arr[RLIMIT_NPROC] = {0};

  do 
  {
    //reset the cmd struct
    init_cmd_struct(&cs);
   
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
          default:
            fprintf(stderr, "No builtin with that name. Exiting.");
            exit(-666);
        }
      }
      //cannot run in bg if flag for SIGTSTP was toggled on
      else if(cs.bg && !sigtstp)
        ;//run_bg_proc(
      else
        run_fg_child(&cs, &fge);
    }
  }
  while(true);
  //TODO - ojo
  free_cmd_struct(&cs);

  return 0;
}

