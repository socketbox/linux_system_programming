#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define ROOM_CNT        7
#define NAME_LEN        9
#define ONID            "boettchc\0"
#define ROOMS           "rooms\0"
#define PID_MAX_LEN     5


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
void mk_room_dir(pid_t pid, char **dir_name)
{
  char *delim = ".";
  /* cat /proc/sys/kernel/pid_max returns 49152 on os1 */
  int pid_str_buff_sz = PID_MAX_LEN * sizeof(char); 
  char pid_str[pid_str_buff_sz]; 
  sprintf(pid_str, "%i", pid);
  //add 3 for two delims and a null byte
  int dir_name_len = strlen(ROOMS) + strlen(ONID) + pid_str_buff_sz + 3;
  if(DEBUG)
    printf("dir_name_len: %i\n", dir_name_len);
  *dir_name = malloc(sizeof(char)*dir_name_len);
  if(!dir_name)
  {
    printf("malloc failed for dir_name. Exiting.\n");
    exit(1);
  }
  memset(*dir_name, '\0', dir_name_len);
  strcpy(*dir_name, ONID);
  strcat(*dir_name, delim);
  strcat(*dir_name, ROOMS);
  strcat(*dir_name, delim);
  strcat(*dir_name, pid_str);
  if(DEBUG)
    printf("room dir name: %s\n", *dir_name);

  int result = mkdir(*dir_name, 0755);
  if(result < 0)
  {
    fprintf(stderr, "Failed to create directory. Error: %i\nExiting.", result);
    exit(result); 
  }
}

void write_eol(int fd)
{
  write(fd, "\n", 1);
}

void write_room_file(int fd, Room *rm, int rmn)
{
  //ROOM NAME
  char *lab1 = "ROOM NAME: ";
  write(fd, lab1, strlen(lab1)*sizeof(char));
  write(fd, rm[rmn].rm_name, strlen(rm[rmn].rm_name)*sizeof(char));
  write_eol(fd);

  //CONNECTION
  //simplify references to connections bitmask
  unsigned char cxs = rm[rmn].cxs;
  char lab2[] = "CONNECTION  : \0";

  //b is iterator for bits in mask, bitval for bit comparison, and cxnum for room # label in output
  unsigned int b, bitval, cxnum;
  b = bitval = cxnum= 0;
  //ignore eigth bit (there are only seven rooms)
  for(; b < 7; b++)
  {
    //don't list room itself as a connection
    if(b != rm[rmn].rm_number)
    {
      bitval = powi(2,b);
      if(DEBUG) 
        printf("cxs: %i, bitval: %u, AND: %u\n", cxs, bitval, cxs & bitval);
      if(cxs & bitval)
      {
        if(DEBUG)
          printf("b: %i, cxnum: %i\n", b, cxnum); 
        //overwrite space before colon with number
        lab2[11] = '0'+(char)(++cxnum);
        write(fd, lab2, strlen(lab2)*sizeof(char));
        write(fd, rm[b].rm_name, NAME_LEN*sizeof(char));
        write_eol(fd); 
      }
    }
  }

  //ROOM TYPE
  char *lab3 = "ROOM TYPE: ";
  write(fd, lab3, strlen(lab3)*sizeof(char));
  char *rmt = ROOM_TYPES[rm[rmn].rm_type];
  //for the sake of consistency
  write(fd, rmt, strlen(rmt)*sizeof(char));
  write_eol(fd);
  close(fd);
}


void mk_room_files(Room *rm, char *dir)
{
  char psep = '/';
  char *suffix = "_room";
  //path length is dir name, separator, room name limit, and suffix length
  int path_len = strlen(dir) + 1 + NAME_LEN + 5;
  //allocate mem for pathname and zero it out 
  char *fn;
  //our file descriptor
  int fd = -1;
  int i = 0; 
  for(; i<ROOM_CNT; i++)
  {
    fn = malloc( path_len * sizeof(char));
    memset(fn, '\0', sizeof(char));
    //Schlemiel the Painter
    strcat( strcat( strcat( strcat(fn, dir), &psep), (rm+i)->rm_name), suffix);
    if(DEBUG)
      printf("filename: %s\n", fn);
    fd = open(fn, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IXUSR);
    if(fd >= 0)
      write_room_file(fd, rm, i);
    else
    {
      fprintf(stderr, "Failed to created room file (errno: %i). Exiting.\n", errno);
      exit(fd);
    }
    free(fn);
  }
  fn = NULL;
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
    rm->cxs = rm->cxs | (unsigned char)powi(2, rm->rm_number);

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
      rmi->cxs = rmi->cxs | (unsigned char)powi(2, rm_cx);
      (rm+rm_cx)->cxs = (rm+rm_cx)->cxs | (unsigned char)powi(2, i);
    }
    
    if(DEBUG)
      printf("cxs for room %i: %u\n", i, rm->cxs);
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
  char *ten_rooms[] = {"BritCo", "Albert", "Saskat", "Manito", "Ontari", "Quebec", "NewLab", "NeBrun", "NoScot", "PrinEd"};
 
  /* get the PID */
  pid_t pid = getpid();

  /* make room file directory */
  char *dir_name = NULL;
  mk_room_dir(pid, &dir_name);
  
  /* seed the PRNG */
  srand(time(NULL));
  
  /* make an $ONID.rooms.$PID directory */
  Room *rooms = gen_rooms(ROOM_CNT, ten_rooms);  

  connect_rooms( rooms ); 

  mk_room_files(rooms, dir_name);

  free(dir_name);
  dir_name = NULL;
  free(rooms);
  rooms = NULL;
  return 0;
}


