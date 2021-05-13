#ifndef NETNODE_HPP
#define NETNODE_HPP

#include "baseNode.hpp"

class netNode : public baseNode
{
private:
    int syncNum;
    int trustLvl;
    bool changeFlag;
    bool connected;
    list<string> conflictiveHashRecord;

    //Mutex
    // pthread_mutex_t lockConnected;
    pthread_mutex_t lockTrustLvl;
    pthread_mutex_t lockSyncNum;
    pthread_mutex_t lockConfHashRecord;

public:
    netNode();
    netNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub);
    string getLastConflictiveHash();
    void updateConflictiveHashList(string hash);
    // bool isConflictiveHashRepeated(string hash);
    void printConflictiveHashList();
    int getSyncNum();
    void setSyncNum(int num);
    bool isTrusted();
    int getTrustLvl();
    void decreaseTrustLvlIn(int sub);
    void resetTrustLvl();
    bool isConnected();
    void createClientSocket();
    void resetClientSocket();
    int estConnection();
    int sendString(const char *buffer);
    string recvString();
};

#endif