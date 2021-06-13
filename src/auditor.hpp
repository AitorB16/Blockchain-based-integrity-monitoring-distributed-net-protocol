#ifndef AUDITOR_HPP
#define AUDITOR_HPP

#include "network.hpp"

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
