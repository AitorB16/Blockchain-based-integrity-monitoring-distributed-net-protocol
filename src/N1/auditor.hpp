#ifndef AUDITOR_HPP
#define AUDITOR_HPP

#include "network.hpp"

#define DEF_TIMER_AUDIT 5//60*5 //seconds
#define AUDITOR_SELECT_WAIT 2 //seconds
//X seconds/DEF_TIMER_AUDIT = TIMES BEIGN AUDITED PER X seconds; X/5 = <2 + TRUST_LEVEL> -> x=50
#define TIME_RESET_TRUST_LVL 30 //seconds

class auditor
{
private:
    network *selfNetwork;

public:
    auditor();
    auditor(network *selfNetwork);
    int auditorUP();
    void auditNode(int auditedID);
};

#endif
