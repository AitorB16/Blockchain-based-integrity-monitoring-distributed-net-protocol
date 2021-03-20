#include "simpleNode.hpp"
using namespace std;

pthread_mutex_t lockChangeFlag = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockConnected = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t lockSyncNum = PTHREAD_MUTEX_INITIALIZER;

simpleNode::simpleNode(){

};
simpleNode::simpleNode(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub)
{
    simpleNode::ID = ID;
    simpleNode::sock = 0;
    simpleNode::IP = ip;
    simpleNode::port = port;
    simpleNode::SyncNum = 0;
    simpleNode::trusted = true;
    simpleNode::changeFlag = false;
    simpleNode::connected = false;
    simpleNode::pub = pub;
    // simpleNode::hash_record.push_back(hash_record);
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
string simpleNode::getCurrentHash()
{
    return currentHash;
}
void simpleNode::setCurrentHash(string hash)
{
    currentHash = hash;
}
int simpleNode::getSyncNum()
{
    return SyncNum;
}
void simpleNode::incrementSyncNum()
{
    SyncNum++;
}

bool simpleNode::isTrusted()
{
    return simpleNode::trusted;
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

int simpleNode::sendString(const char *codigo)
{
    bool tmpConnected;
    pthread_mutex_lock(&lockConnected);
    tmpConnected = connected;
    pthread_mutex_unlock(&lockConnected);
    if (tmpConnected)
    {
        return send(sock, codigo, strlen(codigo), 0);
    }
    else
    {
        return -1;
    }
}

int simpleNode::recvString()
{
    char buffer[1024];

    if (recv(sock, buffer, 1024, 0) < 0)
    {
        printf("[-]Error in receiving data.\n");
        return -1;
    }
    else
    {
        if (strcmp(buffer, ":exit") == 0)
        {
            close(sock);
            cout << "disconnected from server" << endl;
        }
        else
        {
            cout << buffer << endl;
            bzero(buffer, 1024);
        }
        return 0;
    }
}