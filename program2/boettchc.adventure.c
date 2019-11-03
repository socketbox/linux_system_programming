#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <regex.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>

#ifndef DEBUG
#define DEBUG 0
#endif

//directory and file prefix and suffix, respectively
#define DIR_PREFIX      "boettchc.rooms.\0"
#define FILE_SUFFIX     "_room\0"
#define REL_PATH_MAX    64 
//No room file will have a line longer than 128 chars
#define RMF_LINE_MAX    128 
//Room name length MAX with null byte
#define RM_NAME_MAX     7 
//Room count
#define RM_CNT          7
//our hash modulus
#define HASH_MOD        22
//our hash coefficient
#define HASH_COEF       1
//regex for room name
#define RM_NAME_LINE    "ROOM NAME:\0"
#define RM_CONN_LINE    "CONNECTION\0"
#define RM_TYPE_LINE    "ROOM TYPE\0"
//define room types
#define RM_TYPE_START   0
#define RM_TYPE_MID     1
#define RM_TYPE_END     2
#define END_RM_STR      "END"
#define MID_RM_STR      "MID"
#define START_RM_STR    "START"
//maximum length of room type string (END, MID, START) with null
#define RM_POS_MAX      6
//maximum length of room type string (END, MID, START) with null
#define ADV_ROUTE_MAX   128 
//good size for input buffer
#define BUFF_SZ         32 
/*
 * Strings for display to the user
 */
#define CURR_LOC        "CURRENT LOCATION: "
#define POSS_CONN       "POSSIBLE CONNECTIONS: "
#define WHERE           "WHERE TO? > "
#define HUH             "HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN."
#define END_RM          "YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!"
#define END_STEPS       "YOU TOOK %i STEPS. YOUR PATH TO VICTORY WAS:\n"
//Time-related functionality
#define TIME            "time"
#define TIME_FILE       "currentTime.txt"
#define TBUFF_SZ        64


typedef struct node
{
  int   cx_cnt; 
  char  cxs[RM_CNT-1][RM_NAME_MAX];
  char  rm_name[RM_NAME_MAX];
  int   rm_type;
} Node;


typedef struct user_input
{
  int   rm_hash;
  int   cmd;
  char  *strval;
} Usrin;


/* pre:   a directory of nodes has been created using the buildnodes program 
 * in:    a char pointer to hold the value of a directory name (basename, not path)
 * out:   n/a
 * post:  the dir_name argument has a meaningful value
 * note: this code was copied nearly wholesale from https://oregonstate.instructure.com/courses/1738958/pages/2-dot-4-manipulating-directories
 */
void get_newest_dir(char *dir_name)
{
  // Modified timestamp of newest subdir examined
  int newest_dir_time = -1; 
  
  // Holds the directory we're starting in
  DIR *cwd;
  
  // Holds the current subdir of the starting dir
  struct dirent *file_in_dir;
  
  // Holds information we've gained about subdir
  struct stat dir_attributes;
  memset(&dir_attributes, 0, sizeof(struct stat));

  // Open up the directory this program was run in
  cwd = opendir("."); 

  // Make sure the current directory could be opened
  if (cwd > 0) 
  {
    // Check each entry in dir
    while ((file_in_dir = readdir(cwd)) != NULL) 
    {
      if (strstr(file_in_dir->d_name, DIR_PREFIX) != NULL) // If entry has prefix
      {
        if(DEBUG)
          fprintf(stderr, "Found the prefix: %s\n", file_in_dir->d_name);
        stat(file_in_dir->d_name, &dir_attributes); // Get attributes of the entry

        if ((int)dir_attributes.st_mtime > newest_dir_time) // If this time is bigger
        {
          newest_dir_time = (int)dir_attributes.st_mtime;
          memset(dir_name, '\0', sizeof(char)*strlen(dir_name));
          strcpy(dir_name, file_in_dir->d_name);
          if(DEBUG)
            printf("Newer subdir: %s, new time: %d\n",
                 file_in_dir->d_name, newest_dir_time);
        }
      }
    }
  }

  closedir(cwd); // Close the directory we opened
  
  file_in_dir = NULL;

  if(DEBUG)
    fprintf(stderr, "Newest entry found is: %s\n", dir_name);
}


 
int match_line(const char *patt, char *line, int buffsz, char *buff)
{
  //return value
  int retval = 1;
  int match_cnt = 2;                                            
    
  //setp for compilation
  regex_t compreg;                                               
  memset(&compreg, 0, sizeof(regex_t));                          
                       
  //setup for matching
  regmatch_t matches[match_cnt];                                 
  memset(&matches, 0, match_cnt*sizeof(regmatch_t));           
                     
  //buffer for regcomp/regexec error info
  char *errbuff = 0;                                                 
                                                                 
  if(DEBUG)
    fprintf(stderr, "Trying to match with extended regex...\n");                  
 
  int compret = -2;                                                        
  //compile with REG_EXTENDED flag                                              
  if((compret = regcomp(&compreg, patt, REG_EXTENDED)) == 0)            
  {                                                                    
    if(DEBUG)
      fprintf(stderr, "Compiling successful.\n");                                 
    int execret = -2;                                                  
    if((execret = regexec(&compreg, line, match_cnt, matches, 0)) == 0)  
    {                                                                  
      strncpy(buff, line+matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
      if(DEBUG)
        fprintf(stderr, "Captured substring: %s\n", buff);
      
      //make sure buffer is null terminated
      buff[buffsz - 1]='\0';
    }
    else if(execret != 0)                                              
    {                                                                  
      printf("Regexec failed.\n");                                     
      size_t errbuffsz = regerror(execret, &compreg, 0, 0);            
      errbuff = malloc(errbuffsz);                                     
      memset(errbuff, '\0', errbuffsz);                                
      regerror(execret, &compreg, errbuff, errbuffsz);                 
      fprintf(stderr, "Regexec error: %s\n", errbuff);                 
      retval = errno;
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
    retval = errno;
  } 

  if(errbuff)
  {
    free(errbuff);                                                       
    errbuff = NULL;          
  }
  regfree(&compreg);                                                   
  return retval;                                                            
}

  
/*
 * pre:   modulo selected for perfect hash (see notes)
 * in:    room name as c-string
 * out:   an integer hash that can be used as an array index
 * post:  n/a
 *
 * notes: I went down a rabbit hole regarding hashes. Perfect hashing using the FKS algorithm of two
 * levels of hash tables was too complex for the amount of time I had (and the size of the
 * dictionary) so I modified [the code here]
 * (https://raw.githubusercontent.com/reneargento/algorithms-sedgewick-wayne/master/src/chapter3/section4/Exercise4.java) 
 * to arrive at a modulus (table size) and coefficient that met the needs of the program, and
 * defined that modulus as a preprocessor directive for clarity.
 */
int get_room_hash(const char *name, int mod, int coeff)
{
  if(DEBUG)
    fprintf(stderr, "Getting hash for room %s\n", name);

  int charsum = 0;
  for(; *name != '\0'; name++)
  {
    charsum += *name;
  }
  int hash = (coeff * charsum) % mod;
  if(DEBUG)
    fprintf(stderr, "Sum: %i, Hash: %i\n", (int)charsum, hash);
  return hash;
}


/* convenience method for retrieving a hash that uses the global default */
int get_hash(char *name)
{
  return get_room_hash(name, HASH_MOD, HASH_COEF); 
}


/* pre:   f is a file that's been opened
 * in:    a FILE stream and an array of Node structs
 * out:   an integer hash of the starting node, or -1, to indicate that finding the starting node
 *        failed
 * post:  on success, a Node struct occupies an element in the rm array is n/a
 */
  int parse_file(FILE *rmf, Node nodearr[])
  {
    /* Each type of line has a regex, starting with a Room's name */
    const char *type_patt = "^ROOM[[:space:]]{1}TYPE:[[:space:]]{1}([[:alpha:]]{3,5})";
    const char *conn_patt = "^CONNECTION[[:space:]]{1}[0-9]{1}:[[:space:]]{1}([[:alpha:]]{6})";
    const char *rm_name_patt = "^ROOM[[:space:]]{1}NAME:[[:space:]]{1}([[:alpha:]]{6})";

    //node to be populated from file
    Node n;
    memset(&n, 0, sizeof(Node));

    //the line we'll read in
    char line[RMF_LINE_MAX];
    memset(line, '\0', sizeof(char)*RMF_LINE_MAX);

    /* the name buffer that will boostrap us, getting us the node we want to populate from the
     * already allocated Node array. less than optimal.*/
    char namebuff[RM_NAME_MAX];
    memset(namebuff, '\0', RM_NAME_MAX);
    
    //yes, this is a kludge due to poor planning; need to get hash of starting node back to main()
    int retval = -1; 
    int hash_idx = -1; 
    while( fgets( line, RMF_LINE_MAX, rmf) )
    {    
      if(DEBUG)
      {
        fprintf(stderr, "In fgets loop for; line:  %s\n", line);
        fflush(stdout);
      }
      //get the room name
      if( strstr(line, RM_NAME_LINE) )
      {
        
        if(match_line(rm_name_patt, line, RM_NAME_MAX, namebuff) > 0)
        { 
          //use the hash of the room name to insert the node's pointer into the array
          hash_idx = get_room_hash(namebuff, HASH_MOD, HASH_COEF);
          strcpy(nodearr[hash_idx].rm_name, namebuff);
        }
        else
        {
          fprintf(stderr, "Failed to match name line.\n");
        }
      }
      else if( strstr( line, RM_CONN_LINE ) )
      {
        /* the last arg here is indexing into the nodearr with the hash, getting the connection
         * array, then indexing into that array with the node's connection count */
        if(match_line(conn_patt, line, RM_NAME_MAX, nodearr[hash_idx].cxs[nodearr[hash_idx].cx_cnt]))
          nodearr[hash_idx].cx_cnt++;
        else
        {
          fprintf(stderr, "Failed to match connection line.\n");
        }
      }
      else if( strstr( line, RM_TYPE_LINE ) )
      {
        char rmt[RM_POS_MAX];
        memset(rmt, '\0', RM_POS_MAX); 
        if(match_line(type_patt, line, RM_POS_MAX, rmt))
        {
          if(DEBUG)
            fprintf(stderr, "Evaluating type before insertion in node.\n");
         
          //take the string and convert it to an integer
          if( strcmp(rmt, END_RM_STR) == 0) 
            nodearr[hash_idx].rm_type = RM_TYPE_END;
          else if( strcmp(rmt, MID_RM_STR) == 0)
            nodearr[hash_idx].rm_type = RM_TYPE_MID;
          else if(strcmp(rmt, START_RM_STR) == 0)
            nodearr[hash_idx].rm_type = RM_TYPE_START;
              
          if(nodearr[hash_idx].rm_type == RM_TYPE_START)
          {
            retval = hash_idx;
            if(DEBUG)
              fprintf(stderr, "Starting hash is %i for room %s.\n", hash_idx, nodearr[hash_idx].rm_name);
          }
        }  
        else
        {
          fprintf(stderr, "Failed to match type line.\n");
        }
      }
  }
  if(DEBUG)
    fprintf(stderr, "Retval from parse_file is %i for room %s.\n", retval, nodearr[hash_idx].rm_name);
  return retval;
}


/* pre:   a directory of room files has been opened
 * in:    a directory name relative to the cwd of the program and an array of nodes
 * out:   the hash of the starting node
 * post:  the node array is sparsely populated
 */
int process_room_files(const char * const dirname, Node nodes[])
{
  /* if I don't make this deep copy and use it to form the file path 
   * the dirname argument's value is emptied right before opening
   * the seventh file of the directory and the program crashes
   * */
  char *dircpy;
  dircpy = malloc(strlen(dirname)*sizeof(char));
  strcpy(dircpy, dirname);
  //DIR *the_dir = opendir(dircpy);
  DIR *the_dir = opendir(dirname);

  //result of parsing the file. if it's the start node, it will be >= 0
  int pfres = -2;
  //the hash of the starting node
  int start_hash = -2;
  if(the_dir)
  {
    struct dirent *rmf;
    FILE *f;
    char *abspath;
    abspath = malloc(PATH_MAX*sizeof(char));
    char rpath[REL_PATH_MAX];
    while((rmf = readdir(the_dir)) != NULL) 
    {
      if(strstr(rmf->d_name, FILE_SUFFIX) != NULL)
      {
        if(DEBUG)
        {
          fprintf(stderr, "Found room file %s\n", rmf->d_name);
          fprintf(stderr, "Dirname: %s\n", dirname);
          fflush(stdout);
        }
        memset(rpath, '\0', REL_PATH_MAX); 
        strcat(rpath, dircpy); 
        //this will bring the program to a fiery stop: 
        //strcat(rpath, dirname); 
        strcat(rpath, "/"); 
        strcat(rpath, rmf->d_name); 
        abspath = realpath(rpath, abspath); 
        if(abspath)
          f = fopen(abspath, "r");
        if( f )
        {
          if(DEBUG)
          {
            fprintf(stderr, "Opened room file %s for parsing.\n", rmf->d_name);
            fflush(stdout);
          }
          //call to parse function that generates "graph"
          if( (pfres = parse_file(f, nodes)) > -1)
              start_hash = pfres; 
          if(DEBUG)
          {
            fprintf(stderr, "In process_room_files...\nRoom %s has start_hash of %i\n", rmf->d_name, start_hash);
          }
          if(start_hash == -2 && pfres == -2) 
          {
            fprintf(stderr, "Failed to parse file: %s; Error: %s\n", rmf->d_name, strerror(errno));
            fclose(f);
            exit(errno);
          }
          fclose(f);
        }
        else
        {
          fprintf(stderr, "Failed to open room file %s. Error: %s\n", rmf->d_name, strerror(errno));
        }
        //clear the abspath 
        memset(abspath, '\0', PATH_MAX*sizeof(char));
      }
    }
    free(abspath);
    if(errno > 0)
      fprintf(stderr, "Exiting from directory stream with error code: %s\n", strerror(errno));
  }
  closedir(the_dir);
  return start_hash;
}


Node* get_node_for_hash(int hash, Node nodes[])
{
  Node *n;
  if(hash <= HASH_MOD)
  {
    n = &(nodes[hash]);
  }
  else
  {
    fprintf(stderr, "FATAL: Hash larger than table. Exiting.\n");
    exit(-1);
  }
  return n;
}


void end(Node nodes[], int hash_route[], int steps)
{
  fprintf(stdout, "\n%s\n", END_RM);
  fprintf(stdout, END_STEPS, steps);
  
  //tmp var for node
  Node *n = NULL;
  int s; 

  //end path list starts from room resulting from first step, not START
  for(s = 1; s <= steps; s++)
  {
    n = get_node_for_hash(hash_route[s], nodes);
    fprintf(stdout, "%s\n", n->rm_name);
  }
  exit(0);
}


/*
 * I'm throwing everything into this function; proabably a sign of poor design.
 *
 * pre:   user input has been gathered and the node graph has taken its ultimate form 
 * in:    all the things
 * out:   n/a
 * post:  the user is either routed to the end node, or falls out of the switch
 *        statement and we display the node per usual
 */
void route_user(Usrin *u, Node **current, Node nodes[], int hash_route[], int *steps)
{
  //the node the user just selected
  Node *ntmp = get_node_for_hash(u->rm_hash, nodes);
  *current = ntmp;

  //when is pass by value/reference going to sink in?
  if(DEBUG) fprintf(stderr, "In route user 1. Steps=%i\n", *steps);

  hash_route[++(*steps)] = u->rm_hash;
  
  if(DEBUG) fprintf(stderr, "In route user 2. Steps=%i\n", *steps);
  if(ntmp->rm_type == 2)
  {
    end(nodes, hash_route, *steps);
  }
  return;
}


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


void display_node(Node *n)
{
  //at start of game, do not precede display of node with newline
  if(n->rm_type == 0)
    fprintf(stdout, "%s", CURR_LOC);
  else
    fprintf(stdout, "\n%s", CURR_LOC);
  
  fprintf(stdout, "%s\n", n->rm_name);

  fprintf(stdout, POSS_CONN);
  //randomize display of connections and track those already selected
  int slctd[6] = {0};
  int x, y = 0;
  for(x=0; x < n->cx_cnt; x++)
  { 
    do
    {
      y = get_rand(0, n->cx_cnt-1);
    }
    while( slctd[y] == 1 );
    slctd[y] = 1;
    
    //check to see if we need a comma
    if(x != (n->cx_cnt)-1)
      fprintf(stdout, "%s, ", n->cxs[y]); 
    else
      fprintf(stdout, "%s.\n", n->cxs[y]); 
  }

  //where to prompt
  fprintf(stdout, WHERE);
 
  fflush(stdout);
}


void parse_user_input(Usrin *u, Node *n)
{
  //get a set number of chars from stdin and reduce them to a trimmed string
  char buff[BUFF_SZ] = {'\0'};
  fgets(buff, BUFF_SZ, stdin);
  //guard against especially long lines and leaving the newline in the stdin buffer
  //https://stackoverflow.com/a/45470329/148680 
  if (strchr(buff, '\n') == NULL) 
  {
    int ch;
    while ((ch = fgetc(stdin)) != '\n' && ch != EOF);
  }
  
  char parsedin[BUFF_SZ] = {'\0'};
  sscanf(buff, "%6s", parsedin);
  
  if(DEBUG)
    fprintf(stderr, "Parsed input: %s\n", parsedin);
  
  int match = 0;
  //does the input match "time"
  if(strcmp(TIME, parsedin)==0)
  {
    u->cmd = 't';
    //trying a little less interruptive DEBUG clause
    if(DEBUG){ fprintf(stderr, "Matched input (%s) to 'time'\n", parsedin); }
    match = 1;
  }
  if(!match)
  {
    //check for room name in input
    int c;
    for(c=0; c < n->cx_cnt && !match; c++)
    {
      if( (strcmp(n->cxs[c], parsedin) == 0) )
      {
        u->cmd = 'n';
        if(DEBUG){ fprintf(stderr, "Matched room name to userinput %s\n", parsedin); }
        u->rm_hash = get_hash(n->cxs[c]); 
        match = 1;
      }
    }
  }
  if(!match)
  {
    if(DEBUG){ fprintf(stderr, "No match found for %s ...Expressing confusion.\n", parsedin); }
    u->cmd = 'h';
  }
  return;
}


void print_nodes(Node nodes[])
{
  int i, c;
  Node n; 
  for(i=0; i<HASH_MOD; i++)
  {
    n=nodes[i];
    if( n.rm_name[0] != '\0')
    {
      fprintf(stderr, "Node/Room: \t\t%s\n", n.rm_name);
      fprintf(stderr, "Type:\t\t\t%i\n", n.rm_type);
      fprintf(stderr, "Connections:\t%i\n", n.cx_cnt);
      for(c=0; c<n.cx_cnt; c++)
        fprintf(stderr, "Connection %i: %s\n", c, n.cxs[c]);
    }
  }
}


void* write_time(void *mutex)
{
  if(DEBUG){fprintf(stderr, "time: Locking mutex\n"); }
  //lock the mutex
  int res = -1;
  if( (res = pthread_mutex_trylock(mutex)) == 0)
  {
    FILE* f = fopen(TIME_FILE, "w");
    if(f)
    {
      //get time
      struct tm now;
      //now = malloc(sizeof(struct tm));
      memset(&now, 0, sizeof(struct tm));
      time_t nownow = time(NULL); 
      now = *(localtime( &nownow ));
      if(DEBUG)
        fprintf(stderr, "Got time. Attempting to write it.\n");
      char tbuff[TBUFF_SZ] = {'\0'};
      strftime(tbuff, TBUFF_SZ, "%I:%M%p, %A, %B %e, %Y", &now); 
      if(DEBUG)
        fprintf(stderr, "Writing time: %s to file.\n", tbuff);
      fprintf(f, "%s", tbuff); 
      fclose(f);
    }
    else
      fprintf(stderr, "Failed to open file for write_time(). Error: %s\n", strerror(errno));
  }
  else
  {
    fprintf(stderr, "Lock failed. Error: %i", res);
  }
  pthread_mutex_unlock(mutex);
}    
  

int show_time(pthread_mutex_t *tf_mutex)
{
  /*
   *  1. create the second thread and 
   *    a. give it a routine that
   *      i. writes time to a file, creating or overwriting it if necessary 
   *    b. pass it a reference to the unlocked mutex
   *  
   *  2. call pthread_join and lock on the mutex 
   *
   *  3. read the file and print it to the screen 
   *
   *  4. confirm that the first thread has been destroyed and then create
   *     a new thread
   */
  if(DEBUG){fprintf(stderr, "main: Unlocking mutex...\n"); }
  if(pthread_mutex_unlock(tf_mutex) != 0)
    fprintf(stderr, "Error unlocking mutex: %s\n", strerror(errno));
  pthread_t time_thread;
  pthread_create(&time_thread, NULL, *write_time, tf_mutex);
  //stop main thread from moving while time thread is messing with I/O
  pthread_join(time_thread, NULL);
  pthread_mutex_lock(tf_mutex);
  
  //read the file created by thread routine write_time()
  char tbuff[TBUFF_SZ] = {'\0'};
  FILE* f;
  if(f = fopen(TIME_FILE, "r"))
  {
    if(fgets(tbuff, TBUFF_SZ, f))
    {
      fprintf(stdout, "\n\t%s\n", tbuff);
      fprintf(stdout, "\n%s", WHERE);
    }
    else
      fprintf(stderr, "Failed to read time from file. Error: %i\n", errno);
  } 
}



int main()
{
  //mutex for time functionality
  pthread_mutex_t tf_mutex = PTHREAD_MUTEX_INITIALIZER;
  
  char dir_of_interest[PATH_MAX];
  memset(dir_of_interest, '\0', PATH_MAX*sizeof(char));
  get_newest_dir(dir_of_interest);
  
  if(DEBUG)
    fprintf(stderr, "Newest dir: %s\n", dir_of_interest);

  //the array that forms the basis for the graph of rooms
  Node nodes[HASH_MOD];
  memset(nodes, 0, HASH_MOD*sizeof(Node));
  
  /* if I were going to refactor this, I'd get an array of absolute paths
   * and pass each path to a file parser, rather than call the parser from the 
   * function that iterates over the diretory */

  //start is the hash of the first room
  int start = -1;
  start = process_room_files(dir_of_interest, nodes);

  if(DEBUG)
  {
    fprintf(stderr, "Starting Hash: %i\n", start);
    print_nodes(nodes);
  }

  //the current node
  Node *curr_node;
  curr_node = get_node_for_hash(start, nodes);

  Usrin u;
  int steps = 0; 
  // should this be dynamic and be adjusted with realloc?
  int route[ADV_ROUTE_MAX] = {0};
  //put starting node in route array
  route[0]=start;

  do 
  {
    //accomodates display requirements for prompt after time command 
    if(u.cmd != 't')
    {
      memset(&u, 0, sizeof(Usrin));
      display_node(curr_node);
    }
    parse_user_input(&u, curr_node); 
    switch(u.cmd)
    {
      case 't':
        show_time(&tf_mutex);
        break;
      case 'n':
        route_user(&u, &curr_node, nodes, route, &steps);
        break;
      case 'h':
        fprintf(stdout, "\n%s\n", HUH);
        fflush(stdout);
        break; 
    }
  }
  while(u.cmd != 'x');
  
  return 0;
}
