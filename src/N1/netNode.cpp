#include "netNode.hpp"
using namespace std;

netNode::netNode(){

};
netNode::netNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub)
    : baseNode(ID, ip, port, pub)
{
    netNode::syncNum = 0;
    netNode::trustLvl = TRUST_LEVEL;
    netNode::connected = false;

    //init mutextes
    pthread_mutex_init(&lockConnected, NULL);
    pthread_mutex_init(&lockTrustLvl, NULL);
    pthread_mutex_init(&lockSyncNum, NULL);
    pthread_mutex_init(&lockConfHashRecord, NULL);
}

string netNode::getLastConflictiveHash()
{
    string hash;
    pthread_mutex_lock(&lockConfHashRecord);
    hash = conflictiveHashRecord.front();
    pthread_mutex_unlock(&lockConfHashRecord);
    return hash;
}

void netNode::updateConflictiveHashList(string hash)
{
    pthread_mutex_lock(&lockConfHashRecord);
    conflictiveHashRecord.push_front(hash);
    pthread_mutex_unlock(&lockConfHashRecord);
}
bool netNode::isConflictiveHashRepeated(string hash)
{
    bool isIn = false;
    pthread_mutex_lock(&lockConfHashRecord);
    for (auto &i : conflictiveHashRecord)
    {
        if (i == hash)
        {
            isIn = true;
        }
    }
    pthread_mutex_unlock(&lockConfHashRecord);
    return isIn;
}

void netNode::printConflictiveHashList()
{
    // mutex?
    for (auto &i : conflictiveHashRecord)
    {
        cout << "* " << i << endl;
    }
}

int netNode::getSyncNum()
{
    int tmpSyncNum;
    pthread_mutex_lock(&lockSyncNum);
    tmpSyncNum = syncNum;
    pthread_mutex_unlock(&lockSyncNum);
    return tmpSyncNum;
}
void netNode::incrementSyncNum()
{
    pthread_mutex_lock(&lockSyncNum);
    syncNum++;
    pthread_mutex_unlock(&lockSyncNum);
}

bool netNode::isTrusted()
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

int netNode::getTrustLvl()
{
    int tmpTrustLvl;
    pthread_mutex_lock(&lockTrustLvl);
    tmpTrustLvl = trustLvl;
    pthread_mutex_unlock(&lockTrustLvl);
    return tmpTrustLvl;
}

void netNode::decreaseTrustLvlIn(int sub)
{
    pthread_mutex_lock(&lockTrustLvl);
    trustLvl -= sub;
    pthread_mutex_unlock(&lockTrustLvl);
}

void netNode::resetTrustLvl()
{
    pthread_mutex_lock(&lockTrustLvl);
    //only if trusted
    if (trustLvl > 0 && trustLvl < TRUST_LEVEL)
        trustLvl++;
    pthread_mutex_unlock(&lockTrustLvl);
}

bool netNode::isConnected()
{
    bool tmpConnected = false;
    pthread_mutex_lock(&lockConnected);
    tmpConnected = connected;
    pthread_mutex_unlock(&lockConnected);
    return tmpConnected;
}

void netNode::createClientSocket()
{
    if ((netNode::sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw std::invalid_argument("Socket creation error");
    }

    memset(&addr, '0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, IP, &addr.sin_addr) <= 0)
    {
        throw std::invalid_argument("Invalid address/ Address not supported net");
    }
}

void netNode::resetClientSocket()
{
    close(sock);

    if ((netNode::sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw std::invalid_argument("Socket creation error");
    }

    //Set connected to false
    pthread_mutex_lock(&lockConnected);
    connected = false;
    pthread_mutex_unlock(&lockConnected);
}

int netNode::estConnection()
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

int netNode::sendString(const char *buffer)
{
    // bool tmpConnected;
    // pthread_mutex_lock(&lockConnected);
    // tmpConnected = connected;
    // pthread_mutex_unlock(&lockConnected);
    // if (tmpConnected)
    // {
    return send(sock, buffer, strlen(buffer), 0);
    // }
    // else
    // {
    //     return -1;
    // }
}

string netNode::recvString()
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
}