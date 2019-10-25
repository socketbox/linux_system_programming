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
#define NAME_LEN 9
#define ONID "boettchc\0"
#define ROOMS "rooms\0"

enum RM_TYPE { START_ROOM, MID_ROOM, END_ROOM };
char *ROOM_TYPES[] = { "START_ROOM", "MID_ROOM", "END_ROOM" };

typedef struct room
{
  char rm_name[NAME_LEN];
  int rm_number;
  int rm_type;
  int cx_count;
  unsigned char cxs;
} Room;


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
  //use overflow trick to calculate with a full 2^32
  while( r - t > (-range));                        
  return t + low;                                  
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


void get_rm_name(char *names[], Room *rm)
{
  int idx = -1;
  memset(rm->rm_name, '\0', NAME_LEN);
  
  do
  {
    idx = get_rand(0, sizeof(names));
    if(names[idx] != NULL)
    {
      strcpy(rm->rm_name, names[idx]);
      names[idx]=NULL;
    }
  }
  while(rm->rm_name[0] == '\0');
  
  if(DEBUG)
    printf("get room name: %s\n", rm->rm_name);
 
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


void connect_rooms( Room *rm )
{
  int cx_count = -1;
  int rm_cx = -1;

  //alias for convenience
  Room *rmi;
  
  //for each room...
  int i=0; 
  for(; i<ROOM_CNT; i++)
  {
    rmi = (rm+i);
    
    //each room to have between 3 and 6 outside connections
    cx_count = get_rand(3, 6);

    //connect the room to itself
    rm->cxs = rm->cxs | powi(2, rm->rm_number);

    //...given a random number of connections, decide... 
    int c=0; 
    for(; c<cx_count; c++)
    {
      do
      {
        //...which rooms to connect
        rm_cx = get_rand(0, ROOM_CNT-1);
      }
      //keep trying if we generate a room number equivalent to our own
      while(rm_cx == (rm+i)->rm_number);

      //make the two-way connection 
      rmi->cxs = rmi->cxs | powi(2, rm_cx);
      (rm+rm_cx)->cxs = (rm+rm_cx)->cxs | powi(2, i);
    }
    
    if(DEBUG)
      printf("cxs for room %i: %c", i, rm->cxs);
  }
}


Room* gen_rooms(int count, char *names[])
{
  Room *rooms = malloc(count*sizeof(Room));
  
  int type = -1;

  int i = 0;
  for(; i < count; i++)
  {
    //rm_number allows us to do array index and room connection bit mask tricks 
    rooms[i].rm_number = i;

    /* 
     * pointless in randomizing room type assignment, since what defines a room in the user's
     * eyes is its name and connections 
     */
    if(i == 0)
      type = START_ROOM;
    else if (i == 1)
      type = END_ROOM;
    else
      type = MID_ROOM;

    rooms[i].rm_type = type;

    //set the connections bitmask
    rooms[i].cxs = '\x00';

    get_rm_name(names, &rooms[i]);       
    
  }

  return rooms;
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
  Room *rooms = gen_rooms(ROOM_CNT, ten_rooms);  

  connect_rooms( rooms ); 
  
  free(rooms);
  rooms = NULL;
  return 0;
}

