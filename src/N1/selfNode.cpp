#include "selfNode.hpp"

pthread_mutex_t lockFlag = PTHREAD_MUTEX_INITIALIZER;

selfNode::selfNode(){

};
selfNode::selfNode(int ID, char *ip, int port, CryptoPP::RSA::PublicKey pub, CryptoPP::RSA::PrivateKey prv)
    : baseNode(ID, ip, port, pub)
{ //Call to parent constructor
    selfNode::prv = prv;
    // selfNode::hashRecord.push_front(hash);
}

void selfNode::createServerSocket()
{
    int opt = 1;
    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        throw std::invalid_argument("Socket creation error");
    }

    // Forcefully attaching socket to the port
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        throw std::invalid_argument("Error socketopt");
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, IP, &addr.sin_addr) <= 0)
    {
        throw std::invalid_argument("Invalid address/ Address not supported self");
    }

    // Forcefully attaching socket to the port
    if (bind(sock, (struct sockaddr *)&addr,
             sizeof(addr)) < 0)
    {
        throw std::invalid_argument("Bind failed");
    }
}
