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


void encrypt_buffers(char *pt, char *key, char **encd, int buffsz)
{
  encd = calloc(buffsz, sizeof(char));
  unsigned short sumc = 0;
  for(int i=0; i<buffsz; i++)
  {
    sumc = *(key+i) + **(encd+i);
    *(encd[i]) = sumc % 27; 
  }
  if(DEBUG){ fprintf(stderr, "otp_end_d: encrypted buffer: %s\n", *encd); }
}

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
  fprintf(stderr, "DEBUG: %i\n", DEBUG);

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
  while(1) 
  {
    cxfd = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
    if (cxfd < 0) error("ERROR on accept");

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
        char **ptbuff, **keybuff, **cypherbuff;
        keybuff = ptbuff = cypherbuff = NULL;

        //ensure client is otp_enc
        check_client(cxfd, ENCC); 
        //get plaintext
        bfsz = get_clients_file(cxfd, ptbuff); 
        if(DEBUG){fprintf(stderr, "otp_enc_d: plaintext received == %s\n", *ptbuff);}
        //get key
        keybfsz = get_clients_file(cxfd, keybuff); 
        //check keysize 
        if( keybfsz < bfsz ) error("Key of insufficient size.");
        
        //encrypt plaintext 
        if(DEBUG){fprintf(stderr, "%s", "otp_enc_d: before encrypt\n");}
        if(DEBUG){fprintf(stderr, "otp_enc_d: ptbuff== %s\n", *ptbuff);}
        if(DEBUG){fprintf(stderr, "otp_enc_d: keybuff== %s\n", *keybuff);}
        encrypt_buffers(*ptbuff, *keybuff, cypherbuff, bfsz);
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
        //close parent's copy of socket fd
        close(cxfd);
        break;
    } 
    // Get the message from the client and display it
    /*memset(buffer, '\0', 256);
    charsRead = recv(establishedConnectionFD, buffer, 255, 0); // Read the client's message from the socket
    if (charsRead < 0) error("ERROR reading from socket");
    printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    // Send a Success message back to the client
    charsRead = send(establishedConnectionFD, "I am the server, and I got your message", 39, 0); // Send success back
    if (charsRead < 0) error("ERROR writing to socket");
    //TODO this will not need to be done here, as we'll close the socket after spawning 
    close(establishedConnectionFD); // Close the existing socket which is connected to the client*/
  }
  close(listenSocketFD); // Close the listening socket
	return 0; 
}

