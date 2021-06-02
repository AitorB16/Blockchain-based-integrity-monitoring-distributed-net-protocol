#include "auditor.hpp"

//Reset confidence

// bool waitResponseFromAudited(int sock)
// {
//     struct timeval tv;
//     tv.tv_sec = RESPONSE_DELAY_MAX;
//     tv.tv_usec = 0;

//     int selectStatus;
//     int counter = 0;
//     fd_set FdSet;

//     FD_ZERO(&FdSet);  //clear the socket set
//     int maxFD = sock; //initialize tmpMaxFD
//     FD_SET(sock, &FdSet);

//     //just monitorize trusted sockets; low-eq maxFD descriptor
//     selectStatus = select(maxFD + 1, &FdSet, NULL, NULL, &tv);
//     //
//     counter += selectStatus;

//     //if timeout or received message number is gr eq to resNum -> break the loop
//     if (selectStatus == 0)
//         return true;

//     return false;
// }

// struct argNetwork
// {
//     network *net;
// };

void *resetIncidencesThread(void *arg)
{

    // struct argNetwork *args = (struct argNetwork *)arg;
    // network *net = args->net;

    network *net = net->getInstance();

    //Default sleep time
    // sleep(TIME_RESET_TRUST_LVL);
    sleep(TIME_RESET_INCIDENTS);
    net->resetIncidenceNum();
    pthread_exit(NULL);
}

auditor::auditor(){

};

auditor::auditor(network *selfNetwork)
{
    auditor::selfNetwork = selfNetwork;
};

int auditor::auditorUP()
{
    int auditedID;

    // argNetwork args;
    // args.net = selfNetwork;

    pthread_t incidentThread;

    if (pthread_create(&incidentThread, NULL, resetIncidencesThread, NULL) != 0)
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
        //Update trusted node number
        selfNetwork->updateTrustedNodeNumber();

        //Sleep default time until next audition
        sleep(AUDITOR_INTERVAL);

        //if changeflag active, pause auditor
        if (!selfNetwork->getSelfNode()->getChangeFlag())
        {

            //If network not trusted, kill thread
            if (selfNetwork->isNetworkComprometed())
            {
                // if (EXEC_MODE == DEBUG_MODE)
                cout << "Aud - STOPPING AUDITOR, NETWORK COMPROMETED" << endl;
                Logger("Aud - STOPPING AUDITOR, NETWORK COMPROMETED");
                return 1;
            }

            auditedID = selfNetwork->getTrustedRandomNode();
            //No enough nodes are trusted
            if (auditedID == -1)
            {
                selfNetwork->setNetworkToComprometed();
                //SET ALL nodes to no trust
                cout << "Aud - STOPPING AUDITOR, NO TRUSTED NODES LEFT; NETWORK COMPROMETED" << endl;
                Logger("Aud - STOPPING AUDITOR, NO TRUSTED NODES LEFT; NETWORK COMPROMETED");
                //End auditor
                return 1;
            }
            auditNode(auditedID);
        }
    }
    return 0;
}

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

                //msg not valid
                if (!msgValid)
                {

                    //Attempt of message falsification
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Aud - Message was faked" << endl;
                    Logger("Aud - Message was faked");
                    //Decrease confidence in node
                    // auditedNode->decreaseTrustLvlIn(TRUST_DECREASE_C2);
                    auditedNode->increaseIncidenceNum(INCIDENT_INCREASE);
                    selfNetwork->reassembleSocket(auditedID);
                }
                //Something valid is received
                else
                {
                    selfNetwork->reassembleSocket(auditedID);

                    //If content is valid and current OK
                    if (content == auditedNode->getLastHash())
                    {
                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Aud - " << auditedID << " is OK" << endl;
                        // Logger(auditedID + " is OK");
                    }
                    //If content isnt the last, no trust on current node
                    // else if (auditedNode->isHashRepeated(content))
                    // {
                    //     auditedNode->decreaseTrustLvlIn(TRUST_LEVEL);
                    // }
                    //If content is different, blame node.
                    else
                    {
                        //BCAST TO OTHER NODES
                        //Connect to ALL
                        // if (selfNetwork->connectToAllNodes())
                        // {
                        selfNetwork->connectToAllNodes();

                        //Send just to connected nodes
                        msg = MsgToVerify + ";" + MsgSignature;

                        selfNetwork->sendStringToAll(3, selfNetwork->getSelfNode()->getID(), msg);

                        //Wait 2/3 of network to send OK Select; timeout RESPONSE_DELAY_MAX sec

                        numRes = selfNetwork->waitResponses(selfNetwork->getNetNodeNumber() * THRESHOLD, RESPONSE_DELAY_MAX);

                        //The network answers if hash value is valid and last.
                        if (numRes >= selfNetwork->getNetNodeNumber() * THRESHOLD)
                        {
                            if (EXEC_MODE == DEBUG_MODE)
                                cout << "Aud - Hash corrected" << endl;
                            Logger("Aud - Hash corrected - Hash: " + content + " ID: " + to_string(audID));
                            auditedNode->updateHashList(content);
                            auditedNode->updateNodeBChain(content);
                        }
                        //The network doesnt answer if value doesnt need to be updated
                        else
                        {
                            //Insert conflictive hash in list
                            if (auditedNode->getLastConflictiveHash() != content)
                            {
                                auditedNode->updateConflictiveHashList(content);
                                auditedNode->updateNodeBChain(content);
                                if (EXEC_MODE == DEBUG_MODE)
                                    cout << "Aud - Last conflictive Hash updated - ID: " << auditedID << " Hash: " << content;
                                Logger("Aud  - Last conflictive Hash updated - ID: " + to_string(auditedID) + " Hash: " + content);
                            }
                            else
                            {
                                if (EXEC_MODE == DEBUG_MODE)
                                    cout << "Aud - Last conflictive hash values eq to blamed - ID: " << auditedID << " Hash: " << content << endl;
                                Logger("Aud - Last conflictive hash values eq to blamed - ID: " + to_string(auditedID) + " Hash: " + content);
                            }
                            if (EXEC_MODE == DEBUG_MODE)
                                cout << "Aud - Blame to - ID: " << auditedID << " ended successfully" << endl;
                            Logger("Aud - Blame to - ID: " + to_string(auditedID) + " ended successfully");
                            auditedNode->decreaseTrustLvlIn(TRUST_DECREASE);
                        }

                        //Close connection
                        selfNetwork->reassembleAllSockets();
                        // }
                        // else
                        // {
                        //     if (EXEC_MODE == DEBUG_MODE)
                        //         cout << "Aud - Network busy" << endl;
                        //     Logger("Aud - Network busy");
                        // }
                    }
                }
            }
            // }
            //Catch error from recv
            catch (const std::exception &e)
            {

                // cout << TRUST_DECREASE_C1 << " "<< TRUST_DECREASE_C2 << endl;

                auditedNode->increaseIncidenceNum(INCIDENT_INCREASE);

                // cout << auditedNode->getTrustLvl() << endl;

                // auditedNode->incrementSyncNum(); //Comment??
                selfNetwork->reassembleSocket(auditedID);
                // auditedNode->incrementSyncNum();
                if (EXEC_MODE == DEBUG_MODE)
                    cerr << "Aud - Node doesnt trust me - ID: " << auditedID << " SyncNum: " << auditedNode->getSyncNum() << endl;
                Logger("Aud - Node doesnt trust me - ID: " + to_string(auditedID) + " SyncNum: " + to_string(auditedNode->getSyncNum()));
                // pthread_exit(NULL);
            }
        }
    }
    //Error connecting to audited node
    else
    {
        // auditedNode->decreaseTrustLvlIn(TRUST_DECREASE_C2);
        auditedNode->increaseIncidenceNum(INCIDENT_INCREASE);
    }
    // pthread_exit(NULL);
}
