#ifndef AUDITOR_HPP
#define AUDITOR_HPP

#include "network.hpp"

#define DEF_TIMER_AUDIT 5//60*5 //seconds
#define AUDITOR_SELECT_WAIT 2 //seconds
#define TIME_RESET_TRUST_LVL 120 //seconds

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
