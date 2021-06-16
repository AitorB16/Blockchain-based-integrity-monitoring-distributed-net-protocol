#ifndef NETNODE_HPP
#define NETNODE_HPP

#include "baseNode.hpp"

class netNode : public baseNode
{
private:
    int syncNum;
    int trustLvl;
    int incidentNum;
    bool connected;
    list<string> troublesomeHashRecord;

    /*Mutexes*/
    pthread_mutex_t lockTrustLvl;
    pthread_mutex_t lockincidentNum;
    pthread_mutex_t lockSyncNum;
    pthread_mutex_t lockTroubHashRecord;

public:
    netNode();
    netNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub);
    int getSyncNum();
    void setSyncNum(int num);
    int getTrustLvl();
    void setTrustLvl(int trl);
    bool isTrusted();
    void decreaseTrustLvlIn(int sub);
    int getincidentNum();
    void setincidentNum(int iN);
    void increaseIncidentNum(int sum);
    void resetincidentNum();
    bool isConnected();
    int estConnection();
    string getLastTroublesomeHash();
    void updateTroublesomeHashList(string hash);
    void printTroublesomeHashList();
    void createClientSocket();
    void resetClientSocket();
    int sendString(const char *buffer);
    string recvString();
};

#endif