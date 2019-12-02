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

int are_connected(Room rm1, Room rm2)
{
  int retval = 0;
  printf("rm1.rm_number: %c", rm1.cxs);
  printf("rm2.rm_number: %c", rm2.cxs);
  int anded = rm1.cxs & rm2.cxs;
  int powed = powi(2, rm1.rm_number) + powi(2, rm2.rm_number);
  if ( anded == powed) 
    retval = 1;
  return retval;
}

int main( int argc, char **args )
{
  char *ten_rooms[] = {"BC", "Alberta", "Saskatch", "Manitoba", "Ontario", "Quebec", "NewfLab", "NewBrun", "NovaScot", "PrinceEd"};
  
  struct Room rm0 = {ten_rooms[5], 0, MID_ROOM, 3, '\x17' };
  struct Room rm3 = {ten_rooms[3], 3, MID_ROOM, 2, '\x2C' };
  struct Room rm4 = {"Saskatch", 4, MID_ROOM, 3, '\x71' };
  struct Room rm5 = {"Alberta", 5, END_ROOM, 3, '\x3A' };
  
  //seed the PRNG
  srand(time(NULL));

  int cond1 = are_connected(rm3, rm5);
  int cond2 = are_connected(rm0, rm4);
  int cond3 = are_connected(rm0, rm5);
  printf("Connected rm3 and rm5: %i\n", cond1);
  printf("Connected rm0 and rm4: %i\n", cond2);
  printf("Connected rm0 and rm5: %i\n", cond3);

  // Declare and init an array of ten room labels
  // # make an $ONID.rooms.$PID directory

  return 0;
}


