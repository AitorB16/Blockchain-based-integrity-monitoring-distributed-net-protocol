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
#include "simpleNode.hpp"
#include "selfNode.hpp"

#define RANDOM_STR_LEN 16
#define NETWORK_SELECT_WAIT 30
#define THRESHOLD 2 / 3

class network
{

private:
    int otherNodeNumber;
    int trustedNodeNumber;
    bool networkComprometed;
    selfNode *self;
    std::list<simpleNode *> otherNodes;
    // std::list<std::string> receivedMsgs;

    //set of socket descriptors and control
    fd_set readfds;
    int maxFD;
    // std::list<fd_set> fdsList;

    //Mutex
    pthread_mutex_t lockTrustedNodeNumber;
    pthread_mutex_t lockNetworkComprometed;
    pthread_mutex_t lockMaxFD;

    static network *instance;
    network();

public:
    static network *getInstance();
    selfNode *getSelfNode();
    simpleNode *getNode(int ID);
    int getID();
    // void setID(int ID);
    int getNodeNumber();
    int getTrustedNodeNumber();
    void updateTrustedNodeNumber();
    bool imTrusted();
    // bool isTrsuted(int ID);
    // int trustLvl(int ID);
    bool isNetworkComprometed();
    void setNetworkToComprometed();
    int getMaxFD();
    void setMaxFD(int fd);
    fd_set getSetOfSockets();
    // void insertInReceivedMsgs(string s);
    // bool isMsgRepeated(string s);
    void printNetwork();
    // void initializeServer();
    bool connectToAllNodes();
    bool connectToNode(int ID);
    void reassembleSocket(int ID);
    void reassembleAllSockets();
    bool validateMsg(string selfID, string clientID, int syncNumReceived, string MsgToVerify, string MsgSignature);
    bool sendString(int code, int destID, int sourceID, string content = "");
    void sendStringToAll(int code, int sourceID, string content = "");
    // void sendStringToAllNotSingingContent(int code, int sourceID, string content);
    int waitResponses(int resNum, int select_time);
    int getTrustedRandomNode();
    void resetTrustLvl();
    // string recvString(int ID);
    // void sendPackage();sendPackage Google buffer, XML
};

#endif