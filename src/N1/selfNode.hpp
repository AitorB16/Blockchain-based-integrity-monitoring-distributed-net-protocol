#ifndef SELFNODE_HPP
#define SELFNODE_HPP

#include "simpleNode.hpp"

class selfNode: public simpleNode
{
protected:
    CryptoPP::RSA::PrivateKey prv;

public:
    selfNode();
    selfNode(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub, CryptoPP::RSA::PrivateKey prv, string hash);
    void createServerSocket();
    // int sendString(const char *codigo);
    // int recvString(const char *servResponse);
    // int serverUP(int max_c);
};

#endif