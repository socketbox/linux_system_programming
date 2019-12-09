#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

void error(const char *msg) 
{ 
  if(errno > 0)
    perror(msg); 
  else
    fprintf(stderr, "%s\n", msg);
  exit(1); 
}


