#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>

/*
 * generates an unsigned int between low and high, inclusive of both
 * taken from: https://en.cppreference.com/w/c/numeric/random/rand
 *
 * pre:   srand has been called to seed the PRNG
 * in:    low and high integer values
 * out:   a random, unsigned integer between low and high, inclusive
 * post:  n/a
 */
int get_rand( unsigned int low, unsigned int high)
{
  int r = rand();
  r = low + ( r / ( (RAND_MAX + 1u) / (high + 1) ) );
  return r;
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
  printf("INT_MAX: %i\n", INT_MAX); 
  printf("LONG_MAX: %lu\n", LONG_MAX); 

  //getting a specific char in the array of strings passed in as args 
  //char  foo = ( (*(args+3))[3] );
  //printf("This is foo: %c", foo);

  int counts[high-low+1];
  memset(counts, 0, (sizeof(int)*high));
  //seed the PRNG
  srand(time(NULL));

  int x = -1;
  for(int n = 0; n < its; n++)
  {
    x = get_rand(low, high);
    counts[x+low]++; 
  }

  double p = .0;

  for (int j = 0; j < high; j++)
  {
    printf("Count %i: %i\n", j, counts[j]);
    p = counts[j]/(its+.0);
    p *= 100;
    printf("Percentage %i: %f\n", j, p);
  } 
  
  return 0;
}


