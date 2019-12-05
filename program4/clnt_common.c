#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/socket.h>
#include "protocol.h"


char* recv_enc_bytes(int socket, int buffsz)
{
  char *buff = calloc(buffsz, sizeof(char));
  int recvd = INT_MIN;
  recvd = recv(socket, buff, buffsz, MSG_WAITALL);
  if(recvd != buffsz)
    fprintf(stderr, "Error: received unanticipated number of cyphtertext bytes (%i)\n", recvd);
  return buff;
}


/*
 * ensure that file is valid byte-for-byte
 * out:   file size in chars
 */
int verify_file(char *fn)
{
  int nlflag, count;
  FILE * f = NULL;
  if(( f = fopen(fn, "r") ))
  {  
    char c = '\0'; 
    //make sure newline doesn't come before EOF-1
    nlflag = count = 0; 
    while((c = fgetc(f)) != EOF)
    {
      if(c == '\n')
      {
        if(nlflag == 0)
        {  
          nlflag = 1;
          count += 1;
        }
        else
        {
          fprintf(stderr, "Fatal: file %s invalid; newline not last char. Exiting.\n", fn);
          fclose(f); 
        }
      }
      else if(c > 90 || (c < 65 && c != 32))
      {
        fprintf(stderr, "Fatal: file %s invalid; found %c. Exiting.\n", fn, c);
        fclose(f); 
      }
      else
        count += 1;
    }
  }
  else
  {
    perror("Failed to open file");
    exit(1);
  }
  return count;
}


int send_file_size(int sock, int bytesz)
{
  //all of this is in order to send something like "37@@\0" to the server
  int sentx = INT_MIN;
  char numstr[PROG4_MAX_FSZ+3] = {'\0'};
  sprintf(numstr, "%i", bytesz);
  strcat(numstr, "@@"); 
  sentx = send(sock, numstr, PROG4_MAX_FSZ+3, 0);
  if(DEBUG){fprintf(stderr, "clnt_common: send_file preamble == %s; size == %i\n", numstr, sentx);}
  return sentx;
}


int send_file(int sock, char *fname, int bytesz)
{
  if(DEBUG){fprintf(stderr, "clnt_common: send_file fname  == %s\n", fname);}
  int readn, sentn, fd;
  readn = fd = INT_MIN;
  sentn = 0;

  fd = open(fname, O_RDONLY);
  char *readbuff[bytesz];
  memset(readbuff, '\0', bytesz);

  while((readn = read(fd, readbuff, bytesz)) > 0)
  {
    sentn += send(sock, readbuff, readn, 0);
  }
  if(DEBUG){fprintf(stderr, "clnt_common: send_file sentn  == %i\n", sentn);}

  if(sentn != bytesz)
    fprintf(stderr, "Bytes sent (%i) do not match predetermined file size.\n", sentn);
  
  return sentn;
}

