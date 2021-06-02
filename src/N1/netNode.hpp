#ifndef NETNODE_HPP
#define NETNODE_HPP

#include "baseNode.hpp"

class netNode : public baseNode
{
private:
    int syncNum;
    int trustLvl;
    int incidenceNum;
    bool connected;
    list<string> conflictiveHashRecord;

    //Mutex
    // pthread_mutex_t lockConnected;
    pthread_mutex_t lockTrustLvl;
    pthread_mutex_t lockIncidenceNum;
    pthread_mutex_t lockSyncNum;
    pthread_mutex_t lockConfHashRecord;

public:
    netNode();
    netNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub);
    string getLastConflictiveHash();
    void updateConflictiveHashList(string hash);
    void printConflictiveHashList();
    int getSyncNum();
    void setSyncNum(int num);
    bool isTrusted();
    int getTrustLvl();
    void setTrustLvl(int trl);
    void decreaseTrustLvlIn(int sub);
    void resetTrustLvl();
    int getIncidenceNum();
    void setIncidenceNum(int iN);
    void increaseIncidenceNum(int sum);
    void resetIncidenceNum();
    bool isConnected();
    void createClientSocket();
    void resetClientSocket();
    int estConnection();
    int sendString(const char *buffer);
    string recvString();
};

#endif