#include <errno.h>
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
#include "base.h"


/**
 * pre:   cyphertext and key have been read into buffers
 * in:    char* to cyphertext and key buffers; the size of the PT buffer
 * out:   a char* to the plaintext 
 * post:  plaintext ready for transmission
 */
char* decrypt_buffers(char *ct, char *key, int buffsz)
{
  if(DEBUG){ fprintf(stderr, "otp_dec_d: decrypt_buffers buffsz: %i\n", buffsz); }
  char *decd = calloc(buffsz, sizeof(char));
  signed int diffc, tmp0, tmp1, tmp2, tmp3, tmp5;
  tmp0 = tmp1 = tmp2 = tmp3 = tmp5 = diffc = 0;
  float tmp4 = 0.0; 
  //this was helpful: https://stackoverflow.com/questions/10133194/reverse-modulus-operator 
  for(int i=0; i<buffsz; i++)
  {
    //we don't care about the newline
    if(*(ct+i) != '\n')
    {
      //convert space back to ASCII char after Z for calculation
      if(*(ct+i) == 32)
        tmp0 = 91;
      else
        tmp0 = *(ct+i); 
      //bring value of cypher'd char back down to 0 - 26 
      tmp1 = tmp0 - 65;
      //makes sense within context of algebraic operation (a + x) mod m = b -> a + 65 + x = nm + b 
      tmp2 = (*(key+i) + 65);
      tmp3 = (tmp1 - tmp2);
      //necessary given the way that C handles modular arithmetic of negative values
      tmp4 = ceilf( fabs( (tmp3+.0)/27 ) );
      tmp5 = tmp3 + (tmp4*27);
      diffc = tmp5 + 65;
      //once again, our friend the space char 
      if(diffc == 91)
        diffc = 32;
      decd[i] = diffc; 
    }
  }
  decd[buffsz-1]='\n';
  if(DEBUG){ fprintf(stderr, "otp_dec_d: decrypted buffer: %s", decd); }
  return decd;
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
  //chb: set opt to reuse
  int optval = 1;
  if(setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) 
    error("Failed to set socket option SO_REUSEADDR");
    
	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 
		error("ERROR on binding");

  // Flip the socket on - it can now receive up to 5 connections	
  listen(listenSocketFD, 5); 

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress);
 
  while(1) 
  {
	  //Accept a connection, blocking until one becomes available 
    if(DEBUG){fprintf(stderr, "%s\n", "otp_dec_d: accept loop.");} 
    cxfd = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
    //should be using a signal for this...
    while(KIDCOUNT > 4)
    {  
      usleep(1250);
    }

    KIDCOUNT++;
    if(DEBUG){fprintf(stderr, "Incrementing KIDCOUNT. KIDCOUNT==%i\n", KIDCOUNT);}
    pid_t spawn_pid = fork();
    
    switch(spawn_pid)
    { 
      case -1:
        error("Failed to fork.");
        KIDCOUNT--;
        break; 
      case 0:
      {
        //child won't need this
        close(listenSocketFD);
        if(DEBUG){fprintf(stderr, "otp_dec_d(child): child pid: %i\n", getpid());}
        
        //setup 
        int bfsz, keybfsz;
        bfsz = keybfsz = INT_MIN;
        char *ptbuff, *keybuff, *cypherbuff;
        keybuff = ptbuff = cypherbuff = NULL;
        
        //this call sends ready if client is DECC 
        check_client(cxfd, DECC); 
        
        //get cyphertext
        bfsz = get_file_len(cxfd);
        cypherbuff = get_clients_file(cxfd, bfsz); 
        if(DEBUG){fprintf(stderr, "otp_dec_d: cyphertext received == %s\n", cypherbuff);}
        
        //get key
        keybfsz = get_file_len(cxfd);
        keybuff = get_clients_file(cxfd, keybfsz); 
        
        //check keysize; this is likely an unnecessary safeguard, given client file validation
        if( keybfsz < bfsz ) error("Key of insufficient size.");
        
        //decrypt cyphertext 
        if(DEBUG){fprintf(stderr, "%s", "otp_dec_d: before decrypt\n");}
        if(DEBUG){fprintf(stderr, "otp_dec_d: cypherbuff== %s\n", cypherbuff);}
        if(DEBUG){fprintf(stderr, "otp_dec_d: keybuff== %s\n", keybuff);}
        ptbuff = decrypt_buffers(cypherbuff, keybuff, bfsz);
        //send plaintext
        send(cxfd, ptbuff, bfsz, 0);
        
        //cleanup 
        free(ptbuff);
        free(keybuff);
        free(cypherbuff);
        ptbuff = keybuff = cypherbuff = NULL;
        //close child's copies of the socket descriptions
        close(cxfd);
        exit(0); 
        break;   
      }
      default:
        //close parent's copy of socket fd
        close(cxfd);
        break;
    } 
  }
  close(listenSocketFD); // Close the listening socket
	return 0; 
}

