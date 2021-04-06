#ifndef AUDITOR_HPP
#define AUDITOR_HPP

#include "network.hpp"

#define DEF_TIMER_AUDIT 60*5 //seconds
#define AUDITOR_SELECT_WAIT 5 //seconds

class auditor
{
private:
    network *selfNetwork;

public:
    auditor();
    auditor(network *selfNetwork);
    int auditorUP();
};

#endif