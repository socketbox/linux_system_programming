#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <regex.h>

#define MATCH_CNT   4


void print_raw_matches(char *str, regmatch_t matches[])
{
  printf("Regexec successful.\n");
  int i = 0; 
  printf("String length: %i\n", strlen(str)); 
  for(; i < MATCH_CNT; i++)
  {
    if(i == 0)
      printf("Whole string: %s\n", str+matches[0].rm_so);
    printf("Match %i:\n", i);
    printf("Match %i rm_so: %i\n", i, matches[i].rm_so);
    printf("Match %i rm_eo: %i\n", i, matches[i].rm_eo);
    printf("----------\n"); 
  }
}


void print_matches(char* str, regmatch_t matches[])
{
  printf("Regexec successful.\n");
  int i = 0; 
  for(; i < MATCH_CNT; i++)
  {
    if(matches[i].rm_eo != -1)
    {
      printf("Match %i; Beginning:\t%i\n", i, matches[i].rm_so);
      printf("Match %i; End:\t\t%i\n", i, matches[i].rm_eo);
    }
  }
}

int main(int argc, char *argv[])
{
  if(argc != 3)
    fprintf(stderr, "retester <pattern> <string>\n"); 
  
  char *pat = argv[1];
  char *str = argv[2];

  regex_t compreg;
  memset(&compreg, 0, sizeof(regex_t));

  regmatch_t matches[MATCH_CNT];
  memset(&matches, 0, MATCH_CNT*sizeof(regmatch_t));
  int matchcnt = -1;

  char *errbuff;
  printf("Trying to match with basic regex...\n");
  int compret = -2;
  if((compret = regcomp(&compreg, pat, 0)) == 0)
  {
    printf("Compiling successful.\n");
    int execret = -2; 
    if((execret = regexec(&compreg, str, matchcnt, matches, 0)) == 0)
    {
      printf("regexec successful.\n");
      int i = 0; 
      for(; i < MATCH_CNT; i++)
      {
        if(matches[i].rm_so != -1)
        {
          printf("Beginning of match %i: \n", matches[i].rm_so);
          printf("End of match %i: \n", matches[i].rm_eo);
        }
      }
    }
    else if(execret != 0)
    {
      printf("Regexec failed.\n");
      size_t errbuffsz = regerror(execret, &compreg, 0, 0);
      errbuff = malloc(errbuffsz);
      memset(errbuff, '\0', errbuffsz);
      regerror(execret, &compreg, errbuff, errbuffsz);
      fprintf(stderr, "Regexec error: %s", errbuff);
    }
  }
  else
  {
    printf("Compiling failed.\n");
    size_t errbuffsz = regerror(compret, &compreg, 0, 0);
    errbuff = malloc(errbuffsz);
    memset(errbuff, '\0', errbuffsz);
    regerror(compret, &compreg, errbuff, errbuffsz);
    fprintf(stderr, "Regexec error: %s\n", errbuff);
  }
  //reset the buffers
  memset(&compreg, 0, sizeof(regex_t));
  memset(&matches, 0, MATCH_CNT*sizeof(regmatch_t));
  matchcnt = -1;

  printf("\nTrying to match with extended regex...\n");
  compret = -2;
  //set REG_EXTENDED flag
  if((compret = regcomp(&compreg, pat, REG_EXTENDED)) == 0)
  {
    printf("Compiling successful.\n");
    int execret = -2; 
    if((execret = regexec(&compreg, str, matchcnt, matches, 0)) == 0)
    {
      print_raw_matches(str, matches);
    }
    else if(execret != 0)
    {
      printf("Regexec failed.\n");
      size_t errbuffsz = regerror(execret, &compreg, 0, 0);
      errbuff = malloc(errbuffsz);
      memset(errbuff, '\0', errbuffsz);
      regerror(execret, &compreg, errbuff, errbuffsz);
      fprintf(stderr, "Regexec error: %s\n", errbuff);
    }
  }
  else
  {
    printf("Compiling failed.\n");
    size_t errbuffsz = regerror(compret, &compreg, 0, 0);
    errbuff = malloc(errbuffsz);
    memset(errbuff, '\0', errbuffsz);
    regerror(compret, &compreg, errbuff, errbuffsz);
    fprintf(stderr, "Regexec error: %s", errbuff);
  }
  free(errbuff);
  errbuff = NULL; 
  regfree(&compreg);
  return 0;

}
