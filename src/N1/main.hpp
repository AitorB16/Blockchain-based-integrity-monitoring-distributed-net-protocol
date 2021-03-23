// g++ -std=c++1y crypto.cpp -I /home/shravan40/cryptopp/build -lcryptopp

#include <iostream>
#include <stdlib.h>
#include <cryptopp/integer.h>

#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <signal.h>

#include <bitset>

// #include "crypto.hpp"
#include "network.hpp"
#include "server.hpp"
#include "auditor.hpp"
#include "crypto.hpp"
#include "utils.hpp"

#define THRESHOLD 2/3

int main();