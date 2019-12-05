#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "srvr_common.h"
#include "protocol.h"


/*int set_socket_to(int cxfd, int milsecs)
{
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 1000*milsecs;
  return setsockopt(cxfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
}*/


void send_ready(int cxfd)
{
  int sent = INT_MIN;
  sent = send(cxfd, RDY_STR, SRVR_RESP_LEN, 0);
  if(DEBUG){fprintf(stderr,"otp_?_d: in send_ready; sent== %i\n", sent);}
}


void send_error(int cxfd, char *msg, int msglen)
{
  send(cxfd, msg, msglen, 0);
}

/*
 * receive client_id byte from client and verify
 * respond with ready if valid
 */
int check_client(int cxfd, int client_type)
{
  int res = INT_MIN;
  char client_type_id[PREAMB_LEN] = {'\0'};
  int recvd = INT_MIN; 
  if((recvd = recv(cxfd, client_type_id, PREAMB_LEN, 0)) < PREAMB_LEN)  
  {
    if(DEBUG){fprintf(stderr,"otp_%i_d: in check_client; recvd: %i\n", client_type, recvd);}
    perror("Failed to obtain preamble from client.");
    exit(1);
  }
  fprintf(stderr, "otp_%i_d: client type id == %s\n", client_type, client_type_id);
  if(client_type == ENCC)
  {
    if(strstr(client_type_id, "ENCC"))
    {
      res = 1; 
      if(DEBUG){fprintf(stderr,"otp_%i_d: client identified as %s\n", client_type, OTP_ENC_ID);}
      send_ready(cxfd);
    }
    else
    {
      perror("Fatal: client is not otp_enc.");
      exit(2);
    }
  }
  else if(client_type == DECC)
  {
    if(strstr(client_type_id, "DECC"))
    {
      res = 1; 
      send_ready(cxfd);
    }
    else
    {
      perror("Fatal: client is not otp_dec.");
      exit(2);
    }
  }
  return res; 
}


int get_file_len(char *preamb, int plen)
{
  char c = ' ';
  char len[plen];
  memset(len, '\0', sizeof(char)*plen);

  for(int i = 0; i < plen; i++)
  {
    c = *(preamb+i); 
    if(isdigit(c) != 0)
      len[i] = c;
  }
  int intlen = atoi(len); 
  if(DEBUG){fprintf(stderr, "otp_?_d: in get_file_len; length == %i\n", intlen);}
  return intlen;
}


/*
 * get a file
 */
int get_clients_file(int cxfd, char **buff)
{
  if(DEBUG){fprintf(stderr, "%s\n", "otp_?_d: in get file");}
  
  int filelen, n;
  filelen = n = INT_MIN;
  char preamble[PREAMB_LEN] = {'\0'};
 
  int recvd = INT_MIN; 
  if((recvd = recv(cxfd, preamble, PREAMB_LEN, MSG_WAITALL)) < 1)  
  {
    fprintf(stderr, "%s\n", "Failed to obtain valid preamble from client. Exiting.");
    if(DEBUG){fprintf(stderr,"otp_?_d: recvd==%i\n", recvd);}
    if(DEBUG){fprintf(stderr,"otp_?_d: preamble==%s\n", preamble);}
    exit(1);
  }
  
  filelen = get_file_len(preamble, PREAMB_LEN);
  if(filelen <= 0 )
  {
    fprintf(stderr, "%s", "File length not given in preamble");
    exit(1);
  }

  //ready the buffer and notify the client
  *buff = calloc(filelen, sizeof(char));
  if(buff) 
    send_ready(cxfd); 
  else
  {
    fprintf(stderr, "%s", "Fatal: buffer allocation failed on server. Exiting\n");
    exit(1);
  }
  
  recvd = n = 0;
  //TODO Will this work without a loop for larger files? Should we set socket timeout w/ MSG_WAITALL
  /*while(filelen != recvd)
    {
      while((n = recv(cxfd, buff, filelen, 0)) > 0)
      recvd += n;
  */
  fprintf(stderr, "s%\n", "HERE"); 
  n = recv(cxfd, buff, filelen, MSG_WAITALL);

  if(DEBUG){fprintf(stderr, "otp_?_d: get_file; recvd and filelen: %i and %i\n", recvd, filelen);}
  if(DEBUG){fprintf(stderr, "otp_?_d: get_file; buffer: %s\n", *buff);}
  if(n != filelen)
  {
    send_error(cxfd, INC_STR, SRVR_RESP_LEN);
    recvd = 0;
  }
  if(errno == EAGAIN || errno == EWOULDBLOCK)
    send_error(cxfd, SKTTO_STR, SRVR_RESP_LEN);
  //}
  send_ready(cxfd);
  return filelen;
}


