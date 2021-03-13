#ifndef SIMPLENODE_HPP
#define SIMPLENODE_HPP

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

class simpleNode
{
protected:
    int ID;
    int sock;
    const char *IP;
    struct sockaddr_in addr;
    int port;
    bool trusted;
    bool changeFlag;
    bool connected;
    CryptoPP::RSA::PublicKey pub;
    std::string currentHash;

public:
    simpleNode();
    simpleNode(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub);
    int getID();
    void setID(int ID);
    bool getChangeFlag();
    void setChangeFlag(bool flagValue);
    int getSock();
    sockaddr_in getAddr();
    string getCurrentHash();
    void setCurrentHash(string hash);
    bool isTrusted();
    bool isConnected();
    void createClientSocket();
    int estConnection();
    int sendString(const char *codigo);
    int recvString();

};

#endif