#include "auditor.hpp"

auditor::auditor(){

};
auditor::auditor(network *selfNetwork)
{
    auditor::selfNetwork = selfNetwork;
};

int auditor::auditorUP()
{
    int auditedID;

    while (1)
    {
        // sleep(DEF_TIMER_AUDIT);

        //If network not trusted, kill thread
        //return -1;

        auditedID = selfNetwork->getTrustedRandomNode();
        selfNetwork->sendString(2, auditedID, selfNetwork->getID());

        sleep(1);
    }
    return 0;
}