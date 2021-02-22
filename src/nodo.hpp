#ifndef NODO_HPP
#define NODE_HPP

#include <list>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cryptopp/rsa.h>

#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

class nodo
{
private:
    int ID;
    struct sockaddr_in addr;
    int port;
    bool trusted;
    CryptoPP::RSA::PublicKey pub;
    CryptoPP::RSA::PublicKey prv;
    std::list<std::string> hash_record;

public:
    nodo();
    nodo(int ID, const char* ip, int port, CryptoPP::RSA::PublicKey pub, CryptoPP::RSA::PublicKey prv, std::string hash_record);
    int getID();
};

#endif