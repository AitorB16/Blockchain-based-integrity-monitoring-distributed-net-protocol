CC=g++

#IDIR = ../src
# ODIR = ../obj
# LDIR = ../include

DEPS = main.hpp crypto.hpp network.hpp server.hpp linker.hpp auditor.hpp baseNode.hpp netNode.hpp selfNode.hpp utils.hpp globals.hpp

OBJ = main.o crypto.o network.o server.o linker.o auditor.o baseNode.o netNode.o selfNode.o utils.o globals.o


CFLAGS=-I .
LIBS = -g -lcryptopp -lpthread -lsystemd

%.o: %.cpp $(DEPS)
	$(CC) -std=c++1y -c -o $@ $< $(CFLAGS)

run: $(OBJ)
	$(CC) -std=c++1y -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o
