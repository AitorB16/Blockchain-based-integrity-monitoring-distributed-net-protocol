#include "netNode.hpp"
using namespace std;

/* Default constructor */ 
netNode::netNode(){

};

/* Custom constructor using inheritance; parent baseNode */ 
netNode::netNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub)
    : baseNode(ID, ip, port, pub)
{
    netNode::syncNum = 0;
    netNode::trustLvl = TRUST_LEVEL;
    netNode::incidentNum = 0;
    netNode::connected = false;
    netNode::conflictiveHashRecord.push_back(FIRST_HASH_SEC);

    /* init mutextes */ 
    pthread_mutex_init(&lockTrustLvl, NULL);
    pthread_mutex_init(&lockincidentNum, NULL);
    pthread_mutex_init(&lockSyncNum, NULL);
    pthread_mutex_init(&lockConfHashRecord, NULL);
}

/* Get latest conflictive hash */ 
string netNode::getLastConflictiveHash()
{
    string hash;
    pthread_mutex_lock(&lockConfHashRecord);
    hash = conflictiveHashRecord.back();
    pthread_mutex_unlock(&lockConfHashRecord);
    return splitBuffer(hash.c_str()).at(0);
}

/* Insert new hash into conflictive LIFO hashRecord */ 
void netNode::updateConflictiveHashList(string hash)
{
    pthread_mutex_lock(&lockConfHashRecord);
    conflictiveHashRecord.push_back(hash + "; sec - " + to_string(nodeBChain.size()));
    pthread_mutex_unlock(&lockConfHashRecord);
}

/* Print conflictive hash record */ 
void netNode::printConflictiveHashList()
{
    for (auto &i : conflictiveHashRecord)
    {
        cout << "* " << i << endl;
    }
}

/* Get synchronization number */ 
int netNode::getSyncNum()
{
    int tmpSyncNum;
    pthread_mutex_lock(&lockSyncNum);
    tmpSyncNum = syncNum;
    pthread_mutex_unlock(&lockSyncNum);
    return tmpSyncNum;
}

/* Set synchronization number */ 
void netNode::setSyncNum(int num)
{
    pthread_mutex_lock(&lockSyncNum);
    syncNum = num;
    pthread_mutex_unlock(&lockSyncNum);
}

/* Get node trust estatus */ 
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

/* Get node trust level */ 
int netNode::getTrustLvl()
{
    int tmpTrustLvl;
    pthread_mutex_lock(&lockTrustLvl);
    tmpTrustLvl = trustLvl;
    pthread_mutex_unlock(&lockTrustLvl);
    return tmpTrustLvl;
}

/* Set node trust to trl */ 
void netNode::setTrustLvl(int trl)
{
    pthread_mutex_lock(&lockTrustLvl);
    trustLvl = trl;
    pthread_mutex_unlock(&lockTrustLvl);
}

/* Decrease trust level by sub */ 
void netNode::decreaseTrustLvlIn(int sub)
{
    pthread_mutex_lock(&lockTrustLvl);
    trustLvl -= sub;
    pthread_mutex_unlock(&lockTrustLvl);
}

/* Get incident number */  
int netNode::getincidentNum()
{
    int tmpincidentNum;
    pthread_mutex_lock(&lockincidentNum);
    tmpincidentNum = incidentNum;
    pthread_mutex_unlock(&lockincidentNum);
    return tmpincidentNum;
}

/* Set incident number to iN */  
void netNode::setincidentNum(int iN)
{
    pthread_mutex_lock(&lockincidentNum);
    incidentNum = iN;
    pthread_mutex_unlock(&lockincidentNum);
}

/* Increase incident number by sum */  
void netNode::increaseincidentNum(int sum)
{
    pthread_mutex_lock(&lockincidentNum);
    incidentNum += sum;
    if (incidentNum >= MAX_INCIDENTS)
    {
        setTrustLvl(0);
    }
    pthread_mutex_unlock(&lockincidentNum);
}

/* Reset incidents by decreasing a unit the accumulated num */  
void netNode::resetincidentNum()
{
    pthread_mutex_lock(&lockincidentNum);
    if (incidentNum > 0 && incidentNum < MAX_INCIDENTS)
    {
        incidentNum--;
    }
    pthread_mutex_unlock(&lockincidentNum);
}

/* Return connected status */  
bool netNode::isConnected()
{
    bool tmpConnected = false;
    tmpConnected = connected;
    return tmpConnected;
}

/* Create socket used wrap */  
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

/* Socket reset */  
void netNode::resetClientSocket()
{
    close(sock); /* Free resources */

    if ((netNode::sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) /* Create new socket */
    {
        throw std::invalid_argument("Socket reset error");
    }

    /* Set connected to false */
    connected = false;
}

/* Open a connection */  
int netNode::estConnection()
{
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        return -1;
    }
    else
    {
        connected = true;
        return 0;
    }
}

/* Socket send */  
int netNode::sendString(const char *buffer)
{
    return send(sock, buffer, strlen(buffer), 0);
}

/* Socket recv; ex blocking for RESPONSE_DELAY_MAX */  
string netNode::recvString()
{
    char buffer[4096];

    bzero(buffer, 4096);

    struct timeval tv;
    tv.tv_sec = RESPONSE_DELAY_MAX;
    tv.tv_usec = 0;

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(sock, &fdSet);

    /* If no responses in time or an invalid response is received, throw exception */ 
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