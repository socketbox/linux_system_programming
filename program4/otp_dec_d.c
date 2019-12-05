#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <math.h>
#include "protocol.h"
#include "srvr_common.h"

char* decrypt_buffers(char *ct, char *key, int buffsz)
{
  if(DEBUG){ fprintf(stderr, "otp_dec_d: decrypt_buffers buffsz: %i\n", buffsz); }
  char *decd = calloc(buffsz, sizeof(char));
  signed int diffc, tmp0, tmp1, tmp2, tmp3, tmp5;
  tmp0 = tmp1 = tmp2 = tmp3 = tmp5 = diffc = 0;
  float tmp4 = 0.0; 
  //https://stackoverflow.com/questions/10133194/reverse-modulus-operator 
  for(int i=0; i<buffsz; i++)
  {
    if(*(ct+i) != '\n')
    {
      if(*(ct+i) == 32)
        tmp0 = 91;
      else
        tmp0 = *(ct+i); 
      tmp1 = tmp0 - 65;
      tmp2 = (*(key+i) + 65);
      tmp3 = (tmp1 - tmp2);
      tmp4 = ceilf( fabs( (tmp3+.0)/27 ) );
      tmp5 = tmp3 + (tmp4*27);
      diffc = tmp5 + 65;
      if(DEBUG){ fprintf(stderr, "otp_dec_d: in decrypt_buffers tmp1, tmp2, tmp3, tmp4, tmp5, diffc:\
          %i; %i; %i; %f; %i; %i;", tmp1, tmp2, tmp3, tmp4, tmp5, diffc); }
      if(diffc == 91)
        diffc = 32;
      decd[i] = diffc; 
      if(DEBUG){ fprintf(stderr, "%c\n", diffc); }
    }
  }
  decd[buffsz-1]='\n';
  if(DEBUG){ fprintf(stderr, "otp_dec_d: decrypted buffer: %s", decd); }
  return decd;
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
        char *ptbuff, *keybuff, *cypherbuff;
        keybuff = ptbuff = cypherbuff = NULL;
        //ensure client is otp_enc
        check_client(cxfd, DECC); 
        //above call sends ready if client is ENCC 
        //get plaintext
        bfsz = get_file_len(cxfd);
        cypherbuff = get_clients_file(cxfd, bfsz); 
        if(DEBUG){fprintf(stderr, "otp_dec_d: cyphertext received == %s\n", cypherbuff);}
        //get key
        keybfsz = get_file_len(cxfd);
        keybuff = get_clients_file(cxfd, keybfsz); 
        //check keysize 
        if( keybfsz < bfsz ) error("Key of insufficient size.");
        
        //encrypt plaintext 
        if(DEBUG){fprintf(stderr, "%s", "otp_dec_d: before decrypt\n");}
        if(DEBUG){fprintf(stderr, "otp_dec_d: cypherbuff== %s\n", cypherbuff);}
        if(DEBUG){fprintf(stderr, "otp_dec_d: keybuff== %s\n", keybuff);}
        ptbuff = decrypt_buffers(cypherbuff, keybuff, bfsz);
        //send cyphertext
        send(cxfd, ptbuff, bfsz, 0);
        
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

