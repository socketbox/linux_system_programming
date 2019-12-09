#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include "protocol.h"
#include "base.h"


/**
 * pre:     connection established 
 * in:      connected socket and client type
 * out:     n/a 
 * post:    server sent client type identifier 
 */
void send_client_type(int cxfd, int type)
{
  if(DEBUG){fprintf(stderr, "%s\n", "otp_enc: sending client type");}
  
  int sent = INT_MIN; 
  if(type == ENCC)
    sent = send(cxfd, ENCC_PAMB_STR, PREAMB_LEN, 0);
  else if(type == DECC)
    sent = send(cxfd, DECC_PAMB_STR, PREAMB_LEN, 0);

  if(DEBUG){fprintf(stderr, "otp_enc: bytes sent==%i\n", sent);}
}


/**
 * pre:     connection established 
 * in:      connected socket 
 * out:     n/a 
 * post:    n/a
 * nb:      all or nothing bet on receiving proper response from server
 */
void parse_response(int cxfd)
{
  int recvd = INT_MIN;
  char resp[SRVR_RESP_LEN] = {'\0'};
  recvd = recv(cxfd, resp, SRVR_RESP_LEN, MSG_WAITALL);
  if(DEBUG){fprintf(stderr, "Received: %i in response\n", recvd); }
  
  if(recvd > 0) 
  {
    if(strcmp(resp, INVCL_STR) == 0)
    {
      fprintf(stderr, "%s", "Fatal: attempted connection to wrong server type. Exiting.\n"); 
      exit(2);
    }
    else if(strcmp(resp, RDY_STR) != 0)
    {
      fprintf(stderr, "Fatal: server did not send READY: code %s. Exiting.\n", resp); 
      exit(1);
    }
  }
  else
  {
    fprintf(stderr, "Fatal: received no response from server. Exiting.\n");
    exit(1);
  }
}


/**
 * pre:     connection established   
 * in:      connected socket, receiving buffer size 
 * out:     pointer to received text 
 * post:    n/a 
 */
char* recv_bytes(int socket, int buffsz)
{
  char *buff = calloc(buffsz, sizeof(char));
  int recvd = INT_MIN;
  recvd = recv(socket, buff, buffsz, MSG_WAITALL);
  if(recvd != buffsz)
    fprintf(stderr, "Error: received unanticipated number of cyphtertext bytes (%i)\n", recvd);
  return buff;
}


/**
 * pre:   n/a
 * in:    c-string filename 
 * out:   file size in chars
 * post:  file is validated
 * nb:    ensure that file is valid byte-for-byte
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
      //check for anything neither ASCII uppercase alpha nor space
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



/**
 * pre:     established connection 
 * in:      connected socket, file size to transmit
 * out:     number of sent bytes, as reported by send()
 * post:    server aware of size to anticipate
 * nb:      sending size allows us to rely heavily upon MSG_WAITALL sockopt
 */
int send_file_size(int sock, int bytesz)
{
  //all of this is in order to send something like "37@@\0" to the server
  int sentx = INT_MIN;
  char numstr[PROG4_MAX_FSZ+3] = {'\0'};
  //convert bytesz int to string  
  sprintf(numstr, "%i", bytesz);
  strcat(numstr, "@@"); 
  sentx = send(sock, numstr, PROG4_MAX_FSZ+3, 0);
  if(DEBUG){fprintf(stderr, "clnt_common: send_file preamble == %s; size == %i\n", numstr, sentx);}
  return sentx;
}



/**
 * pre:     connection established 
 * in:      connected socket, filename, file size 
 * out:     sent bytes as reported by send() 
 * post:    server receives file (or not) 
 * nb:      generic file sending function that handles large files
 */
int send_file(int sock, char *fname, int bytesz)
{
  if(DEBUG){fprintf(stderr, "clnt_common: send_file fname  == %s\n", fname);}
  int readn, sentn, fd;
  readn = fd = INT_MIN;
  sentn = 0;

  fd = open(fname, O_RDONLY);
  char *readbuff[bytesz];
  memset(readbuff, '\0', bytesz);

  //Kerrisk p1260
  while((readn = read(fd, readbuff, bytesz)) > 0)
  {
    sentn += send(sock, readbuff, readn, 0);
  }
  if(DEBUG){fprintf(stderr, "clnt_common: send_file sentn  == %i\n", sentn);}

  if(sentn != bytesz)
    fprintf(stderr, "Bytes sent (%i) do not match predetermined file size.\n", sentn);
  
  return sentn;
}


