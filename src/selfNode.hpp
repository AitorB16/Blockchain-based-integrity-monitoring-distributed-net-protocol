#ifndef SELFNODE_HPP
#define SELFNODE_HPP

#include "baseNode.hpp"

class selfNode: public baseNode
{
private:
    CryptoPP::RSA::PrivateKey prv;

public:
    selfNode();
    selfNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub, CryptoPP::RSA::PrivateKey prv);
    void createServerSocket();
};

#endif