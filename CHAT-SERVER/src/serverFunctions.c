/*
 * FILE : serverFunctions.c
 * PROJECT : PROG 1970: A-04-Can we talk?
 * PROGRAMMER :   Daniel Baker
 * FIRST VERSION : April 5th 2018
 * DESCRIPTION : This file contains all of the functions that the chat server
 * requires to function. It also contains three global variables that are used
 * to keep track of the clients. Many of the functions and the logic contained
 * herein is referenced from Sean Clarke's examples (sockets example, threads
 * examples)
 *
 */

/*==============================================================================
MECHANISMS
----------
The client will send the formatted message containing the IP address, client
name and message contents. The server also holds all of this information in the
client struct that will then be used to send messages to the clients.
checkIfClientAccepts is used to get the client information and fill it, while
accepting the client's connection.

To keep all of the information of the clients, there is an array of clientInfo
structs. One of the elements in the array is isItFilled, which is a 0 or a 1
depending on whether there is a client filling the array element. If there is a
client there, the socketThread function can send messages to that client. We
will also not add a new client to that element if there is a client present.
Once a client leaves, the element will be freed, by setting the isItFilled back
to 0. We will stop broadcasting to it, and allow the element to be overwritten.

Once the client leaves, we release the socket, and close the thread by using
pthread_exit and setting the isItFilled value back to zero (kEmptyThread). We
also decriment the global variable that holds the total number of clients.

When all of the clients have left, the watchdog timer becomes active. After
three intervals of no clients, it shuts down the server. The socket is released
and closed.

Please see the comments on the struct in the header for the descriptions of all
fields of the struct.
===============================================================================*/
#include "../inc/serverHeader.h"

// used for accepting incoming command and also holding the command's response

// global to keep track of the number of connections
static int nClients = 0; // number of clients in the system
static int nNoConnections =
    0; // number of intervals with no clients, used by WATCHDOG
static int ClientArrayNumber =
    0; // the array number of clients, used when adding clients

clientInfo clientStruct[kNumberOfThreads] = {
    0}; // the array of structs that holds all of the client information

// FUNCTION: startServer()
// DESCRIPTION: This function is used to control
// the flow of the client. It is our puppet master It is used to ensure that if
// functions used to support creating the server fail, the
// program will terminate. Itr also reduces the problem domain
// from the main()
// PARAMETERS : void
// RETURNS : void

void startServer() {

  int server_socket = 0;

  int client_socket = -100;
  int client_socket2 = 0;
  int client_length = 0;
  int client_len2 = 0;
  int client1Gone = 0;
  int client2Gone = 0;
  int whichClient = 0;
  pthread_t tid[kNumberOfThreads];
  int arrayOfThreads[kNumberOfThreads];

  struct sockaddr_in client_address;
  struct sockaddr_in server_addr;

  for (int i = 0; i < kNumberOfThreads; i++) {
    clientStruct[i].clientID = 0;
    clientStruct[i].isItFilled = kEmptyThread;
  }

  installSignalHandlerWatchdog();
  int returnCodeClient = 0;

  int obtainSocketRetValue = obtainSocket(&server_socket);
  if (obtainSocketRetValue != kObtainSocketFailed) {
    int portReturnValue = openPort(&server_addr, &server_socket);
    if (portReturnValue != kOpenPortFailed) {
      int listenOnSocketReturnValue = listenOnSocket(&server_socket);
      if (listenOnSocketReturnValue != kListenPortFailed) {
        returnCodeClient = checkIfClientAccepts(
            &client_socket, &client1Gone, &client_address, &client_length, tid,
            &server_socket, &whichClient);
      } else {
        printf("ERROR: Failed to accept clients. \n");
      }

    } else {
      printf("ERROR: Failed to listen on socket.\n");
    }

  } else {
    printf(" ERROR:Failed to optain socket.\n");
  }
}

// FUNCTION: installSignalHandlerWatchdog()
// DESCRIPTION: This function is designed to install
// the watchdog timer. The purpose of the watchdog timer is
// to check to see the number of clients are active on the server.
// The watchdog is set to go off every 10 secconds.
// PARAMETERS : void
// RETURNS : void

void installSignalHandlerWatchdog() {
  // sigalrm is
  signal(SIGALRM, alarmHandler);
  alarm(10);
}

// FUNCTION: obtainSocket()
// DESCRIPTION: This function is designed to obtain
// a stream socket. This function uses AF_INEt system
// call, which tells us it is an IPV4 communication protocol
// This server socket also uses a socket stream. Sockets are designed
// to process and recieve packets from the clients
// PARAMETERS :int* server_socket : a pointer to the socket number that we will
// fill in this function. RETURNS :  kObtainSocketFailed if we could not obtain
// the socket or kObtainSocketSuccess if we do get the socket
int obtainSocket(int *server_socket) {

  /*
   * obtain a socket for the server
   */
  // AF_INET is a system call - this tells that it is an IPV4 communication
  if ((*(server_socket) = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return kObtainSocketFailed;
  } else {
    return kObtainSocketSuccess;
  }
}

// FUNCTION: openPort()
// DESCRIPTION: This function uses the struct to hold
// the server address. It attempts to open a port at 5000
// If it is successful, the function binds. The address specified
// by the struct is binded to the socket. Upon failure, the program
// closes
// PARAMETERS : struct sockaddr_in *server_addr: the server's IP information,
// used to bind to the socket
//              int* server_socket: A pointer to the int that holds the server's
//              ID
// RETURNS : kOpenPortSucceeded if the port is successfully opened
//           or kOpenPortFailed if the port could not be opened

int openPort(struct sockaddr_in *server_addr, int *server_socket) {
  // opening the ports- ready for communication - Binding socket to server
  // address
  /*
   * initialize our server address info for binding purposes
   */
  memset(server_addr, 0, sizeof(server_addr));
  server_addr->sin_family = AF_INET;
  server_addr->sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr->sin_port = htons(PORT);

  fflush(stdout);
  if (bind(*(server_socket), (struct sockaddr *)server_addr,
           sizeof(*(server_addr))) < 0) {
    close(*server_socket);
    return kOpenPortFailed;
  } else {
    return kOpenPortSucceeded;
  }
}

// FUNCTION: listenOnSocket()
// DESCRIPTION: This function specifies the maximum
// number of clients who can be held in a queue waiting to
// connect to the server. This program holds up to 10 clients.
// PARAMETERS : int* server_socket: A pointer to the server's ID.
// RETURNS : kListenPortFailed if the listen failed, and kListenPortSuccessful
// if listening on the port was successful
int listenOnSocket(int *server_socket) {
  // listen on socket for incoming connections
  /*
   * start listening on the socket
   */

  if (listen(*server_socket, kNumberOfClientsWaiting) < 0) {
    close(*server_socket);
    return kListenPortFailed;
  } else {
    return kListenPortSuccessful;
  }
}

// FUNCTION: checkIfClientAccepts( )
// DESCRIPTION: This function checks to see if the client
// accepts. If they do, the program spawns a thread. In this function
// the new clients thread ID is recorded. If 10 clients connect,
// the threads join and everything else from this program runs
// in the foreground until the clients complete.
// PARAMETERS :
//      int *client_socket: A pointer to the int holding the client's socket ID
//      int* client1Gone: A pointer to an int holding a variable stating whether
//      the client is there struct sockaddr_in *client_address: holds the
//      client's IP and port information int* client_length:  a pointer to an
//      int that holds the size of the clients looking to be accepted pthread_t
//      tid[] : The array that holds the thread ID's for all 10 potential
//      clients int *server_socket: the pointer to the int that holds the server
//      socket's details int* whichClient: a pointer to an int that holds the
//      client's ID information
// RETURNS : kClientFailed if we cannot accept the client,  kJoiningThreadFailed
// if we cannot
//          join the threads when all clients are present, or kClientSucceeded
//          if all is successful
int checkIfClientAccepts(int *client_socket, int *client1Gone,
                         struct sockaddr_in *client_address, int *client_length,
                         pthread_t tid[], int *server_socket,
                         int *whichClient) {
  int retValue = 0;
  while (nClients < kNumberOfThreads) {

    //* accept a packet from the client1 --- client 1 connecting.....
    *client_length = sizeof(client_address);
    if ((*client_socket =
             accept(*server_socket, (struct sockaddr *)&client_address,
                    client_length)) == -1) {
      close(*server_socket);
      retValue = kClientFailed;
      return retValue;
    }
    if (*client_socket > 0) // if it is greater than negative number it
                            // succeeded - redundant but it isnt working
    {
      nClients++;

    } else {
      retValue = kClientFailed;
      return retValue;
    }

    int errorCode = -1;
    errorCode = pthread_create(&(tid[(nClients - 1)]), NULL, socketThread,
                               (void *)client_socket);
    if (errorCode != 0) {
      printf("[SERVER] : pthread_create() FAILED\n");
      fflush(stdout);
    } else {
      fflush(stdout);
      // return 5;
    }

  }

  //  once we reach 10 clients - let's go into a busy "join" loop waiting for
  // all of the clients to finish and join back up to this main thread
  for (int i = 0; i < kNumberOfThreads; i++) {
    if ((pthread_join(tid[i], (void *)&whichClient) !=
         0)) // this line needed a ) and a &; I added them.
    {
      printf("[SERVER] : pthread_join() FAILED\n");
      fflush(stdout);

      return kJoiningThreadFailed;
    }
  }

  close(*server_socket);

  return kClientSucceeded;
}

// FUNCTION: alarmHandler()
// DESCRIPTION: This function is set again from within itself.
// It tracks the number of clients who have connected
// to the server. If it determines that there are not any
// clients left on the server, the function closes the program.
// it catches a signal from a watchdog timer.
// PARAMETERS : int signal_number : The number of the signal that the watchdog
// will throw RETURNS : void
/* Watch dog timer - to keep informed and watch how long the server goes without
 * a connection */
void alarmHandler(int signal_number) {
  if (nClients == 0) {
    nNoConnections++;
    // It's been 10 seconds - determine how many 10 second intervals its been
    // without a connection
  } else {
    // reset the number of times we've checked with no client connections
    nNoConnections = 0;
  }

  // reactivate signal handler for next time ...
  if ((nNoConnections == 3) && (nClients == 0)) {
    printf("Server is shutting down due to inactivity\n.");
    exit(-1);
  }

  signal(signal_number, alarmHandler);
  alarm(10); // reset alarm
}

// FUNCTION:socketThread()
// DESCRIPTION: This creates a thread within
// the program. Up to 10 of these threads may
// be spawned. Each individual thread broadcasts
// to all other threads on the server and reads from
// the server socket.
// PARAMETERS :void *clientSocket : A pointer to the client's specific socket
// RETURNS : void* NULL. All reading and writing from the socket will occur
// here. it doesn't matter why we exit the thread, once we leave, we are
// available and we quit.
void *socketThread(void *clientSocket) {
  // used for accepting incoming command and also holding the command's response
  int localClientNumber = -1;
  localClientNumber = nClients - 1;

  int sizeOfRead;
  int timeToExit = -1;

  // remap the clientSocket value (which is a void*) back into an INT
  int clSocket = *((int *)clientSocket);

  clientStruct[localClientNumber].clientID =
      *((int *)clientSocket); // assigning client socket
  clientStruct[localClientNumber].isItFilled = kThreadExists;

  // we will quit once the client write >>bye<<
  // otherwise we read, and broadcast to every other client present.
  while (strstr(clientStruct[localClientNumber].writeBuffer, ">>bye<<") ==
         NULL) {

    memset(clientStruct[localClientNumber].readBuffer, 0, BUFSIZ);
    memset(clientStruct[localClientNumber].writeBuffer, 0, BUFSIZ);

    if (read(clientStruct[localClientNumber].clientID,
             clientStruct[localClientNumber].writeBuffer, BUFSIZ) == -1) {
      printf("ERROR: failed to read from client %d\n",
             clientStruct[localClientNumber].clientID);
      break;
    }

    {
      strcpy(clientStruct[localClientNumber].readBuffer,
             clientStruct[localClientNumber].writeBuffer);
      for (int i = 0; i < 10; i++) {
        if (clientStruct[i].isItFilled !=
            kEmptyThread) // if not zero it means that in that a pid was assigned
        {
          if (write(clientStruct[i].clientID,
                    clientStruct[localClientNumber].readBuffer, BUFSIZ) < 1) {
            printf("ERROR: Failed to write %s to Client %d\n",
                   clientStruct[localClientNumber].readBuffer,
                   clientStruct[i].clientID);
            break;
          }
        }
      }
    }
  }

  clientStruct[localClientNumber].isItFilled = kEmptyThread; // dictates that the client is not present and the spot is availble in the array
  nClients--;
  pthread_exit(0);
  return 0;
}
