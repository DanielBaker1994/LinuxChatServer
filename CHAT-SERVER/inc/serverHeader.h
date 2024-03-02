/*
* FILE : serverHeader.h
* PROJECT : PROG 1970: A-04 Can We Talk?
* PROGRAMMER : Daniel Baker
* FIRST VERSION : April 5 2018
* DESCRIPTION : This header file contains all of the constants, prototypes, and
*               header files and datatype definitions for the server.
*/

/*==============================================================================
MECHANISMS
----------
The client will send the formatted message containing the IP address, client name
and message contents. The server also holds all of this information in the client
struct that will then be used to send messages to the clients. checkIfClientAccepts
is used to get the client information and fill it, while accepting the client's
connection.

To keep all of the information of the clients, there is an array of clientInfo
structs. One of the elements in the array is isItFilled, which is a 0 or a 1 depending
on whether there is a client filling the array element. If there is a client there,
the socketThread function can send messages to that client. We will also not add
a new client to that element if there is a client present. Once a client leaves,
the element will be freed, by setting the isItFilled back to 0. We will stop
broadcasting to it, and allow the element to be overwritten.

Once the client leaves, we release the socket, and close the thread by using
pthread_exit and setting the isItFilled value back to zero (kEmptyThread). We
also decriment the global variable that holds the total number of clients.

When all of the clients have left, the watchdog timer becomes active. After three
intervals of no clients, it shuts down the server. The socket is released and closed.

Please see the comments on the struct below for the descriptions of all
fields of the struct. (Called clientInfo)
===============================================================================*/
#pragma once
#define _REENTRANT
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>



// Constants
#define PORT 5000
#define kClientFailed -99
#define kClientSucceeded 99
#define kClientTwoFailed -99
#define kClientOneSucceeded 100
#define kClientTwoSucceeded 100
#define kJoiningThreadFailed 6
#define kObtainSocketFailed -101
#define kOpenPortFailed -102
#define kOpenPortSucceeded 102
#define kListenPortFailed -103
#define kListenPortSuccessful 103
#define kNumberOfClientsWaiting 11
#define kQuitLoop -104
#define kNumberOfThreads 10
#define kThreadExists 1
#define kEmptyThread 0
#define kMaxClientName 6
#define kMaxIPAddressSize 16
#define kObtainSocketSuccess -1000

// the structure holding client information
// different read and write buffers used to prevent
// errors
typedef struct clientInfo
{
    char clientName[kMaxClientName]; // the username of the person using the chat client
    int clientID;                    // The ID number of the client
    int isItFilled;                  // The variable dictating whether the element is used
    int socketID;                    // the ID number of the socket that the client is bound to
    char readBuffer[BUFSIZ];         // the buffer containing the information the the client will READ FROM
    char writeBuffer[BUFSIZ];        // the buffer containing the information that the client will WRITE TO
    char IPaddress[kMaxIPAddressSize]; // The client's IP address

} clientInfo;

// prototypes
void startServer();
void alarmHandler(int signal_number);
void installSignalHandlerWatchdog();
int obtainSocket(int* server_socket);
int openPort(struct sockaddr_in *server_addr, int* server_socket);
int listenOnSocket(int* server_socket);
int checkIfClientAccepts(int *client_socket,int* client1Gone, struct sockaddr_in *client_address  , int* client_length ,  pthread_t tid[] , int *server_socket , int* whichClient );
void *socketThread(void *);
