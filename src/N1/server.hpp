#ifndef SERVER_HPP
#define SERVER_HPP

#include "network.hpp"
#include "utils.hpp"

#define DEF_TIMER_WAIT 30 //seconds

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