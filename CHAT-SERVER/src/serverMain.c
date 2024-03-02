
/*
 * FILE : serverMain.c
 * PROJECT : PROG 1970: A-04 Can We Talk?
 * PROGRAMMER : Daniel Baker
 * FIRST VERSION : April 5 2018
 * DESCRIPTION : This server is used to transmit information to the clients that
 * connect to it. It is used by a chat-system. Once a client connects to it, it
 * splits off into threads, and from there recieves messages from it's client
 * and broadcasts it to every client connected to the server.
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

int main(void) {

  startServer();
  return 0;
}
