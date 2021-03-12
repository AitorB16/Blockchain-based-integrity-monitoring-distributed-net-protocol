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

#include "../../include/rapidxml-1.13/rapidxml.hpp"
#include "../../include/rapidxml-1.13/rapidxml_utils.hpp"

#include "crypto.hpp"
#include "simpleNode.hpp"
#include "selfNode.hpp"

#define RANDOM_STR_LEN 128
#define DEFAULT_SELECT_WAIT 30


class network
{

private:
    int otherNodeNumber;
    int trustedNodeNumber;
    selfNode *self;
    std::list<simpleNode *> otherNodes;
    std::list<std::string> receivedMsgs;
    
    //set of socket descriptors and control
    fd_set readfds;
    int maxFD;
    // std::list<fd_set> fdsList;



    static network *instance;
    network();

public:
    static network *getInstance();
    selfNode *getSelfNode();
    simpleNode *getNode(int ID);
    int getID();
    void setID(int ID);
    int getNodeNumber();
    int getTrustedNodeNumber();
    bool imTrusted();
    bool isTrsuted(int ID);
    int getMaxFD();
    fd_set getSetOfSockets();
    void insertInReceivedMsgs(string s);
    void isMsgRepeated(string s, bool *isRepeated);
    void printOtherNodes();
    // void initializeServer();
    void connectToAllNodes();
    void connectToNode(int ID);
    void reassembleSocket(int ID);
    void reassembleAllSockets();
    void sendString(int code, int destID, int sourceID, string content = "");
    void sendStringToAll(int code, int sourceID, string content = "");
    int waitResponses(int resNum);
    void recvString(int ID, const char *servResponse);
    // void sendPackage();sendPackage Google buffer, XML
};

#endif