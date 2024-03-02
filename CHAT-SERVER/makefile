#
# this makefile will compile and and all source
# found in the ~/System Programming/CHAT-SERVER directory.  This represents a sample
# development directory structure and project
#
# =======================================================
#              SP-Assign-04: CHAT-SERVER
# =======================================================
# FINAL BINARY Target
./bin/chat-server : ./obj/serverMain.o ./obj/serverFunctions.o 
	cc ./obj/serverMain.o ./obj/serverFunctions.o -lncurses -o ./bin/chat-server -lpthread
#
# =======================================================
#                     Dependencies
# =======================================================
./obj/serverMain.o : ./src/serverMain.c ./inc/serverHeader.h 
	cc -c ./src/serverMain.c -o ./obj/serverMain.o

./obj/serverFunctions.o : ./src/serverFunctions.c ./inc/serverHeader.h 
	cc -c ./src/serverFunctions.c -o ./obj/serverFunctions.o

#
# =======================================================
# Other targets
# =======================================================
all : ./bin/chat-server

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
	rm -f ./inc/*.h~
	rm -f ./src/*.c~
