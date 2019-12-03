#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RAND_LOW    65 
#define RAND_HIGH   91

#ifndef DEBUG
#define DEBUG       0
#endif

/*                                                                   
 * generates an unsigned int between low and high, inclusive of both 
 * taken from: http://www.pcg-random.org/posts/bounded-rands.html    
 *                                                                   
 * pre:   srand has been called to seed the PRNG                     
 * in:    low and high integer values                                
 * out:   a random, unsigned integer between low and high, inclusive 
 * post:  n/a                                                        
 */                                                                  
unsigned int get_rand( unsigned int low, unsigned int high)                   
{                                                                    
  unsigned int range = high - low + 1;                               
  int r, t;                                                          
  r = t = -1;                                                        
  do                                                                 
  {                                                                  
    r = rand();                                                      
    t = r % range;                                                   
  }                                                                  
  //use overflow trick to calculate with a full 2^32                 
  while( r - t > (-range));                                          
  return t + low;                                                    
}                                                                    


int main(int argc, char **argv)
{
  setbuf(stdout, NULL);
  srand(time(0));

  int keylen = atoi(argv[1]);
  if(DEBUG){ fprintf(stderr, "Args to keygen...argc: %i, keylen: %i, argv[2]: %s\n", argc, keylen, argv[1]); } 

  //need an extra char for null byte
  char buff[keylen+1];
  memset(buff, '\0', keylen+1);
  char c = 0; 
  int r = 0; 
  for(int i = 0; i < keylen; i++)
  {
    r = get_rand(RAND_LOW, RAND_HIGH);
    if(r == 91)
      c = 32;
    else
      c = r;
    buff[i] = c; 
  }
  puts(buff);
  return 0;
}

