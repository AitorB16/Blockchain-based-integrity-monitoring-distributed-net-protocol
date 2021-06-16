#include "auditor.hpp"

/* Reset incident thread */
void *resetincidentsThread(void *arg)
{

    network *net = net->getInstance();
    sleep(TIME_RESET_INCIDENTS);
    net->resetincidentNum();
    pthread_exit(NULL);
}

/* Default constructor */
auditor::auditor(){

};

/* Custom constructor */
auditor::auditor(network *selfNetwork)
{
    auditor::selfNetwork = selfNetwork;
};

/* Auditor launch function */
int auditor::auditorUP()
{
    int auditedID;

    pthread_t incidentThread;

    if (pthread_create(&incidentThread, NULL, resetincidentsThread, NULL) != 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Aud - Error creating reset trustlvl thread" << endl;
        Logger("Aud - Error creating reset trustlvl thread");
        exit(1);
    }

    cout << "Aud - Auditor UP" << endl;
    Logger("Aud - Auditor UP");

    while (1)
    {
        /* Update trusted node number */
        selfNetwork->updateTrustedNodeNumber();

        /* Sleep default time until next audition */
        sleep(AUDITOR_INTERVAL);

        /* If changeflag active, pause auditor */
        if (!selfNetwork->getSelfNode()->getChangeFlag())
        {

            /* If network not trusted, kill thread */
            if (selfNetwork->isNetworkCompromised())
            {
                cout << "Aud - STOPPING AUDITOR, NETWORK COMPROMISED" << endl;
                Logger("Aud - STOPPING AUDITOR, NETWORK COMPROMISED");
                return 1;
            }

            auditedID = selfNetwork->getTrustedRandomNode();

            /* If not enough nodes are trusted */
            if (auditedID == -1)
            {
                selfNetwork->setNetworkToCompromised();
                cout << "Aud - STOPPING AUDITOR, NO TRUSTED NODES LEFT; NETWORK COMPROMISED" << endl;
                Logger("Aud - STOPPING AUDITOR, NO TRUSTED NODES LEFT; NETWORK COMPROMISED");
                /* End auditor */
                return 1;
            }
            auditNode(auditedID);
        }
    }
    return 0;
}

/* Audit node method */
void auditor::auditNode(int auditedID)
{
    int msgCode, audID, selfID, syncNumReceived, numRes;

    string MsgToVerify, MsgSignature, content, msg;
    bool msgValid;
    vector<string> vs;
    netNode *auditedNode;

    msgValid = false;

    auditedNode = selfNetwork->getNode(auditedID);

    if (selfNetwork->connectToNode(auditedID))
    {
        if (selfNetwork->sendString(2, auditedID, selfNetwork->getSelfNode()->getID()))
        {
            try
            {
                msg = auditedNode->recvString();

                vs = splitBuffer(msg.c_str());

                splitVectString(vs, msgCode, audID, selfID, syncNumReceived, content, MsgToVerify, MsgSignature);
                msgValid = selfNetwork->validateMsg(selfID, audID, syncNumReceived, MsgToVerify, MsgSignature);

                /* Msg not valid */
                if (!msgValid)
                {

                    /* Attempt of message falsification */
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Aud - Message was faked" << endl;
                    Logger("Aud - Message was faked");
                    /* Decrease confidence in node */
                    auditedNode->increaseIncidentNum(INCIDENT_INCREASE);
                    selfNetwork->reassembleSocket(auditedID);
                }
                /* Valid data is received */
                else
                {
                    selfNetwork->reassembleSocket(auditedID);

                    /* If content is valid and eq to latest local hash */
                    if (content == auditedNode->getLastHash())
                    {
                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Aud - " << auditedID << " is OK" << endl;
                    }
                    else
                    {
                        selfNetwork->connectToAllNodes();

                        /* Send just to connected nodes, double encapsulated datagram */
                        msg = MsgToVerify + ";" + MsgSignature;

                        selfNetwork->sendStringToAll(3, selfNetwork->getSelfNode()->getID(), msg);

                        /* Wait 2/3 of network to send OK Select; timeout RESPONSE_DELAY_MAX sec */
                        numRes = selfNetwork->waitResponses(selfNetwork->getNetNodeNumber() * THRESHOLD, RESPONSE_DELAY_MAX);

                        /* The network answers if hash value is valid and last. */
                        if (numRes >= selfNetwork->getNetNodeNumber() * THRESHOLD)
                        {
                            if (EXEC_MODE == DEBUG_MODE)
                                cout << "Aud - Hash corrected" << endl;
                            Logger("Aud - Hash corrected - Hash: " + content + " ID: " + to_string(audID));
                            auditedNode->updateHashList(content);
                            auditedNode->updateNodeBChain(content);
                        }
                        /* The network doesnt answer if value doesnt need to be updated */
                        else
                        {
                            /* Insert troublesome hash in list */
                            if (auditedNode->getLastTroublesomeHash() != content)
                            {
                                auditedNode->updateTroublesomeHashList(content);
                                auditedNode->updateNodeBChain(content);
                                if (EXEC_MODE == DEBUG_MODE)
                                    cout << "Aud - Last troublesome Hash updated - ID: " << auditedID << " Hash: " << content;
                                Logger("Aud  - Last troublesome Hash updated - ID: " + to_string(auditedID) + " Hash: " + content);
                            }
                            else
                            {
                                if (EXEC_MODE == DEBUG_MODE)
                                    cout << "Aud - Last troublesome hash values eq to blamed - ID: " << auditedID << " Hash: " << content << endl;
                                Logger("Aud - Last troublesome hash values eq to blamed - ID: " + to_string(auditedID) + " Hash: " + content);
                            }
                            if (EXEC_MODE == DEBUG_MODE)
                                cout << "Aud - Blame to - ID: " << auditedID << " ended successfully" << endl;
                            Logger("Aud - Blame to - ID: " + to_string(auditedID) + " ended successfully");
                            auditedNode->decreaseTrustLvlIn(TRUST_DECREASE);
                        }

                        /* Close connection */
                        selfNetwork->reassembleAllSockets();
                    }
                }
            }
            /* Catch error from recv */
            catch (const std::exception &e)
            {

                auditedNode->increaseIncidentNum(INCIDENT_INCREASE);

                selfNetwork->reassembleSocket(auditedID);
                if (EXEC_MODE == DEBUG_MODE)
                    cerr << "Aud - Node doesnt trust me - ID: " << auditedID << " SyncNum: " << auditedNode->getSyncNum() << endl;
                Logger("Aud - Node doesnt trust me - ID: " + to_string(auditedID) + " SyncNum: " + to_string(auditedNode->getSyncNum()));
            }
        }
    }
    /* Error connecting to audited node */
    else
    {
        auditedNode->increaseIncidentNum(INCIDENT_INCREASE);
    }
}
