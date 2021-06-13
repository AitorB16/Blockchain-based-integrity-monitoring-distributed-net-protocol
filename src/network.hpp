#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <sys/time.h>

#include "../include/rapidxml-1.13/rapidxml.hpp"
#include "../include/rapidxml-1.13/rapidxml_utils.hpp"

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

    /*set of socket descriptors and max socket descriptor number*/
    fd_set readfds;
    int maxFD;

    /*Mutexes*/
    pthread_mutex_t lockTrustedNodeNumber;
    pthread_mutex_t lockNetworkComprometed;

    /* Static instance of network - SINGLETON */
    static network *instance;

    /* Constructor private - SINGLETON */
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
    void printNetwork();
    bool connectToAllNodes();
    bool connectToNode(int ID);
    void reassembleSocket(int ID);
    void reassembleAllSockets();
    bool validateMsg(int selfID, int clientID, int syncNumReceived, string MsgToVerify, string MsgSignature);
    bool sendString(int code, int destID, int sourceID, string content = "");
    void sendStringToAll(int code, int sourceID, string content = "");
    int waitResponses(int resNum, int selecTime);
    int getTrustedRandomNode();
    void resetincidentNum();
    void pauseAuditor();
    void resumeAuditor();
};

#endif