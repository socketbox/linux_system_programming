#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "protocol.h"
#include "srvr_common.h"
#include "base.h"


/**
 * pre:   valid plaintext and key have been read into buffers
 * in:    char* to plaintext and key buffers; the size of the PT buffer
 * out:   a char* to the cyphertext 
 * post:  cyphertext ready for transmission
 */
char* encrypt_buffers(char *pt, char *key, int buffsz)
{
  if(DEBUG){ fprintf(stderr, "otp_end_d: encrypt_buffers buffsz: %i\n", buffsz); }
  char *encd = calloc(buffsz, sizeof(char));
  //we can use unsigned shorts here, but not in decode 
  unsigned short sumc, tmp, tmp1;
  tmp = tmp1 = sumc = 0;
  for(int i=0; i<buffsz; i++)
  {
    //don't encrypt the final newline
    if(*(pt+i) != '\n')
    {
      //for the purposes of summation and modulo, treat the space as ASCII char after Z
      if(*(pt+i) == 32)
        tmp1 = 91;
      else
        tmp1 = *(pt+i); 
      sumc = *(key+i) + tmp1;
      //add 65 ('A') to put outcome in proper, printing range
      tmp = 65+(sumc % 27); 
      //correct initial contrivance involving spaces 
      if(tmp == 91)
        tmp = 32;
      encd[i] = tmp; 
    }
  }
  encd[buffsz-1]='\n';
  if(DEBUG){ fprintf(stderr, "otp_end_d: encrypted buffer: %s", encd); }
  return encd;
}

//global for child counting
int KIDCOUNT;

/**
 * Much of this is taken from the server.c code provided
 */
int main(int argc, char *argv[])
{
  //ignore all signals but SIGCHLD and SIGINT
  set_srvr_proc_mask();
 
  //register the SIGCHLD handler
  reg_chld_handler();
 
	int listenSocketFD, cxfd, portNumber;
	socklen_t sizeOfClientInfo;
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

  //set option to reuse the socket; essentialy for smooth testing
  int optval = 1;
  if(setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) error("Failed to set socket option SO_REUSEADDR");
    
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	
  listen(listenSocketFD, 5); 

	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
  
  while(1) 
  {
	  //Accept a connection, blocking until one becomes available 
    cxfd = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
    if (cxfd < 0)
    //should be using a signal for this...
    while(KIDCOUNT > 4)
    {  
      usleep(1250);
    }


    if(DEBUG){fprintf(stderr, "%s\n", "otp_enc_d: accept loop.");} 
    pid_t spawn_pid = fork();
    
    switch(spawn_pid)
    { 
      case -1:
        error("Failed to fork.");
        break; 
      case 0:
      {
        //setup 
        int bfsz, keybfsz;
        bfsz = keybfsz = INT_MIN;
        char *ptbuff, *keybuff, *cypherbuff;
        keybuff = ptbuff = cypherbuff = NULL;
  
        //ensure client is otp_enc; this call sends ready if client is ENCC 
        check_client(cxfd, ENCC); 
        
        //get plaintext
        bfsz = get_file_len(cxfd);
        ptbuff = get_clients_file(cxfd, bfsz); 
        if(DEBUG){fprintf(stderr, "otp_enc_d: plaintext received == %s\n", ptbuff);}
        
        //get key
        keybfsz = get_file_len(cxfd);
        keybuff = get_clients_file(cxfd, keybfsz); 
        
        //check keysize; this is likely an unnecessary safeguard, given client file validation
        if( keybfsz < bfsz ) error("Key of insufficient size.");
        
        //encrypt plaintext 
        if(DEBUG){fprintf(stderr, "%s", "otp_enc_d: before encrypt\n");}
        if(DEBUG){fprintf(stderr, "otp_enc_d: ptbuff== %s\n", ptbuff);}
        if(DEBUG){fprintf(stderr, "otp_enc_d: keybuff== %s\n", keybuff);}
        cypherbuff = encrypt_buffers(ptbuff, keybuff, bfsz);
        //send cyphertext
        send(cxfd, cypherbuff, bfsz, 0);
        
        //cleanup 
        free(ptbuff);
        free(keybuff);
        free(cypherbuff);
        ptbuff = keybuff = cypherbuff = NULL;
        close(cxfd);
        exit(0);
        break;   
      }
      default:
      {
        //close parent's copy of socket fd
        close(cxfd);
        break;
      }
    } 
  } 
  close(listenSocketFD); // Close the listening socket
	return 0; 
}

