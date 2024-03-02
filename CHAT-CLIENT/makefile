#
# this makefile will compile and and all source
# found in the ~/System Programming/CHAT-CLIENT directory.  This represents a sample
# development directory structure and project
#
# =======================================================
#              SP-Assign-04: CHAT-CLIENT
# =======================================================
# FINAL BINARY Target
./bin/chat-client : ./obj/clientMain.o ./obj/clientFunctions.o 
	cc ./obj/clientMain.o ./obj/clientFunctions.o -lncurses -o ./bin/chat-client -lpthread
#
# =======================================================
#                     Dependencies
# =======================================================
./obj/clientMain.o : ./src/clientMain.c ./inc/clientHeader.h 
	cc -c ./src/clientMain.c -o ./obj/clientMain.o

./obj/clientFunctions.o : ./src/clientFunctions.c ./inc/clientHeader.h 
	cc -c  ./src/clientFunctions.c -o ./obj/clientFunctions.o

#
# =======================================================
# Other targets
# =======================================================
all : ./bin/chat-client

clean:
	rm -f ./bin/*
	rm -f ./obj/*.o
	rm -f ./inc/*.h~
	rm -f ./src/*.c~
