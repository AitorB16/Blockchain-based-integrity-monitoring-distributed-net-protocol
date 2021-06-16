#include "baseNode.hpp"
using namespace std;

/* Default constructor */
baseNode::baseNode(){

};

/* Custom constructor */
baseNode::baseNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub)
{
    baseNode::ID = ID;
    baseNode::sock = 0;
    baseNode::IP = ip;
    baseNode::port = port;
    baseNode::changeFlag = false;
    baseNode::pub = pub;
    baseNode::hashRecord.push_back(FIRST_HASH_SEQ);
    baseNode::nodeBChain.push_back(FIRST_HASH);
    
    /* init mutexes */
    pthread_mutex_init(&lockChangeFlag, NULL);
    pthread_mutex_init(&lockHashRecord, NULL);
    pthread_mutex_init(&lockNodeBChain, NULL);
}

/* Get Node ID */
int baseNode::getID()
{
    return baseNode::ID;
}

/* Set node ID */
void baseNode::setID(int ID)
{
    baseNode::ID = ID;
}

/* Get Node socket */
int baseNode::getSock()
{
    return baseNode::sock;
}

/* Get Node IP address */
sockaddr_in baseNode::getAddr()
{
    return baseNode::addr;
}

/* Get ChangeFlag status */
bool baseNode::getChangeFlag()
{
    bool tmpChangeFlag;

    pthread_mutex_lock(&lockChangeFlag);
    tmpChangeFlag = baseNode::changeFlag;
    pthread_mutex_unlock(&lockChangeFlag);
    return tmpChangeFlag;
}

/* Set changeFlag status */
void baseNode::setChangeFlag(bool flagValue)
{
    pthread_mutex_lock(&lockChangeFlag);
    baseNode::changeFlag = flagValue;
    pthread_mutex_unlock(&lockChangeFlag);
}

/* Get Node latest good hash */
string baseNode::getLastHash()
{
    string hash;
    pthread_mutex_lock(&lockHashRecord);
    hash = hashRecord.back();
    pthread_mutex_unlock(&lockHashRecord);
    return splitBuffer(hash.c_str()).at(0);
}

/* Insert new hash into LIFO hashRecord */
void baseNode::updateHashList(string hash)
{
    pthread_mutex_lock(&lockHashRecord);
    hashRecord.push_back(hash + "; seq - " + to_string(nodeBChain.size()));
    pthread_mutex_unlock(&lockHashRecord);
}

/* Print hashRecord */
void baseNode::printHashList()
{
    for (auto &i : hashRecord)
    {
        cout << "* " << i << endl;
    }
}

/* Get latest blockchain hash */
string baseNode::getLastNodeBChain()
{
    string hash;
    pthread_mutex_lock(&lockNodeBChain);
    hash = nodeBChain.back();
    pthread_mutex_unlock(&lockNodeBChain);
    return hash;
}

/* Generate a new blockchain hash from input hash */
void baseNode::updateNodeBChain(string hash)
{
    string prevHash;
    pthread_mutex_lock(&lockNodeBChain);
    prevHash = nodeBChain.back();
    hash = hashText(prevHash + "\n" + hash);
    nodeBChain.push_back(hash);
    pthread_mutex_unlock(&lockNodeBChain);
}

/* Print Blockchain */
void baseNode::printNodeBchain()
{
    for (auto &i : nodeBChain)
    {
        cout << "* " << i << endl;
    }
}