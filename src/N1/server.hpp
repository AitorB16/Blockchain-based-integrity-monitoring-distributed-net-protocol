#ifndef SERVER_HPP
#define SERVER_HPP

#include "network.hpp"
#include "utils.hpp"

class server
{
private:
    network *selfNetwork;

public: 
    server();
    server(network *selfNetwork);
    int serverUP();
};

#endif