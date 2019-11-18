#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include "smshregex.h"
#include "cmdstruct.h"

#ifndef DEBUG
#define DEBUG     0
#endif

/* cat /proc/sys/kernel/pid_max returns 49152 on os1 */
#define PID_MAX_LEN   5


void check_builtin(Cmd *cs)
{

  if(strcmp(cs->the_cmd, "exit") == 0)
    cs->builtin = CASE_EXIT;
  if(strcmp(cs->the_cmd, "status") == 0)
    cs->builtin = CASE_STATUS;
  if(strcmp(cs->the_cmd, "cd") == 0)
    cs->builtin = CASE_CD;
 
  //builtins do not run in the background 
  if(cs->builtin > -1)
    cs->bg = 0;
}


/* pre:   user's command was obtained
 * in:    cmd struct, raw command string
 * out:   n/a
 * post:  cmd structure is appropriately flagged as a comment
 */
void check_comment(Cmd *cs, char *token, int len)
{
  //don't waste cycles on trivial case
  if(len == 1)
  {
    if( *token == '#')
      cs->comment = 1;
  }
  else
  {
    //match an ampersand with zero or more whitespace chars around it right before line's end
    char *pat = "^#.*";
    char *errbuff;  
    regex_t compreg;
    memset(&compreg, 0, sizeof(regex_t));

    int compret = -2;
    //set extended regex flag and the ignore subgroups flag
    if((compret = regcomp(&compreg, pat, REG_EXTENDED | REG_NOSUB)) == 0)
    {
      int execret = -2;
      if((execret = regexec(&compreg, token, 0, NULL, 0)) == 0)
      {
        if(DEBUG){fprintf(stderr, "%s", "Regex match for comment\n");}
        cs->comment = 1;
      }
      else if(execret != 0)
      {
        if(DEBUG){fprintf(stderr, "%s", "Regexec didn't match comment.\n");};
        size_t errbuffsz = regerror(execret, &compreg, 0, 0);
        errbuff = malloc(errbuffsz);
        memset(errbuff, '\0', errbuffsz);
        regerror(execret, &compreg, errbuff, errbuffsz);
        if(DEBUG){fprintf(stderr, "Regexec error: %s\n", errbuff);}
        //set cmd struct to fg execution 
        cs->comment = 0;  
        free(errbuff);
        errbuff = NULL;
      }
    }
    regfree(&compreg);
  }
}


/* pre:   user's command was obtained
 * in:    Cmd struct, index of string argument to be matched
 * out:   0 if no pid sub was made, 1 if it was
 * post:  argument altered, in Cmd struct, with substituted pid
 */
int check_pid(Cmd *cs, char *arg, int argidx)
{
  int submade = 0;
  
  if(strstr(arg, "$$"))
  {
    //get the pid and convert to a string 
    char pidstr[PID_MAX_LEN+1];
    snprintf(pidstr, PID_MAX_LEN+1, "%d", getpid());
    if(DEBUG){fprintf(stderr, "This is pidstr: %s\n", pidstr);} 

    //length of our interpolated string
    int dstlen = strlen(pidstr)+strlen(arg)-1;
    char dst[dstlen];
    memset(dst, '\0', dstlen);
    char *t = NULL;
    //find the first dollar sign and then NULL it and its twin
    t = strchr(arg, '$');
    *t = '\0';
    *(t+1) = '\0';
    strcat(dst, arg);
    if(DEBUG){fprintf(stderr, "Arg after null byte sub: %s\n", arg);}
    strcat(dst, pidstr);
    if(DEBUG){fprintf(stderr, "dst after strcat: %s\n", dst);}
    strcat(dst, t+2);
    if(DEBUG){fprintf(stderr, "dst after 2nd strcat: %s\n", dst);}

    cs->cmd_args[argidx] = malloc(dstlen);
    if(cs->cmd_args[argidx])
      strcpy(cs->cmd_args[argidx], dst); 
    submade = 1;
  }
  return submade;
}


/* pre:   user's command was obtained
 * in:    cmd struct, raw command string
 * out:   n/a
 * post:  cmd structure is appropriately flagged for foreground/background logic
 */
void check_bg(struct cmd *cs, char *cmdline)
{
  //sanity check 
  int len = strlen(cmdline);
  assert(cmdline[len] == '\0');
  assert(cmdline[len-1] == '\n');
  
  //match an ampersand with zero or more whitespace chars around it right before line's end
  char *pat = "^.*[[:space:]]*&[[:space:]]*";
  char *errbuff;  
  regex_t compreg;
  memset(&compreg, 0, sizeof(regex_t));

  int compret = -2;
  //set extended regex flag and the ignore subgroups flag
  if((compret = regcomp(&compreg, pat, REG_EXTENDED | REG_NOSUB)) == 0)
  {
    int execret = -2;
    if((execret = regexec(&compreg, cmdline, 0, NULL, 0)) == 0)
    {
      if(DEBUG){fprintf(stderr, "%s", "Regex match for BG\n");};
      cs->bg = 1;
      //now zap the & so we don't have to mess with it later
      char *amp = strrchr(cmdline, '&');
      *amp = '\n';
      *(amp+1) = '\0';
    }
    else if(execret != 0)
    {
      if(DEBUG){fprintf(stderr, "%s", "Regexec failed.\n");};
      size_t errbuffsz = regerror(execret, &compreg, 0, 0);
      errbuff = malloc(errbuffsz);
      memset(errbuff, '\0', errbuffsz);
      regerror(execret, &compreg, errbuff, errbuffsz);
      if(DEBUG){fprintf(stderr, "Regexec error: %s\n", errbuff);}
      //set cmd struct to fg execution 
      cs->bg = 0;  
      free(errbuff);
      errbuff = NULL;
    }
  }
  regfree(&compreg);
}

