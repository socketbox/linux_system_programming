#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h> 
#include <netdb.h> 
#include "protocol.h"
#include "clnt_common.h"
#include "base.h"


/**
 * 
 *  
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



int main(int argc, char *argv[])
{
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
   
  // Check usage & args 
	if (argc < 4) { fprintf(stderr,"USAGE: %s cyphertext_file key_file port\n", argv[0]); exit(0); } 

  int cyptfsz = verify_file(argv[1]);
  int kfsz = verify_file(argv[2]);
  if(kfsz < cyptfsz){ error("Key file of insufficient length"); }

  // Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if(socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 
		error("CLIENT: ERROR connecting");

  //Identify ourselves to server
  send_client_type(socketFD, DECC);
  parse_response(socketFD);

  //send cyphertext 
  send_file_size(socketFD, cyptfsz);
  parse_response(socketFD);
  
  send_file(socketFD, argv[1], cyptfsz);
  parse_response(socketFD);
  
  //send key 
  send_file_size(socketFD, kfsz);
  parse_response(socketFD);
  
  send_file(socketFD, argv[2], kfsz);
  parse_response(socketFD);
  
  //get cyphertext 
  char *decbuff = NULL;
  decbuff = recv_bytes(socketFD, cyptfsz);
  if(DEBUG){fprintf(stderr, "%s", "otp_enc: after recv_bytes\n");}

  //print out encbytes!
  fprintf(stdout, "%s", decbuff);
  
  //we're closing in the parent, so this shouldn't be necessary  
  shutdown(socketFD, SHUT_WR);
  close(socketFD);
	return 0;
}


