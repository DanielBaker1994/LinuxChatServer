/*
 * FILE : clientFunction.c
 * PROJECT :  A-04 Can We Talk?
 * PROGRAMMER : Daniel Baker
 * FIRST VERSION : April 5th 2018
 * DESCRIPTION : This file contains the functions necessary to support
 * allowing to connect to a server via a socket. Included in this document is
 * two threads, one that allows the client to read from the socket, and another
 * that supports the client to write to the socket. If the program is
 *unsuccessful at starting the server at the passing arguments via command line,
 *getting the hos struct, intializing the server struct and getting the server
 *struct – the program will terminate. The client will continually to read and
 *write to the server in the absence of this REFERERENCE : Many of these
 *functions, this code and the logic within this file has been created using
 *examples from Sean Clarke. (Sockets example, threading example and ncurses
 *examples)
 */

#include "../inc/clientHeader.h"

clientInfo clientStruct = {0};

pthread_t clientReadThreadID;
pthread_t clientWriteThreadID;
WINDOW *chat_win;
WINDOW *msg_win;

// FUNCTION: startClient()
// DESCRIPTION: This function is designed to control the flow of the program.
// It holds all the functions used to support starting the client. If any of the
// functions designed to support starting the client shall fail, the program
// ends. This function takes user input from the command line. PARAMETERS : int
// argc, char *argv[] RETURNS : void
void startClient(int argc, char *argv[]) {

  int my_server_socket;
  int len;
  int done;
  int whichClient;
  struct sockaddr_in server_addr;
  struct hostent *host;

  int retValueCheckArguments = checkArguments(argc, argv);
  if (retValueCheckArguments != kInvalidArgumentNumber) {

    int retValueDetermineHost = determineHost(&whichClient, argv, &host);
    if (retValueDetermineHost != kReturnHostFailed) {

      initializeStruct(&server_addr, host);

      int socketReturnValue =
          getSocketForCommunication(&whichClient, &my_server_socket);
      if (socketReturnValue != kGetSocketFailed) {

        int connectServerRetValue = attemptToConnectToServer(
            &whichClient, &my_server_socket, &server_addr);
        if (connectServerRetValue != kConnectServerFailed) {
          initscr(); // using screens                     /* Start curses mode
                     // */
          cbreak();  // tells program to stop and get the character entered
          noecho();  // do not output to the screen
          refresh(); // equivalent to fflush, but part of ncurses
          // now make our threads!
          threadClient();

          closeServerSocket(&my_server_socket, &whichClient);
          refresh();
          destroy_win(chat_win);
          destroy_win(msg_win);
          endwin();
        } else {
          printf("ERROR: Failed to connect to server!\n");
        }
      } else {
        printf("ERROR: Failed to find socket!\n");
      }

    } else {
      printf("ERROR: Failed to bind to socket!\n");
    }

  } else {
    printf("USAGE : chat-client <clientID> <server_name>\n");
    printf("\t<clientID>: Your username. Maximum %d characters.\n",
           kNameMaxLength);
    printf("\t<server_name>: The server's IP address. Must be in "
           "XXX.XXX.XXX.XXX format.\n");
    printf("\tNote: do not include <> in command line switches.\n");
    printf("\tNote: switches must appear in the order above.\n");
  }
}

// FUNCTION: checkArguments()
// DESCRIPTION: This function is designed to ensure that
// the user passes 2 arguments on the command line, the IP address of the server
// and the clients name. If too few or many arguments are specified, a usage
// statement is printed. PARAMETERS : int argc: number of command line arguments
// char *argv[]: an array of strings holding the
// arguments
// RETURNS :  int kInvalidArgumentNumber if the user entered invalid arguments,
// or kValidArguments otherwise
int checkArguments(int argc, char *argv[]) {
  fflush(stdout);

  /*
   * check for sanity
   */
  if (argc != 3) {
    printf("USAGE : chat-client <clientID> <server_name>\n");
    printf("\t<clientID>: Your username. Maximum %d characters.\n",
           kNameMaxLength);
    printf("\t<server_name>: The server's IP address. Must be in "
           "XXX.XXX.XXX.XXX format.\n");
    printf("\tNote: do not include <> in command line switches.\n");
    printf("\tNote: switches must appear in the order above.\n");

    return kInvalidArgumentNumber;
  } else {

    int lengthOfName = strlen(argv[1]);
    if (lengthOfName >= kNameMinLength && lengthOfName <= kNameMaxLength) {
      strcpy(clientStruct.clientName, argv[1]);
    } else {
      return kInvalidArgumentNumber;
    }
  }
  return kValidArguments;
}

// FUNCTION: determineHost()
// DESCRIPTION: Determines if the IP address passed on the command line is able
// to be determined. If it is successful, it passes the host host name into a
// struct. PARAMETERS : int* whichClient: the clientID that is used to bind to
// the socket char *argv[]: an array of strings holding the
// arguments struct hostent** host: holds host information
// RETURNS : int constant, whether the host was successful
// (kReturnHostSuccessful) or failed (kReturnHostFailed)
int determineHost(int *whichClient, char *argv[], struct hostent **host) {

  /*
   * determine host info for server name supplied
   */
  if (((*host) = gethostbyname(argv[2])) == NULL) {
    printf("ERROR! Could not authenticate host IP address\n");
    printf(
        "\tPlease check that IP address is correct and that host is online.\n");

    return kReturnHostFailed;
  }

  return kReturnHostSuccessful;
}

// FUNCTION: initializeStruct()
// DESCRIPTION: This function first uses memset to clear the struct that will
// contain the socket to a host. Next, it initializes the struct that will be
// used to get a socket to the host. Htons is used to convert the unsigned int
// into a network byte order PARAMETERS : struct sockaddr_in* server_addr: our
// IP address struct hostent* host: host's details
// RETURNS : NONE

void initializeStruct(struct sockaddr_in *server_addr, struct hostent *host) {

  /*
   * initialize struct to get a socket to host
   */
  memset(server_addr, 0, sizeof(*server_addr));
  server_addr->sin_family = AF_INET;
  memcpy(&server_addr->sin_addr, host->h_addr, host->h_length);
  server_addr->sin_port = htons(PORT);

  char myIP[kIPAddressSize] = {'\0'};
  char *ipbuf;
  int oct1;
  int oct2;
  int oct3;
  int oct4;
  ipbuf = inet_ntoa(*((struct in_addr *)host->h_addr_list[0]));
  sscanf(ipbuf, "%d.%d.%d.%d", &oct1, &oct2, &oct3, &oct4);
  /*Now we have all the infromation from the host*/
  sprintf(myIP, "%03d.%03d.%03d.%03d", oct1, oct2, oct3, oct4);
  strcpy(clientStruct.IPaddress, myIP);
}

// FUNCTION: getSocketForCommunication()
// DESCRIPTION: This function is used to get a stream socket so that the
// client can connect to the server. A pointer to the server socket is returned
// upon success. The protocol sock_stream is agreed upon in this function.
// PARAMETERS : int* whichClient: Our client ID
// int* my_server_socket: Socket ID
// RETURNS : int kGetSocketFailed if the process failed, or kSocketSuccessful if
// it succeeded

int getSocketForCommunication(int *whichClient, int *my_server_socket) {

  /*
   * get a socket for communications
   */
  if ((*my_server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("getSocketForCommunication() FAILED\n");
    return kGetSocketFailed;
  } else {

    clientStruct.socketID = *my_server_socket;

    fflush(stdout);
    return kSocketSuccessful;
  }
}

// FUNCTION: attemptToConnectToServer()
// DESCRIPTION: This function attempts to connect to the newly created server,
// using the server socket generated in getSocketForCommunication(). If this
// function is unsuccessful the server socket is closed and the cleints progress
// is terminated PARAMETERS : int* whichClient: client ID
// int* my_server_socket : pointer to socket ID so it can be canged
// struct sockaddr_in* server_addr	: the server's details
// RETURNS : int: kConnectServerFailed if the attempt failed or
// kConnectServerSucceeded if successful

int attemptToConnectToServer(int *whichClient, int *my_server_socket,
                             struct sockaddr_in *server_addr) {
  /*
   * attempt a connection to server
   */

  fflush(stdout);
  if (connect(*my_server_socket, (struct sockaddr *)server_addr,
              sizeof(*server_addr)) < 0) {
    printf("ERROR: connection to server failed\n");

    close(*my_server_socket);
    return kConnectServerFailed;
  } else {
    return kConnectServerSucceeded;
  }
}

// FUNCTION: threadClient()
// DESCRIPTION: This function is used to create a threaded client
// It calls upon the function pthread_create which is used to start a new thread
// in the call process. In this function, two threads are created, one for the
// client to read from the server socket, and the other for the client to write
// to the server socket PARAMETERS : void RETURNS : int: kThreadCreateFailed if
// threading failed or kThreadCreateSuccessful if successful
int threadClient(void) {

  int errorCodeClientRead = -1;

  errorCodeClientRead = pthread_create(&clientReadThreadID, NULL, clientRead,
                                       (void *)&clientStruct);

  if (errorCodeClientRead != 0) {
    printf("[SERVER] : pthread_create() FAILED\n");
    fflush(stdout);
  }

  int errorCodeClientWrite = -1;

  errorCodeClientWrite = pthread_create(&clientWriteThreadID, NULL, clientWrite,
                                        (void *)&clientStruct);
  if (errorCodeClientWrite != 0) {
    printf("[CLIENT] : pthread_create() FAILED\n");
    fflush(stdout);
  }

  if ((pthread_join(clientWriteThreadID, NULL) != 0)) {
    printf("[CLIENT] : pthread_join() FAILED\n");
    fflush(stdout);

    return kThreadCreateFailed;
  }
  if ((pthread_join(clientReadThreadID, NULL) != 0)) {
    printf("[CLIENT] : pthread_join() FAILED\n");
    fflush(stdout);

    return kThreadCreateFailed;
  }

  return kThreadCreateSuccessful;
}

// FUNCTION: closeServerSocket()
// DESCRIPTION: This function closes the server socket if the programs
// is unsuccessful at joining the server at any point, or if the user enters
// >>bye<< which disconnected the client from the server PARAMETERS : void
// RETURNS : int: kSocketClosed indicating that the socket was closed.

int closeServerSocket(int *my_server_socket, int *whichClient) {
  close(*my_server_socket);
  return kSocketClosed;
}

// FUNCTION: clientRead()
// DESCRIPTION: Using the struct passed to this thread, this function is
// designed to read from the structs buffer. This buffer is then passed to the
// server socket PARAMETERS : void*clientStuct the struct that contains the
// details of the client, the messages that the client sends and recieves
// RETURNS : void*

void *clientRead(void *clientStruct) {

  int zeroLengthMessages = 0;

  clientInfo clientInfoRead = {0};

  clientInfoRead = (*(clientInfo *)clientStruct);

  /* create the input window */
  msg_win = create_newwin(msg_height, msg_width, msg_starty, msg_startx);
  scrollok(msg_win, TRUE);

  int msgCurserPositionY = 0;
  int msgCurserPositionX = 1;

  while (1) {
    char formattedName[BUFSIZ] = {'\0'};

    memset(clientInfoRead.readBuffer, 0, BUFSIZ);
    bzero(clientInfoRead.readBuffer, BUFSIZ);

    if (read(clientInfoRead.socketID, clientInfoRead.readBuffer, BUFSIZ) ==
        -1) {
      break;

    } else {

      int findLength = strlen(clientInfoRead.readBuffer);

      if (findLength == 0) {
        if (zeroLengthMessages == kMax0LengthMsg) {
          refresh();
          destroy_win(chat_win);
          destroy_win(msg_win);
          endwin();
          pthread_cancel(clientWriteThreadID);
          pthread_exit(0);
        }
      } else {
        zeroLengthMessages = 0;
      }

      if (findLength != 0) {
        // check if it is the clients own name
        sprintf(formattedName, "[%-5s ]", clientInfoRead.clientName);
        if (strstr(clientInfoRead.readBuffer, formattedName) != NULL) {

          char *pointerToSymbols = &clientInfoRead.readBuffer[27];
          *pointerToSymbols = '<';
          pointerToSymbols++;
          *pointerToSymbols = '<';
        }

        // this time function was borrowed from wikibooks
        time_t currentTime;
        struct tm *ts;
        char buffer[100];

        /* Get the current time */
        currentTime = time(NULL);

        /* Format and print the time, "ddd yyyy-mm-dd hh:mm:ss zzz" */
        ts = localtime(&currentTime);
        strftime(buffer, sizeof(buffer), "%H:%M:%S", ts);

        // what we read with the time stamp
        char whatToPrint[BUFSIZ] = {'\0'};
        sprintf(whatToPrint, "%s(%s)\n", clientInfoRead.readBuffer, buffer);

        /* I had to display the text in the thread.
         * To be able to scroll, I needed to keep track of the Y position of the
         * cursor. That would have made the function non-reentrant, and since we
         * have threads, it's a bad idea. I'm keeping it inline to ensure safety
         * with threads. This display window "function" goes until the end of
         * wrefresh(msg_win);
         */

        if (msgCurserPositionY == 0) {
          blankWin(msg_win); /* make it a clean window */
        }
        if (msgCurserPositionY >= kMaxDisplayed) {
          scroll(msg_win);
          wmove(msg_win, msg_height - 2,
                msgCurserPositionX); /* position cusor at top */
          wclrtoeol(msg_win);
        } else {
          msgCurserPositionY++;
        }
        wmove(msg_win, msgCurserPositionY,
              msgCurserPositionX); /* position cusor at top */
        wprintw(msg_win, whatToPrint);
        box(msg_win, 0, 0); /* draw a box; makes the border  */
        wrefresh(msg_win);
        bzero(whatToPrint, BUFSIZ);
      }
    }
  }
  refresh(); // equivalent to fflush, but part of ncurses
  pthread_exit(0);
  return 0;
}

// FUNCTION: clientWrite()
// DESCRIPTION: Using the struct passed to this thread, this function is
// designed to write to the buffer. This buffer is then passed to the server
// socket PARAMETERS : void*clientStuct : a void pointer to the client struct
// that contains the client's details, plus the reading and writing
// buffers
// RETURNS : void*

void *clientWrite(void *clientStruct) {

  clientInfo clientInfoWrite = {0};

  clientInfoWrite = (*(clientInfo *)clientStruct);

  /* create the input window */
  chat_win = create_newwin(chat_height, chat_width, chat_starty, chat_startx);
  scrollok(chat_win, TRUE);

  /* Clear out the input Buffer */
  char buffer[81] = {'\0'};
  while (strstr(buffer, ">>bye<<") == NULL) {

    input_win(chat_win, buffer);

    int lengthOfString = 0;
    lengthOfString = strlen(buffer);

    char clientInformation[BUFSIZ] = {"\0"};
    sprintf(clientInformation, "%-15s   [%-5s ] >> ", clientInfoWrite.IPaddress,
            clientInfoWrite.clientName);

    char firstWrite[BUFSIZ] = {'\0'};
    char secondWrite[BUFSIZ] = {'\0'};
    strcat(firstWrite, clientInformation);
    strcat(secondWrite, clientInformation);

    if (lengthOfString > kUserMessageSize) {

      char firstFourtyCharacters[kInputStringSize] = {""};
      strcpy(firstFourtyCharacters, buffer);
      char *pointerOne = &firstFourtyCharacters[kUserMessageSize];
      *pointerOne = '\0';
      char adjustStringOne[BUFSIZ] = {'\0'};

      sprintf(adjustStringOne, "%-40s",
              firstFourtyCharacters); // now we have ip, name and first 40
                                      // chracters....
      strcat(firstWrite, adjustStringOne);

      strcpy(clientInfoWrite.writeBuffer, firstWrite);
      write(clientInfoWrite.socketID, clientInfoWrite.writeBuffer, BUFSIZ);

      memset(clientInfoWrite.writeBuffer, 0, BUFSIZ);

      char *pointerTwo = &buffer[kUserMessageSize];
      if (*pointerTwo != '\0') {

        char adjustStringTwo[BUFSIZ] = {'\0'};
        sprintf(adjustStringTwo, "%-40s", &buffer[kUserMessageSize]);
        strcat(secondWrite, adjustStringTwo);
        strcpy(clientInfoWrite.writeBuffer, secondWrite);
        write(clientInfoWrite.socketID, clientInfoWrite.writeBuffer, BUFSIZ);
        memset(clientInfoWrite.writeBuffer, 0, BUFSIZ);
      }

    } else {
      char adjustStringOne[BUFSIZ] = {'\0'};

      sprintf(adjustStringOne, "%-40s", buffer);
      strcat(firstWrite, adjustStringOne);
      strcpy(clientInfoWrite.writeBuffer, firstWrite);
      write(clientInfoWrite.socketID, clientInfoWrite.writeBuffer, BUFSIZ);
    }

    if (strstr(clientInfoWrite.writeBuffer, ">>bye<<") != NULL) {
      fflush(stdout);
      pthread_cancel(clientReadThreadID);
      pthread_exit(0);
      break;
    }

    memset(clientInfoWrite.writeBuffer, 0, BUFSIZ);
    bzero(clientInfoWrite.writeBuffer, BUFSIZ);

    memset(buffer, 0, BUFSIZ);
    bzero(buffer, BUFSIZ);
  }

  return 0;
}

/*
 * FUNCTION: create_newwin
 * DESCRIPTION: This function uses the parameters sent to it to create a new
 ncurses window to those specifications.
 * PROGRAMMER: Sean Clarke
 * SOURCE: ncurses sample from assignment 4
 * PARAMETERS:
                int height : The height of the window in lines
                int width : The width of the window in columns
                int starty: The start Y coordinate of the window
                int startx: The start X coordinate of the window
 * RETURNS: WINDOW* a pointer to the window that was created.
 */

WINDOW *create_newwin(int height, int width, int starty, int startx) {
  WINDOW *local_win;

  local_win =
      newwin(height, width, starty, startx); // makes window to specs I gave it
  box(local_win, 0, 0);                      /* draw a box; makes the border  */
  wmove(local_win, 1,
        1); /* position cursor at top, 1x1 to not overwrite box border */
  wrefresh(local_win);
  return local_win;
}

/*
 * FUNCTION: input_win
 * DESCRIPTION: This function takes input from the user using wgetch in a loop,
 *to get 80 characters from the user. It will then fill the array with each
 *char. PROGRAMMER: Sean Clarke SOURCE: ncurses sample from assignment 4
 * PARAMETERS:
 * WINDOW *win: a pointer to the window we will be getting
 *the input from char *word: The c-style string we will be filling with this
 *input. RETURNS:
 */

void input_win(WINDOW *win, char *word) {
  int i, ch;
  int maxrow, maxcol, row = 1, col = 0;

  blankWin(win);                 /* make it a clean window */
  getmaxyx(win, maxrow, maxcol); /* get window size */
  bzero(word, BUFSIZ);
  wmove(win, 1, 1); /* position cusor at top */
  for (i = 0; (ch = wgetch(win)) != '\n'; i++) {
    if (i < kMaxInputSize) {
      word[i] = ch;           /* '\n' not copied */
      if (col++ < maxcol - 2) /* if within window */
      {
        wprintw(win, "%c", word[i]); /* display the char recv'd */
      } else                         /* last char pos reached */
      {
        col = 1;
        if (row == maxrow - 2) /* last line in the window */
        {
          scroll(win);          /* go up one line */
          row = maxrow - 2;     /* stay at the last line */
          wmove(win, row, col); /* move cursor to the beginning */
          wclrtoeol(win);       /* clear from cursor to eol */
          box(win, 0, 0);       /* draw the box again */
        } else {
          row++;
          wmove(win, row, col); /* move cursor to the beginning */
          wrefresh(win);
          wprintw(win, "%c", word[i]); /* display the char recv'd */
        }
      }
    }
  }
}

/*
 * FUNCTION: destroy_win
 * DESCRIPTION: This function deletes a window passed into it.
 * PROGRAMMER: Sean Clarke
 * SOURCE: ncurses sample from assignment 4
 * PARAMETERS: WINDOW *win: the window we want to delete
 * RETURNS: NONE
 */
void destroy_win(WINDOW *win) { delwin(win); } /* destory_win */

/*
 * FUNCTION: blankWin
 * DESCRIPTION: This method rolls through all of the lines in a window and
 * clears each line PROGRAMMER: Sean Clarke SOURCE: ncurses sample from
 * assignment 4 PARAMETERS: WINDOW *win: A pointer to the window we wish to
 * clear RETURNS: NONE
 */

void blankWin(WINDOW *win) {
  int i;
  int maxrow, maxcol;

  getmaxyx(win, maxrow, maxcol);
  for (i = 1; i < maxcol - 2; i++) {
    wmove(win, i, 1);
    refresh();
    wclrtoeol(win);
    wrefresh(win);
  }
  box(win, 0, 0); /* draw the box again */
  wrefresh(win);
} /* blankWin */
