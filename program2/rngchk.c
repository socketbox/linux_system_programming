#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>

#ifndef DEBUG
#define DEBUG 0
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
int get_rand( unsigned int low, unsigned int high)
{
  unsigned int range = high - low + 1;
  int r, t;
  r = t = -1;
  do
  {
    r = rand();
    t = r % range;
  }
  while( r - t > (-range));
  return t + low;
    
  /*
   * unbiased div approach
   *
  int t = -1;
  unsigned int range = high - low + 1;
  unsigned int div = ((-range)/range) + 1;
  int r = rand(); 
  if (div == 0)
    t = 0;
  while(t > range)
  {
    r = rand();
    t = r/div;
  }
  return low + t;
  */
}

//checks the distribution of a billion randomly generated numbers
//in: low, high
//out: a printed perecentage for each generated value from low to high
int main( int argc, char **args )
{
  if(argc != 4)
  {
    printf("Usage: rngchk low high iterations\n\
        where low is an integer indicating the lowest number to be generated\n\
        high is an integer indicating the highest number to be generated\n\
        and iterations is a long indicating the count of numbers to be generated.\n");
    exit(1); 
  }
  unsigned int low = atoi( *(args+1) );
  unsigned int high = atoi( *(args+2) );
  unsigned long its = atol( *(args+3) );

  printf("Applicable limits on this system:\n");
  printf("RAND_MAX: %i\n", RAND_MAX); 
  printf("INT_MAX: %i\n", INT_MAX); 
  printf("LONG_MAX: %lu\n", LONG_MAX); 

  //getting a specific char in the array of strings passed in as args 
  //char  foo = ( (*(args+3))[3] );
  //printf("This is foo: %c", foo);

  //array to tally the times a specific int value is generated
  int len = high-low+1; 
  if(DEBUG)
    printf("range is %i\n", len);
  int counts[len];
  memset(counts, 0, (sizeof(int)*len));
  //seed the PRNG
  srand(time(NULL));

  int x = -1;
  for(int n = 0; n < its; n++)
  {
    x = get_rand(low, high);
    if(DEBUG)
      printf("iter %i: %i\n", n, x);
    counts[x-low]++; 
  }

  double p = .0;

  for (int j = 0; j < len; j++)
  {
    printf("Count %i: %i\n", j, counts[j]);
    p = counts[j]/(its+.0);
    p *= 100;
    printf("Percentage %i: %f\n", j, p);
  } 
  
  return 0;
}


