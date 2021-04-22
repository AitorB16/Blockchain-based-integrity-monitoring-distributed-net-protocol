#include "baseNode.hpp"
using namespace std;

// pthread_mutex_t lockChangeFlag = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockConnected = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockTrustLvl = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockSyncNum = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockSyncNum = PTHREAD_MUTEX_INITIALIZER;

baseNode::baseNode(){

};
baseNode::baseNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub)
{
    baseNode::ID = ID;
    baseNode::sock = 0;
    baseNode::IP = ip;
    baseNode::port = port;
    // baseNode::syncNum = 0;
    // baseNode::trustLvl = TRUST_LEVEL;
    baseNode::changeFlag = false;
    // baseNode::connected = false;
    baseNode::pub = pub;
    baseNode::hashRecord.push_front("c54008913c9085a5a5b322e1f8eb050b843874c5d00811e1bfba2e9bbbb15a4b");

    //init mutextes
    pthread_mutex_init(&lockChangeFlag, NULL);
    // pthread_mutex_init(&lockConnected, NULL);
    // pthread_mutex_init(&lockTrustLvl, NULL);
    // pthread_mutex_init(&lockSyncNum, NULL);
    pthread_mutex_init(&lockHashRecord, NULL);
    // pthread_mutex_init(&lockConfHashRecord, NULL);
}

int baseNode::getID()
{
    return baseNode::ID;
}

void baseNode::setID(int ID)
{
    baseNode::ID = ID;
}
bool baseNode::getChangeFlag()
{
    bool tmpChangeFlag;

    pthread_mutex_lock(&lockChangeFlag);
    tmpChangeFlag = baseNode::changeFlag;
    pthread_mutex_unlock(&lockChangeFlag);
    return tmpChangeFlag;
}
void baseNode::setChangeFlag(bool flagValue)
{
    pthread_mutex_lock(&lockChangeFlag);
    baseNode::changeFlag = flagValue;
    pthread_mutex_unlock(&lockChangeFlag);
}
int baseNode::getSock()
{
    return baseNode::sock;
}

sockaddr_in baseNode::getAddr()
{
    return baseNode::addr;
}

string baseNode::getLastHash()
{
    string hash;
    pthread_mutex_lock(&lockHashRecord);
    hash = hashRecord.front();
    pthread_mutex_unlock(&lockHashRecord);
    return hash;
}

void baseNode::updateHashList(string hash)
{
    pthread_mutex_lock(&lockHashRecord);
    hashRecord.push_front(hash);
    pthread_mutex_unlock(&lockHashRecord);
}

bool baseNode::isHashRepeated(string hash)
{
    bool isIn = false;
    pthread_mutex_lock(&lockHashRecord);
    for (auto &i : hashRecord)
    {
        if (i == hash)
        {
            isIn = true;
        }
    }
    pthread_mutex_unlock(&lockHashRecord);
    return isIn;
}
void baseNode::printHashList()
{
    //mutex?
    for (auto &i : hashRecord)
    {
        cout << "* " << i << endl;
    }
}