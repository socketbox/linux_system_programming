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


void parse_response(int cxfd)
{
  int recvd = INT_MIN;
  char resp[SRVR_RESP_LEN] = {'\0'};
  recvd = recv(cxfd, resp, SRVR_RESP_LEN, MSG_WAITALL);
  fprintf(stderr, "Received: %i in response\n", recvd); 
  //if((recvd = recv(cxfd, resp, SRVR_RESP_LEN, 0)) > 0)
  if(recvd > 0) 
  {
    if(strcmp(resp, RDY_STR) != 0)
    {
      fprintf(stderr, "Fatal: server not ready; code %s. Exiting.\n", resp); 
      exit(1);
    }
  }
  else
  {
    fprintf(stderr, "Fatal: did not receive response from server. Exiting.\n");
    exit(1);
  }
}


void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
   
  // Check usage & args 
	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext_file key_file port\n", argv[0]); exit(0); } 

  int ptfsz = verify_file(argv[1]);
  int kfsz = verify_file(argv[2]);
	
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
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) 
		error("CLIENT: ERROR connecting");

  //Identify ourselves to server
  send_client_type(socketFD, ENCC);
  parse_response(socketFD);

  //send plaintext 
  send_file_size(socketFD, ptfsz);
  parse_response(socketFD);
  
  send_file(socketFD, argv[1], ptfsz);
  parse_response(socketFD);
  
  //send key 
  send_file_size(socketFD, kfsz);
  parse_response(socketFD);
  
  send_file(socketFD, argv[2], kfsz);
  parse_response(socketFD);
  
  //get cyphertext 
  char *encbuff = NULL;
  encbuff = recv_enc_bytes(socketFD, ptfsz);
  if(DEBUG){fprintf(stderr, "%s", "otp_enc: after recv_enc_bytes\n");}

  //print out encbytes!
  fprintf(stdout, "%s", encbuff);
  
  //we're closing in the parent, so this shouldn't be necessary  
  shutdown(socketFD, SHUT_WR);
  close(socketFD);
	return 0;
}


