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

char* encrypt_buffers(char *pt, char *key, int buffsz)
{
  if(DEBUG){ fprintf(stderr, "otp_end_d: encrypt_buffers buffsz: %i\n", buffsz); }
  char *encd = calloc(buffsz, sizeof(char));
  unsigned short sumc, tmp, tmp1;
  tmp = tmp1 = sumc = 0;
  for(int i=0; i<buffsz; i++)
  {
    if(*(pt+i) != '\n')
    {
      if(*(pt+i) == 32)
        tmp1 = 91;
      else
        tmp1 = *(pt+i); 
      sumc = *(key+i) + tmp1; 
      tmp = 65+(sumc % 27); 
      if(tmp == 91)
        tmp = 32;
      encd[i] = tmp; 
    }
  }
  encd[buffsz-1]='\n';
  if(DEBUG){ fprintf(stderr, "otp_end_d: encrypted buffer: %s", encd); }
  return encd;
}

int main(int argc, char *argv[])
{
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
  //chb: set opt to reuse
  int optval = 1;
  if(setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) error("Failed to set socket option SO_REUSEADDR");
    
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
  
  //track children
  Children rugrats = {0};

  while(1) 
  {
    cxfd = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
    if (cxfd < 0) error("ERROR on accept");

    //check to see if we're about to exceed number of children
    if(1)
      fprintf(stderr, "KIDS: %i\n", rugrats.count);

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
        //ensure client is otp_enc
        check_client(cxfd, ENCC); 
        //above call sends ready if client is ENCC 
        //get plaintext
        bfsz = get_file_len(cxfd);
        ptbuff = get_clients_file(cxfd, bfsz); 
        if(DEBUG){fprintf(stderr, "otp_enc_d: plaintext received == %s\n", ptbuff);}
        //get key
        keybfsz = get_file_len(cxfd);
        keybuff = get_clients_file(cxfd, keybfsz); 
        //check keysize 
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
        break;   
      }
      default:
      {
        //put the spawn pid in the Children struct and increment
        rugrats.kids[rugrats.count++] = spawn_pid;
        //close parent's copy of socket fd
        close(cxfd);
        break;
      }
    } 
  }
  close(listenSocketFD); // Close the listening socket
	return 0; 
}

