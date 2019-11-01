#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <linux/limits.h>

#ifndef DEBUG
#define DEBUG 0
#endif

//directory and file prefix and suffix, respectively
#define DIR_PREFIX      "boettchc.nodes."
#define FILE_SUFFIX     "_room"
//No room file will have a line longer than 32 chars
#define RMF_LINE_MAX    32
//Room name length MAX
#define RM_NAME_MAX     9 
//Room count
#define RM_CNT          7
//our hash modulus
#define HASH_MOD        25

typedef struct node Node;
typedef struct node
{
  //array of Node pointers for connections
  Node *conn[HASH_MOD];
  char* rm_name;
} Node;


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
  DIR* cwd;
  // Holds the current subdir of the starting dir
  struct dirent *file_in_dir;
  // Holds information we've gained about subdir
  struct stat dir_attributes;

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
          fprintf(stderr, "Found the prefex: %s\n", file_in_dir->d_name);
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

  if(DEBUG)
    fprintf(stderr, "Newest entry found is: %s\n", dir_name);
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
int get_room_hash(char *name, int mod, int coeff)
{
  int charsum = 0;
  for(; *name != '\0'; name++)
  {
    charsum += *name;
  }
  int hash = (coeff * charsum) % mod;
  if(DEBUG)
    fprintf(stderr, "Sum: %i, Hash: %i, Name: %s\n", name);
  return hash;
}


/* pre:   f is a file that's been opened
 * in:    a FILE stream and an array of Node structs
 * out:   an int indicating success (0) or failure (-1)
 * post:  on success, a Node struct occupies an element in the rm array is n/a
 */
int parse_file(FILE f, Node nodearr[])
{
  /* Each type of line has a regex, starting with a Room's name */
  const char *name_patt = "^ROOM\s{1}NAME:\s{1}([:alpha:]{1,9})";
  const char *type_patt = "^ROOM\s{1}TYPE:\s{1}([:alpha:]{3,5})_ROOM";
  const char *conn_patt = "^CONNECTION\s{1}([:alpha:]{1,9})";
 
  //the struct for the compiled regexes
  regex_t name_comp, type_comp, conn_comp;
  memset(&name_comp, '0', sizeof(regex_t));
  memset(&type_comp, '0', sizeof(regex_t));
  memset(&conn_comp, '0', sizeof(regex_t));

  regmatch_t name_match, type_match, conn_match;
  memset(&name_match, '0', sizeof(regmatch_t));
  memset(&type_match, '0', sizeof(regmatch_t));
  memset(&conn_match, '0', sizeof(regmatch_t));


  //the result of the regcomp
  int compres = -1;

  if( (compres = regcomp(&name_comp, name_patt)) == 0)
  { 
    if( regexec(&name_comp, line, -1, NULL, 0) == 0 )
      retval = 0;
    else
    {
      //create a tmp buffer
  
  char line[RMF_LINE_MAX];
  while( getline( &line, &RMF_LINE_MAX, f) > -1)
  {    
    //sscanf(f, "%d"); 
    if( 
    
    
    memset(line, '\0', sizeof(char)*32);

  }
  if(DEBUG)
    ; 
  return 0;
}
      

void read_room_files(char *dirname, Node nodes[])
{
  DIR *the_dir = opendir(dirname);
  struct dirent *rmf;
  FILE *f = NULL; 
  while((rmf = readdir(the_dir)) != NULL) 
  {
    if(strstr(rmf->d_name, FILE_SUFFIX) != NULL)
    {
      if(DEBUG)
        fprintf(stderr, "Found room file %s\n", rmf->d_name);
        f = fopen(rmf->d_name, 'r');
        if( parse_file(f, nodes) != 0)
        {
            fprintf(stderr, "Failed to parse file: %s; Error: %s\n", rmf->d_name, strerror(errno));
            fclose(f);
            exit(errno);
        }
        close(f);
    }
  }
}


int main()
{
  char dir_of_interest[PATH_MAX];
  get_newest_dir(dir_of_interest);
  if(DEBUG)
    fprintf(stderr, "Newest dir: %s", dir_of_interest);

  Node nodes[7];
  read_room_files(dir_of_interest, nodes);
  return 0;
}
