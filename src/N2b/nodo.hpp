#ifndef NODO_HPP
#define NODE_HPP

#include <list>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cryptopp/rsa.h>

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>

#include <fcntl.h>  // for open
#include <unistd.h> // for close
#include <pthread.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "utils.hpp"
#include "crypto.hpp"

// char client_message[2000];
// char buffer1[1024];

// char buffer2[1024];
// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

class nodo
{
private:
    int ID;
    int sock;
    const char *IP;
    struct sockaddr_in addr;
    int port;
    bool trusted;
    CryptoPP::RSA::PublicKey pub;
    CryptoPP::RSA::PublicKey prv;
    std::list<std::string> receivedMsgs;
    std::string currentHash;

public:
    nodo();
    nodo(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub, CryptoPP::RSA::PublicKey prv);
    int getID();
    void setID(int ID);
    sockaddr_in getAddr();
    void insertInReceiveMsgs(string s);
    bool imTrusted();
    bool isMsgRepeated(string s);
    void createClientSocket();
    void createServerSocket();
    int serverUP(int max_c);
    // void *socketThread(void *arg);
    int estConnection();
    int sendString(const char *codigo);
    int recvString(const char *servResponse);

};

#endif