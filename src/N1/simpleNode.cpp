#include "simpleNode.hpp"
using namespace std;

// pthread_mutex_t lockChangeFlag = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockConnected = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockTrustLvl = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockSyncNum = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockSyncNum = PTHREAD_MUTEX_INITIALIZER;

simpleNode::simpleNode(){

};
simpleNode::simpleNode(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub)
{
    simpleNode::ID = ID;
    simpleNode::sock = 0;
    simpleNode::IP = ip;
    simpleNode::port = port;
    simpleNode::syncNum = 0;
    simpleNode::trustLvl = TRUST_LEVEL;
    simpleNode::changeFlag = false;
    simpleNode::connected = false;
    simpleNode::pub = pub;
    simpleNode::hashRecord.push_front("c54008913c9085a5a5b322e1f8eb050b843874c5d00811e1bfba2e9bbbb15a4b");

    //init mutextes
    pthread_mutex_init(&lockChangeFlag, NULL);
    pthread_mutex_init(&lockConnected, NULL);
    pthread_mutex_init(&lockTrustLvl, NULL);
    pthread_mutex_init(&lockSyncNum, NULL);
    pthread_mutex_init(&lockHashRecord, NULL);
    pthread_mutex_init(&lockConfHashRecord, NULL);
}

int simpleNode::getID()
{
    return simpleNode::ID;
}

void simpleNode::setID(int ID)
{
    simpleNode::ID = ID;
}
bool simpleNode::getChangeFlag()
{
    bool tmpChangeFlag;

    pthread_mutex_lock(&lockChangeFlag);
    tmpChangeFlag = simpleNode::changeFlag;
    pthread_mutex_unlock(&lockChangeFlag);
    return tmpChangeFlag;
}
void simpleNode::setChangeFlag(bool flagValue)
{
    pthread_mutex_lock(&lockChangeFlag);
    simpleNode::changeFlag = flagValue;
    pthread_mutex_unlock(&lockChangeFlag);
}
int simpleNode::getSock()
{
    return simpleNode::sock;
}

sockaddr_in simpleNode::getAddr()
{
    return simpleNode::addr;
}

string simpleNode::getLastHash()
{
    string hash;
    pthread_mutex_lock(&lockHashRecord);
    hash = hashRecord.front();
    pthread_mutex_unlock(&lockHashRecord);
    return hash;
}

void simpleNode::updateHashList(string hash)
{
    pthread_mutex_lock(&lockHashRecord);
    hashRecord.push_front(hash);
    pthread_mutex_unlock(&lockHashRecord);
}

bool simpleNode::isHashRepeated(string hash)
{
    bool isIn = false;
    pthread_mutex_lock(&lockHashRecord);
    for (auto const &i : hashRecord)
    {
        if (i == hash)
        {
            isIn = true;
        }
    }
    pthread_mutex_unlock(&lockHashRecord);
    return isIn;
}
void simpleNode::printHashList()
{
    //mutex?
    for (auto const &i : hashRecord)
    {
        cout << "* " << i << endl;
    }
}
string simpleNode::getLastConflictiveHash()
{
    string hash;
    pthread_mutex_lock(&lockConfHashRecord);
    hash = conflictiveHashRecord.front();
    pthread_mutex_unlock(&lockConfHashRecord);
    return hash;
}

void simpleNode::updateConflictiveHashList(string hash)
{
    pthread_mutex_lock(&lockConfHashRecord);
    conflictiveHashRecord.push_front(hash);
    pthread_mutex_unlock(&lockConfHashRecord);
}
bool simpleNode::isConflictiveHashRepeated(string hash)
{
    bool isIn = false;
    pthread_mutex_lock(&lockConfHashRecord);
    for (auto const &i : conflictiveHashRecord)
    {
        if (i == hash)
        {
            isIn = true;
        }
    }
    pthread_mutex_unlock(&lockConfHashRecord);
    return isIn;
}

void simpleNode::printConflictiveHashList()
{
    // mutex?
    for (auto const &i : conflictiveHashRecord)
    {
        cout << "* " << i << endl;
    }
}

int simpleNode::getSyncNum()
{
    int tmpSyncNum;
    pthread_mutex_lock(&lockSyncNum);
    tmpSyncNum = syncNum;
    pthread_mutex_unlock(&lockSyncNum);
    return tmpSyncNum;
}
void simpleNode::incrementSyncNum()
{
    pthread_mutex_lock(&lockSyncNum);
    syncNum++;
    pthread_mutex_unlock(&lockSyncNum);
}

bool simpleNode::isTrusted()
{
    int tmpTrustLvl;
    pthread_mutex_lock(&lockTrustLvl);
    tmpTrustLvl = trustLvl;
    pthread_mutex_unlock(&lockTrustLvl);
    if (tmpTrustLvl > 0)
        return true;
    else
        return false;
}

int simpleNode::getTrustLvl()
{
    int tmpTrustLvl;
    pthread_mutex_lock(&lockTrustLvl);
    tmpTrustLvl = trustLvl;
    pthread_mutex_unlock(&lockTrustLvl);
    return tmpTrustLvl;
}

void simpleNode::decreaseTrustLvlIn(int sub)
{
    pthread_mutex_lock(&lockTrustLvl);
    trustLvl -= sub;
    pthread_mutex_unlock(&lockTrustLvl);
}

void simpleNode::resetTrustLvl()
{
    pthread_mutex_lock(&lockTrustLvl);
    //only if trusted
    if (trustLvl > 0 && trustLvl < TRUST_LEVEL)
        trustLvl++;
    pthread_mutex_unlock(&lockTrustLvl);
}

bool simpleNode::isConnected()
{
    bool tmpConnected = false;
    pthread_mutex_lock(&lockConnected);
    tmpConnected = connected;
    pthread_mutex_unlock(&lockConnected);
    return tmpConnected;
}

void simpleNode::createClientSocket()
{
    if (sock > 0)
        close(sock);
    if ((simpleNode::sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw std::invalid_argument("Socket creation error");
    }
    memset(&addr, '0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    //Set connected to false
    pthread_mutex_lock(&lockConnected);
    connected = false;
    pthread_mutex_unlock(&lockConnected);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, simpleNode::IP, &addr.sin_addr) <= 0)
    {
        throw std::invalid_argument("Invalid address/ Address not supported");
    }
}

int simpleNode::estConnection()
{
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        return -1;
    }
    else
    {
        pthread_mutex_lock(&lockConnected);
        connected = true;
        pthread_mutex_unlock(&lockConnected);
        return 0;
    }
}

int simpleNode::sendString(const char *buffer)
{
    //Igual no necesario -1
    bool tmpConnected;
    pthread_mutex_lock(&lockConnected);
    tmpConnected = connected;
    pthread_mutex_unlock(&lockConnected);
    if (tmpConnected)
    {
        return send(sock, buffer, strlen(buffer), 0);
    }
    else
    {
        return -1;
    }
}

string simpleNode::recvString()
{
    char buffer[4096];

    bzero(buffer, 4096);

    //SELECT
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    ;
    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(sock, &fdSet);

    if (0 < select(sock + 1, &fdSet, NULL, NULL, &tv))
    {
        if (recv(sock, buffer, 4096, 0) < 0)
        {
            // printf("[-]Error in receiving data.\n");
            throw exception();
        }
        else
        {
            return buffer;
        }
    }
    else
    {
        throw exception();
    }
    // else
    // {
    //     return "ERR";
    // }
}