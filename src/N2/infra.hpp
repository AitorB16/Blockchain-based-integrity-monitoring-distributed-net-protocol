//infra SINGLETON
#ifndef INFRA_HPP
#define INFRA_HPP

#include <list>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cryptopp/rsa.h>

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "../../include/rapidxml-1.13/rapidxml.hpp"
#include "../../include/rapidxml-1.13/rapidxml_utils.hpp"

#include "nodo.hpp"
#include "crypto.hpp"

class infra
{

private:
    int otherNodeNumber;
    int trustedNodeNumber;
    nodo *self;
    std::list<nodo *> otherNodes;

    static infra *instance;
    infra();

public:
    static infra *getInstance();
    int getID();
    void setID(int ID);
    int getTrustedNodeNumber();
    bool imTrusted();
    // bool isTrsuted(int ID);
    void printOtherNodes();
    void initializeServer();
    void connectToAllNodes();
    void connectToNode(int ID);
    void reassembleSocket(int ID);
    void reassembleAllSockets();
    void sendString(int ID, const char *msg);
    void sendStringToAll(const char *msg);
    void recvString(int ID, const char *servResponse);
    // void sendPackage();sendPackage Google buffer, XML
};

#endif