//network SINGLETON
#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <list>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cryptopp/rsa.h>

#include <pthread.h>

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "../../include/rapidxml-1.13/rapidxml.hpp"
#include "../../include/rapidxml-1.13/rapidxml_utils.hpp"

#include "crypto.hpp"
#include "netNode.hpp"
#include "selfNode.hpp"

class network
{

private:
    int netNodeNumber;
    int trustedNodeNumber;
    bool networkComprometed;
    selfNode *self;
    std::list<netNode *> netNodes;

    string passwdSHA256;

    //set of socket descriptors and control
    fd_set readfds;
    int maxFD;

    //Mutex
    pthread_mutex_t lockTrustedNodeNumber;
    pthread_mutex_t lockNetworkComprometed;
    pthread_mutex_t lockMaxFD;

    static network *instance;
    network();

public:
    static network *getInstance();
    selfNode *getSelfNode();
    netNode *getNode(int ID);
    int getNetNodeNumber();
    int getTrustedNodeNumber();
    void updateTrustedNodeNumber();
    bool isNetworkComprometed();
    void setNetworkToComprometed();
    bool verifyPasswd(string inPswd);
    int getMaxFD();
    void setMaxFD(int fd);
    void printNetwork();
    bool connectToAllNodes();
    bool connectToNode(int ID);
    void reassembleSocket(int ID);
    void reassembleAllSockets();
    bool validateMsg(int selfID, int clientID, int syncNumReceived, string MsgToVerify, string MsgSignature);
    bool sendString(int code, int destID, int sourceID, string content = "");
    void sendStringToAll(int code, int sourceID, string content = "");
    int waitResponses(int resNum, int select_time);
    int getTrustedRandomNode();
    void resetTrustLvl();
    // void sendPackage();sendPackage Google buffer, XML
};

#endif