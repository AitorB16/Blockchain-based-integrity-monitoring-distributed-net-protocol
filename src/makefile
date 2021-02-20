# sudo apt install libcrypto++-dev libcrypto++-doc libcrypto++-utils

CC=g++

IDIR = ../src
# ODIR = ../obj
# LDIR = ../lib

DEPS = main.hpp crypto.hpp

OBJ = main.o crypto.o


CFLAGS=-I .
LIBS = -lcryptopp

%.o: %.c $(DEPS)
	$(CC) -std=c++1y -c -o $@ $< $(CFLAGS)

run: $(OBJ)
	$(CC) -std=c++1y -o $@ $^ $(CFLAGS) $(LIBS)

# .PHONY: clean

#  clean:
#  	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ 
