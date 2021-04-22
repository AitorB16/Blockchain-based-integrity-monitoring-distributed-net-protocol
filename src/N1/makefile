# sudo apt install libcrypto++-dev libcrypto++-doc libcrypto++-utils

CC=g++

IDIR = ../src
# ODIR = ../obj
# LDIR = ../include

DEPS = main.hpp crypto.hpp network.hpp server.hpp auditor.hpp baseNode.hpp netNode.hpp selfNode.hpp utils.hpp globals.hpp

OBJ = main.o crypto.o network.o server.o auditor.o baseNode.o netNode.o selfNode.o utils.o globals.o


CFLAGS=-I .
LIBS = -g -lcryptopp -lpthread

%.o: %.c $(DEPS)
	$(CC) -std=c++1y -c -o $@ $< $(CFLAGS)

run: $(OBJ)
	$(CC) -std=c++1y -o $@ $^ $(CFLAGS) $(LIBS)

# .PHONY: clean

#  clean:
#  	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ 
