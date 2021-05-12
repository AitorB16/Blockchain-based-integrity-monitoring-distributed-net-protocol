#ifndef BASENODE_HPP
#define BASENODE_HPP

#include <list>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cryptopp/rsa.h>

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <pthread.h>

#include <sys/types.h>

#include "utils.hpp"
#include "crypto.hpp"
#include "globals.hpp"

class baseNode
{
protected:
    int ID;
    int sock;
    char *IP;
    struct sockaddr_in addr;
    int port;
    bool changeFlag;
    CryptoPP::RSA::PublicKey pub;
    list<string> hashRecord;
    list<string> nodeBChain;

    //Mutex
    pthread_mutex_t lockChangeFlag;
    pthread_mutex_t lockHashRecord;
    pthread_mutex_t lockNodeBChain;

public:
    baseNode();
    baseNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub);
    int getID();
    void setID(int ID);
    bool getChangeFlag();
    void setChangeFlag(bool flagValue);
    int getSock();
    sockaddr_in getAddr();
    string getLastHash();
    void updateHashList(string hash);
    void printHashList();
    string getLastNodeBChain();
    void updateNodeBChain(string hash);
    void printNodeBchain();
};

#endif