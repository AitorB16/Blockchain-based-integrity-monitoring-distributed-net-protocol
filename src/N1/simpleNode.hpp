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


#define TRUST_LEVEL 5
#define DEFAULT_DECREASE_CNT 1

class simpleNode
{
protected:
    int ID;
    int sock;
    const char *IP;
    struct sockaddr_in addr;
    int port;
    int syncNum;
    int trustLvl;
    bool changeFlag;
    bool connected;
    CryptoPP::RSA::PublicKey pub;
    // std::string currentHash;
    list<string> hashRecord;
    list<string> conflictiveHashRecord;

    //Mutex
    pthread_mutex_t lockChangeFlag;
    pthread_mutex_t lockConnected;
    pthread_mutex_t lockTrustLvl;
    pthread_mutex_t lockSyncNum;
    pthread_mutex_t lockHashRecord;
    pthread_mutex_t lockConfHashRecord;

public:
    simpleNode();
    simpleNode(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub);
    int getID();
    void setID(int ID);
    bool getChangeFlag();
    void setChangeFlag(bool flagValue);
    int getSock();
    sockaddr_in getAddr();
    string getLastHash();
    void updateHashList(string hash);
    bool isHashRepeated(string hash);
    void printHashList();
    string getLastConflictiveHash();
    void updateConflictiveHashList(string hash);
    bool isConflictiveHashRepeated(string hash);
    void printConflictiveHashList();
    int getSyncNum();
    void incrementSyncNum();
    bool isTrusted();
    int getTrustLvl();
    void decreaseTrustLvlIn(int sub);
    void resetTrustLvl();
    bool isConnected();
    void createClientSocket();
    int estConnection();
    int sendString(const char *buffer);
    string recvString();
};

#endif