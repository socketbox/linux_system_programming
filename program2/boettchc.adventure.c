#define _XOPEN_SOURCE 500
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <ftw.h>
#include <sys/stat.h>
#include <linux/limits.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define DIR_PREFIX    "boettchc.rooms."


typedef struct room
{
  int cxs[7];
  char* rm_name;
} Room;


/* pre:   a directory of rooms has been created using the buildrooms program 
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


int parse_file(char *path, const struct stat *ss, int type, struct FTW *ftwbuf)
{
  if(DEBUG)
    fprintf(stderr, "In parse_file with args %s, %p, %i, %p\n", path, ss, type, ftwbuf);
  return 0;
}
      

void read_room_files(char *dirname, Room rm[])
{
  int res = nftw(dirname, parse_file, 0, FTW_MOUNT | FTW_PHYS);
  if(res != 0)
    fprintf(stderr, "Failed to read room files. %s", strerror(errno));
}


int main()
{
  char dir_of_interest[PATH_MAX];
  get_newest_dir(dir_of_interest);
  if(DEBUG)
    fprintf(stderr, "Newest dir: %s", dir_of_interest);

  Room rooms[7];
  read_room_files(dir_of_interest, rooms);
  return 0;
}
