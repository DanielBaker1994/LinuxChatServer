/*
 * FILE : clientMain.c
 * PROJECT :  A-04 Can We Talk?
 * PROGRAMMER : Daniel Baker
 * FIRST VERSION : April 5th 2018
 * DESCRIPTION : This is a chat-client that works in tandem with the chat-server
 *				This client uses two threads to allow for
 *"real-time" communication with other clients in this system. Using sockets to
 *communicate to the server, the program is able to connect to other clients on
 *other systems wirelessly.
 */
#include "../inc/clientHeader.h"

int main(int argc, char *argv[]) {
  startClient(argc, argv);
  return 0;
}
