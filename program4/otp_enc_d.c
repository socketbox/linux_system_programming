#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


/*void redir_stderr()
{
  int devnull = open("/dev/null", O_WRONLY);
  int dupres = dup2(devnull, STDERR_FILENO);
  if(dupres == -1)
  { 
    perror("Failed to duplicate stdserr to /dev/null"); 
    exit(1); 
  }
}


*
 * duplicate stdout to the socket
 *
void redir_stdout(int sfd)
{
  int dupres = dup2(sfd, STDOUT_FILENO);
  if(dupres == -1)
  { 
    perror("Failed to duplicate stdout to socket fd"); 
    exit(1); 
  }
  //after dup'ing, we no longer need this
  close(sfd);
}
*/


/*
 *  pre:   n/a
 *   in:   a Cmd structure, a socket address structure
 *  out:   an integer indicating exit status (iffy, not well-implemented)
 * post:   either file is sent to client, or not
 */
int fork_encrypt(int sckt)
//int fork_encrypt(send_file(Cmd cs, struct sockaddr_in *client)
{
  int em, sigstatus, datafd, send_file_res;
  datafd = sigstatus = em = send_file_res = INT_MIN;
  pid_t spawnpid = INT_MIN;

  if((datafd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  { 
    perror("Failed to create socket for client data connection."); 
    exit(EXIT_FAILURE);
  }

  fflush(stdout);
  spawnpid = fork();
  switch(spawnpid)
  {
    case -1:
      perror("Fork in run_fg_child returned error");
      exit(1);
      break;

      //in child 
    case 0:
      {
        int cxres = INT_MAX; 
        socklen_t scklen = sizeof( *client );
        if((cxres = connect(datafd, (struct sockaddr *)client, scklen) < 0))
        {
          perror("Failed to connet prior to exec.");
          exit(1);
        }
        redir_stdout(datafd);
        redir_stderr();
        execlp("cat", "cat", cs.filename, NULL);
        perror("Execution of cat failed. Exiting.");
        send_file_res = errno; 
        break;
      } 
      //in parent
    default:
      //don't need the fd after forking 
      close(datafd);
      spawnpid = waitpid(spawnpid, &em, 0); 
      if (WIFEXITED(em))
      {
        sigstatus = WEXITSTATUS(em);
        send_file_res = sigstatus; 
        if(DEBUG){fprintf(stderr, "Exiting child with status %i\n", sigstatus);}
      }
      else if(WIFSIGNALED(em))
      {
        sigstatus = WTERMSIG(em);
        if(DEBUG){fprintf(stderr, "Child terminated by signal %i\n", sigstatus);}
      }
      else
      {
        fprintf(stderr, "Why are we here? Status/Signal: %i\n", sigstatus);
      }
      break;
  }
  return send_file_res;  
}


void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[256];
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
  if (setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, NULL, NULL) < 0) error("Failed to set socket option SO_REUSEADDR");
    
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

    pid_t spawn_pid = fork();
    
    switch(spawn_pid):
    { 
      case -1:
        error("Failed to fork.");
        break; 
      case 0:
        check_client( 
        encrypt(cxfd); 
        break;   
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
    close(establishedConnectionFD); // Close the existing socket which is connected to the client
  }
  */
  close(listenSocketFD); // Close the listening socket
	return 0; 
}
