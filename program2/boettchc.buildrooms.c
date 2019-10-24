#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define ROOM_CNT 7
#define ONID "boettchc\0"
#define ROOMS "rooms\0"

enum rm_type { START_ROOM, MID_ROOM, END_ROOM };

typedef struct room
{
  char *rm_name;
  int rm_number;
  int rm_type;
  int cx_count;
  unsigned char cxs;
} Room;


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
 * calculates int value given base and exponent
 *
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
    int n;
    res = base;
    for(n = 1; n < exp; n++)
    {
      res *= base;
    }
  }
  return res;
}


/* 
 * creates the room directory
 *
 * pre:     n/a
 * in:      pid of current process
 * out:     n/a
 * post:    room directory exists in cwd
 */
void mk_rm_dir(pid_t pid)
{
  char *delim = ".";
  /* cat /proc/sys/kernel/pid_max returns 49152 on os1 */
  int pid_str_buff_sz = 5 * sizeof(char); 
  char pid_str[pid_str_buff_sz]; 
  sprintf(pid_str, "%i", pid);
  int dir_name_len = strlen(ROOMS) + strlen(ONID) + pid_str_buff_sz + 2;
  char dir_name[dir_name_len];
  memset(dir_name, '\0', sizeof(char)*dir_name_len);
  strcpy(dir_name, ONID);
  strcat(dir_name, delim);
  strcat(dir_name, ROOMS);
  strcat(dir_name, delim);
  strcat(dir_name, pid_str);
  if(DEBUG)
    printf("room dir name: %s\n", dir_name);
}


char* get_rm_name(char *names[])
{
  int idx = get_rand(0, sizeof(names));
  char *n = names[idx];
  if(DEBUG)
    printf("get room name: %s" n);
  return n;
}


Room* gen_rooms(int count, char *names[])
{
  Room *rooms = malloc(count*sizeof(Room));

  char *name = NULL;

  for(int i = 0; i < count; i++)
  {
    if(i = 0)
      type = START_ROOM;
    else if (i = 1)
      type = END_ROOM;
    else
      type = MID_ROOM;

    name = get_rm_name(names);       
  }

  return rooms;
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
  /* Declare and init an array of ten room labels */
  char *ten_rooms[] = {"BC", "Alberta", "Saskatch", "Manitoba", "Ontario", "Quebec", "NewfLab", "NewBrun", "NovaScot", "PrinceEd"};
 
  /* get the PID */
  pid_t pid = getpid();

  /* make room file directory */
  mk_rm_dir(pid);

  /* seed the PRNG */
  srand(time(NULL));
  
  /* make an $ONID.rooms.$PID directory */
  Room *rooms = gen_rooms(ROOM_CNT);  

  free(rooms);
  rooms = NULL;
  return 0;
}


