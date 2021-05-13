#include "baseNode.hpp"
using namespace std;

baseNode::baseNode(){

};
baseNode::baseNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub)
{
    baseNode::ID = ID;
    baseNode::sock = 0;
    baseNode::IP = ip;
    baseNode::port = port;
    baseNode::changeFlag = false;
    baseNode::pub = pub;
    baseNode::hashRecord.push_back(FIRST_HASH_SEC);
    baseNode::nodeBChain.push_back(FIRST_HASH );
    //init mutextes
    pthread_mutex_init(&lockChangeFlag, NULL);
    pthread_mutex_init(&lockHashRecord, NULL);
    pthread_mutex_init(&lockNodeBChain, NULL);
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
    hash = hashRecord.back();
    pthread_mutex_unlock(&lockHashRecord);
    return splitBuffer(hash.c_str()).at(0);
}

void baseNode::updateHashList(string hash)
{
    pthread_mutex_lock(&lockHashRecord);
    hashRecord.push_back(hash + "; sec - " + to_string(nodeBChain.size()));
    pthread_mutex_unlock(&lockHashRecord);
}

void baseNode::printHashList()
{
    //mutex?
    for (auto &i : hashRecord)
    {
        cout << "* " << i << endl;
    }
}

string baseNode::getLastNodeBChain()
{
    string hash;
    pthread_mutex_lock(&lockNodeBChain);
    hash = nodeBChain.back();
    pthread_mutex_unlock(&lockNodeBChain);
    return hash;
}
void baseNode::updateNodeBChain(string hash)
{
    string prevHash;
    pthread_mutex_lock(&lockNodeBChain);
    prevHash = nodeBChain.back();
    hash = stream2hex(hashText(prevHash + "\n" + hash));
    nodeBChain.push_back(hash);
    pthread_mutex_unlock(&lockNodeBChain);
}
void baseNode::printNodeBchain()
{
    //mutex?
    for (auto &i : nodeBChain)
    {
        cout << "* " << i << endl;
    }
}