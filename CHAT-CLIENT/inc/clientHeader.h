/*
* FILE : clientHeader.h
* PROJECT :  A-04 Can We Talk?
* PROGRAMMER : Daniel Baker
* FIRST VERSION : April 5th 2018
* DESCRIPTION : This header file contains all of the header files, the constant
*				definitions and the prototypes required for the chat-client.
*/
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
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include <ncurses.h>



#define PORT 5000


#define kInvalidArgumentNumber -99
#define kValidArguments 100
#define kReturnHostFailed -99
#define kReturnHostSuccessful 99
#define kGetSocketFailed -99
#define kSocketSuccessful 1
#define kConnectServerFailed -99
#define kConnectServerSucceeded 99
#define kNameMinLength 1
#define kNameMaxLength 5
#define kNameSize 6
#define kIPAddressSize 16

#define kThreadCreateFailed 6
#define kThreadCreateSuccessful 0
#define kMaxInputSize 80
#define kInputStringSize 81
#define kUserMessageSize 40
#define kSocketClosed 0

#define kMax0LengthMsg 5

void startClient(int argc, char *argv[]);
int checkArguments(int argc , char *argv[]);
int determineHost(int* whichClient , char *argv[] , struct hostent** host );
void initializeStruct( struct sockaddr_in* server_addr , struct hostent* host);
int getSocketForCommunication(int* whichClient , int* my_server_socket);
int attemptToConnectToServer(int* whichClient , int* my_server_socket , struct sockaddr_in* server_addr);
int determineIfReadOrWrite(int* my_server_socket , int* whichClient);
int threadClient(void);
int closeServerSocket(int* my_server_socket , int* whichClient);


void *clientRead(void *);
void *clientWrite(void *);



typedef struct clientInfo
{
	char clientName[kNameSize];
	int clientID;
	int isItFilled;
	int socketID;
	char readBuffer[BUFSIZ];
	char writeBuffer[BUFSIZ];
	char IPaddress[kIPAddressSize];

} clientInfo;



// dimensions for the message INPUT window
#define chat_height 	5
#define chat_width 		COLS-2
#define chat_startx 	1
#define chat_starty 	LINES-chat_height


// dimensions for the message OUTPUT window
#define msg_height LINES-chat_height-1
#define msg_width COLS
#define msg_startx 0
#define msg_starty 0

// max number of lines of messages displayed in the window
#define kMaxDisplayed 10

// ncurses prototypes
WINDOW* create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW* win);
void input_win(WINDOW * win, char* word);
void blankWin(WINDOW *win);
