# sudo apt install libcrypto++-dev libcrypto++-doc libcrypto++-utils

CC=g++

IDIR = ../src
# ODIR = ../obj
# LDIR = ../include

DEPS = main.hpp crypto.hpp infra.hpp nodo.hpp

OBJ = main.o crypto.o infra.o nodo.o


CFLAGS=-I .
LIBS = -lcryptopp

%.o: %.c $(DEPS)
	$(CC) -std=c++1y -c -o $@ $< $(CFLAGS)

run: $(OBJ)
	$(CC) -std=c++1y -o $@ $^ $(CFLAGS) $(LIBS)

# .PHONY: clean

#  clean:
#  	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ 
