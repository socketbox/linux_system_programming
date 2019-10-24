#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ONID boettchc

enum rm_type { START_ROOM, MID_ROOM, END_ROOM };

typedef struct Room
{
  char *rm_name;
  int rm_number;
  int rm_type;
  int cx_count;
  unsigned char cxs;
} Room;


/* no exponent operator in C? No pow() variant that takes and returns an int?
 * Laaaamme.
 * pre:   n/a
 * in:    two unsigned ints, a base and an exponent
 * out:   an int
 * post:  n/a
 */
int powi(unsigned int base, unsigned int exp)
{
  int res = 0;
  if(exp == 0)
    res = 1;
  else
  {
    res = base;
    for(int n = 1; n < exp; n++)
    {
      res *= base;
    }
  }
  return res;
}


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


/*
 * tests to see if two rooms are connected
 *
 * pre:   n/a
 * in:    two Room structures
 * out:   an integer indicating connectedness
 * post:  n/a
 */
int are_connected(Room rm1, Room rm2)
{
  int retval = 0;
  int anded = rm1.cxs & rm2.cxs;
  int powed = powi(2, rm1.rm_number) + powi(2, rm2.rm_number);
  if ( (double)anded == powed) 
    retval = 1;
  return retval;
}


int main( int argc, char **args )
{
  //seed the PRNG
  srand(time(NULL));

  // Declare and init an array of ten room labels
  char *ten_rooms[] = {"BC", "Alberta", "Saskatch", "Manitoba", "Ontario", "Quebec", "NewfLab", "NewBrun", "NovaScot", "PrinceEd"};
  
  // # make an $ONID.rooms.$PID directory
  

  return 0;
}


