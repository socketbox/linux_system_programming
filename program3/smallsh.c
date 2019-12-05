#include <limits.h>
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

#define CASE_PID          164 
#define CASE_RDSTDOUT     62
#define CASE_RDSTDIN      60
#define CASE_COMMENT      35  
#define CMD_LINE_MAX      2048

//https://stackoverflow.com/a/17811492/148680
#define RLIMIT_NPROC      4096


/*
 * pre:   an array of command arguments in cmd struct argument is partially populated
 * in:    a cmd struct pointer; index of the argument to check; the entire string of user input
 * out:   n/a
 * post:  n/a
 */
void check_redir(struct cmd *cs, int idx)
{
    char *arg = cs->cmd_args[idx];
   //we know file args for redirection come one arg after the operator
    if(strlen(arg) == 1)
    { 
      if(strcmp(arg, ">") == 0)
        cs->redir_out = idx + 1;
      else if( strcmp(arg, "<") == 0)
        cs->redir_in = idx + 1;
    }
    else
    {
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
  char *tkarg = NULL;
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

      if(cs->comment < 1)
      {
        //to be freed in free_cmd_struct() 
        cs->the_cmd = malloc(len+1);
        //check malloc success 
        if(cs->the_cmd)
        {
          strcpy(cs->the_cmd, tkstart);
        }
        //check for builtin
        check_builtin(cs);
  
        //status and exit don't take args, so we can skip the arg loop
        if(!(cs->builtin == CASE_STATUS || cs->builtin == CASE_EXIT))
        {
          int argcnt, pid_sub;
          argcnt = pid_sub = 0; 
          while( ( (tkarg = strtok_r(NULL, delims, tks)) != NULL ) && argcnt < ARGS_MAX)
          { 
            len = strlen(tkarg);
            //need to check for $$ and sub. here, or realloc within the arg array later 
            if((pid_sub = check_pid(cs, tkarg, argcnt)) == 0 )
            { 
              //to be freed in free_cmd_struct() 
              cs->cmd_args[argcnt] = malloc(len+1);
              if(cs->cmd_args[argcnt])
              {
                strcpy(cs->cmd_args[argcnt], tkarg);
              }
              check_redir(cs, argcnt);
            }
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

int STOPPED;

int main(int argc, char *argv[])
{
  STOPPED = 0;
  //why not do this instead of repeated calls to fflush()?
  //setbuf(stdout, NULL);

  //set the default process mask (accept SIGCHLD, SIGINT and block rest)
  set_smsh_mask();
  
  //register signal handlers
  reg_smsh_handlers();
 
  //better to do Cmd *cs = calloc(sizeof(Cmd)) and keep everything in the heap?
  Cmd cs = {0};

  //pass this to fg process handler and status builtin
  Fgexit fge = {INT_MIN, INT_MIN};

  //TODO: must be passed to SIGTSTP handler or modified using results thereof
  //use a bitfield to track state
  State st = {0};

  do 
  {
    //reset the cmd struct
    init_cmd_struct(&cs);

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
          /*set state, assuming we will run a builtin cmd
           * ideally, each of these functions would return an exit code of 
           * some sort*/
          st.builtin_cmd = 1;
          st.fg_cmd = st.bg_cmd = 0;
          case CASE_EXIT:
            run_exit(cs);
            break;
          case CASE_CD:
            run_cd(&cs);
            break;
          case CASE_STATUS:
            run_status(&fge, &st);
            break;
          default:
            fprintf(stderr, "No builtin with that name. Exiting.");
            exit(-1);
            break;
        }
      }
      else
      {
        //this is awkward, but we must wait to reset status until
        //we're sure that the status builtin hasn't been called 
        fge.status = fge.signal = INT_MIN;
      
        //cannot run in bg if flag for SIGTSTP was toggled on
        if(cs.bg)
        {
          if(STOPPED)
          {
            /* set bg to 0 and exit this block, entering the next to run as an
             * fg command */ 
            cs.bg = 0;
          }
          else
          {
            //set state, assuming we will run a background cmd
            st.bg_cmd = 1;
            st.fg_cmd = st.builtin_cmd = 0;
            run_bg_child(&cs);
          }
        }
        //check for bg char sets cs.bg to 0 on failure
        if(!cs.bg)
        {
          /* Kerrisk advises the use of sigsuspend for this scenario (pp464-467),
           * but what if there is no bg process to produce SIGCHLD? Then 
           * wouldn't the smallsh parent remain paused indefinitely? 
           */
          sigset_t orig_mask, chld_mask;
          sigemptyset(&chld_mask);
          //neither SIGCHLD nor SIGTSTP should interrupt the fg command 
          sigaddset(&chld_mask, SIGTSTP);
          sigaddset(&chld_mask, SIGCHLD);
          if(sigprocmask(SIG_BLOCK, &chld_mask, &orig_mask) == -1)
            perror("Failed to set SIGCHLD mask before fg process");

          //set state, assuming we will run a foreground cmd
          st.fg_cmd = st.fg_init = 1;
          st.bg_cmd = st.builtin_cmd = 0;
          run_fg_child(&cs, &fge);

          /* we come back from the FG process (hopefully), and unblock the SIGCHLD
           * signal.*/
          if(sigprocmask(SIG_SETMASK, &orig_mask, NULL) == -1)
            perror("Failed to reset smallsh procmask after fg exit");

        }
      }
    }
  }
  while(true);
  
  free_cmd_struct(&cs);

  return 0;
}

