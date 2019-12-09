#include <sys/wait.h>
#include <signal.h>
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

//our global child process counter
int KIDCOUNT;

void set_srvr_proc_mask()
{
  sigset_t not_chld;                                      
  sigfillset(&not_chld);                                  
  //remove SIGCHLD
  sigdelset(&not_chld, SIGCHLD);                          
  sigdelset(&not_chld, SIGINT);                          
  if(sigprocmask(SIG_SETMASK, &not_chld, NULL) == -1)
      perror("Setting proc. mask for server failed");
}


/**
 * pre:     connection established 
 * in:      connected socket 
 * out:     n/a 
 * post:    client sent READY 
 * nb:      taken from Program 3 of this class
 */
void chld_hndlr(int sig, siginfo_t *sigi, void *ucontext)
{
  //Kerrisk, pp556-559
  int cpid = 0;
  int em = INT_MAX; 
  int status = INT_MAX; 
  if(DEBUG)
  {
    fprintf(stderr, "otp_?_d: chld_hndlr, before cpid eval: cpid=%i; exit status %i from %i\n", \
        cpid, sigi->si_pid, sigi->si_status);
  }
  while((cpid = waitpid(-1, &em, WNOHANG)) > 0)
  {
    if(DEBUG)
      fprintf(stderr, "otp_?_d: chld_hndlr, after waitpid: cpid=%i; exit status %i from %i\n", \
          cpid, sigi->si_pid, sigi->si_status);

    //decrement child process counter
    KIDCOUNT--;
    if(DEBUG){fprintf(stderr, "otp_?_d: decrementing KIDCOUNT for %i\n; KIDCOUNT==%i\n", cpid, KIDCOUNT);}

    if (WIFEXITED(em))
    {
      if(DEBUG){fprintf(stderr, "otp_?_d: the process exited normally\n");}
      status = WEXITSTATUS(em);
      if(DEBUG){fprintf(stderr, "otp_?_d: background pid %i is done: exit value %i\n", cpid, status);}
    }
    else if(WIFSIGNALED(em))
    {
      status = WTERMSIG(em);
      if(DEBUG){fprintf(stderr, "otp_?_d: terminated by signal %i\n", status);}
    }
  }
}

/**
 * pre:     connection established 
 * in:      connected socket 
 * out:     n/a 
 * post:    client sent READY 
 * nb:      taken from Program 3 of this class
 */
void reg_chld_handler()
{
  //create and assign handler to SIGCHLD
  struct sigaction actchld = {{0}};
  
  sigset_t hndl_chld;
  sigfillset(&hndl_chld);
  actchld.sa_mask = hndl_chld;
  
  actchld.sa_sigaction = chld_hndlr;
  actchld.sa_flags = SA_RESTART;

  if(sigaction(SIGCHLD, &actchld, NULL) == -1)
    perror("Call to sigaction failed to register SIGCHLD handler");
}


/**
 * pre:     connection established 
 * in:      connected socket 
 * out:     n/a 
 * post:    client sent READY 
 */
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


/**
 * pre:     connection established
 * in:      connected socket, type of client server expects
 * out:     result of client check; 1 if good 
 * post:    sends ready to client if type is valid
 * nb:      result isn't necessary, as invalid client type results in exit(2)
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
  if(DEBUG){fprintf(stderr, "otp_%i_d: client type id == %s\n", client_type, client_type_id);}
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
      send_error(cxfd, INVCL_STR, 6);
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
      send_error(cxfd, INVCL_STR, 6);
      exit(2);
    }
  }
  return res; 
}



/**
 * pre:     connection established
 * in:      the connection 
 * out:     the length of the file about to be transmitted 
 * post:    n/a 
 */
int get_file_len(int cxfd)
{
  int recvd = INT_MIN; 
  char preamble[PREAMB_LEN] = {'\0'};
  if((recvd = recv(cxfd, preamble, PREAMB_LEN, MSG_WAITALL)) < 1)  
  {
    if(DEBUG){fprintf(stderr, "%s\n", "Failed to obtain valid preamble from client. Exiting.");}
    if(DEBUG){fprintf(stderr,"otp_?_d: recvd==%i\n", recvd);}
    if(DEBUG){fprintf(stderr,"otp_?_d: preamble==%s\n", preamble);}
    exit(1);
  }
  char c = ' ';
  char len[PREAMB_LEN] = {'\0'};

  for(int i = 0; i < PREAMB_LEN; i++)
  {
    c = *(preamble+i); 
    if(isdigit(c) != 0)
      len[i] = c;
  }
  int intlen = atoi(len); 
  if(DEBUG){fprintf(stderr, "otp_?_d: in get_file_len; length == %i\n", intlen);}
  return intlen;
 
}


/**
 * pre:     connection established 
 * in:      socket, anticipated file length 
 * out:     char* to buffer of file contents 
 * post:    n/a 
 */
char* get_clients_file(int cxfd, int filelen)
{
  if(DEBUG){fprintf(stderr, "%s\n", "otp_?_d: in get file");}
  
  //ready the buffer and notify the client
  char *buff = calloc(filelen, sizeof(char));
  if(buff) 
    send_ready(cxfd); 
  else
  {
    fprintf(stderr, "%s", "Fatal: buffer allocation failed on server. Exiting\n");
    exit(1);
  }
  
  int recvd = 0;
  /*key here is to ascertain size of file prior to transmission and set socket
    options such that we wait for all bytes to be received*/
  recvd = recv(cxfd, buff, filelen, MSG_WAITALL);
  if(recvd == -1) perror("Receive Failed");

  if(DEBUG){fprintf(stderr, "otp_?_d: get_file; recvd and filelen: %i and %i\n", recvd, filelen);}
  if(DEBUG){fprintf(stderr, "otp_?_d: get_file; buffer: %s\n", buff);}
  if(recvd != filelen)
  {
    send_error(cxfd, INC_STR, SRVR_RESP_LEN);
    recvd = 0;
  }
  if(errno == EAGAIN || errno == EWOULDBLOCK)
    send_error(cxfd, SKTTO_STR, SRVR_RESP_LEN);
  
  send_ready(cxfd);
  return buff;
}


