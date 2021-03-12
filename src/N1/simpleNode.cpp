#include "simpleNode.hpp"
using namespace std;

pthread_mutex_t lockChangeFlag = PTHREAD_MUTEX_INITIALIZER;

simpleNode::simpleNode(){

};
simpleNode::simpleNode(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub)
{
    simpleNode::ID = ID;
    simpleNode::sock = 0;
    simpleNode::IP = ip;
    simpleNode::port = port;
    simpleNode::trusted = true;
    simpleNode::changeFlag = false;
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
void simpleNode::getChangeFlag(bool *flagValue){
    pthread_mutex_lock(&lockChangeFlag);
    *flagValue = simpleNode::changeFlag;
    pthread_mutex_unlock(&lockChangeFlag);
}
void simpleNode::setChangeFlag(bool flagValue){
    pthread_mutex_lock(&lockChangeFlag);
    simpleNode::changeFlag= flagValue;
    pthread_mutex_unlock(&lockChangeFlag);
}
int simpleNode::getSock(){
    return simpleNode::sock;
}

sockaddr_in simpleNode::getAddr()
{
    return simpleNode::addr;
}

bool simpleNode::isTrusted()
{
    return simpleNode::trusted;
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
        return 0;
}

int simpleNode::sendString(const char *codigo)
{
    return send(sock, codigo, strlen(codigo), 0);
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