#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include "smshregex.h"

#ifndef CMDSTRUCT_H
#include "cmdstruct.h"
#define CMDSTRUCT_H
#endif

#ifndef DEBUG
#define DEBUG     0
#endif


/* pre:   user's command was obtained
 * in:    cmd struct, raw command string
 * out:   n/a
 * post:  cmd structure is appropriately flagged as a comment
 */
void check_comment(struct cmd *cs, char *token, int len)
{
  //don't waste cycles on trivial case
  if(len == 1)
  {
    if(token[0] == '#')
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

