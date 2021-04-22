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

struct argNetwork
{
    network *net;
};

void *resetTrustLvlThread(void *arg)
{

    struct argNetwork *args = (struct argNetwork *)arg;
    network *net = args->net;

    //Default sleep time
    sleep(TIME_RESET_TRUST_LVL);
    net->resetTrustLvl();
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

    argNetwork args;
    args.net = selfNetwork;

    pthread_t trustThread;
    pthread_t auditResetThread;

    if (pthread_create(&trustThread, NULL, resetTrustLvlThread, (void *)&args) != 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Error creating reset trustlvl thread" << endl;
        exit(1);
    }

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
                if (EXEC_MODE == DEBUG_MODE)
                    cout << "STOPPING AUDITOR, NETWORK COMPROMETED" << endl;
                return -1;
            }

            auditedID = selfNetwork->getTrustedRandomNode();
            //No nodes are trusted
            if (auditedID == -1)
            {
                selfNetwork->setNetworkToComprometed();
                //SET ALL nodes to no trust
                if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
                    cout << "STOPPING AUDITOR, NO TRUSTED NODES LEFT; NETWORK COMPROMETED" << endl;
                //End auditor
                return -1;
            }
            auditNode(auditedID);
        }
    }
    return 0;
}

void auditor::auditNode(int auditedID)
{
    int sock, msgCode, audID, selfID, syncNumReceived, numRes;

    string MsgToVerify, MsgSignature, content, msg;
    bool msgValid;
    vector<string> vs;
    netNode *auditedNode;

    msgValid = false;

    auditedNode = selfNetwork->getNode(auditedID);

    if (selfNetwork->connectToNode(auditedID))
    {
        if (selfNetwork->sendString(2, auditedID, selfNetwork->getID()))
        {
            sock = auditedNode->getSock();
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
                        cout << "Disconnected message was faked (in auditor)" << endl;
                    //Decrease confidence in node
                    auditedNode->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
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
                            cout << auditedID << " is OK" << endl;
                    }
                    //If content isnt the last, no trust on current node
                    else if (auditedNode->isHashRepeated(content))
                    {
                        auditedNode->decreaseTrustLvlIn(TRUST_LEVEL);
                    }
                    //If content is new, blame node.
                    else
                    {
                        //BCAST TO OTHER NODES
                        //Connect to ALL
                        if (selfNetwork->connectToAllNodes())
                        {

                            //Send just to connected nodes
                            msg = MsgToVerify + ";" + MsgSignature;

                            selfNetwork->sendStringToAll(3, selfNetwork->getID(), msg);

                            //Wait 2/3 of network to send OK Select; timeout 30sec

                            numRes = selfNetwork->waitResponses(selfNetwork->getTrustedNodeNumber() * THRESHOLD, RESPONSE_DELAY_MAX);

                            //The network answers if hash value is valid and last.
                            if (numRes >= selfNetwork->getTrustedNodeNumber() * THRESHOLD)
                            {
                                if (EXEC_MODE == DEBUG_MODE)
                                    cout << "Hash corrected" << endl;
                                auditedNode->updateHashList(content);
                            }
                            //The network doesnt answer if value doesnt need to be updated
                            else
                            {
                                //Insert conflictive hash in list
                                if (!auditedNode->isConflictiveHashRepeated(content))
                                    auditedNode->updateConflictiveHashList(content);
                                if (EXEC_MODE == DEBUG_MODE)
                                    cout << "Blame to " << auditedID << " ended successfully" << endl;
                                auditedNode->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
                            }

                            //Close connection
                            selfNetwork->reassembleAllSockets();
                        }
                        else
                        {
                            if (EXEC_MODE == DEBUG_MODE)
                                cout << "Network busy" << endl;
                        }
                    }
                }
            }
            // }
            //Catch error from recv
            catch (const std::exception &e)
            {
                auditedNode->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
                // auditedNode->incrementSyncNum(); //Comment??
                selfNetwork->reassembleSocket(auditedID);
                // auditedNode->incrementSyncNum();
                if (EXEC_MODE == DEBUG_MODE)
                    cerr << "Node doesnt trust me" << endl;
                // pthread_exit(NULL);
            }
        }
    }
    //Error connecting to audited node
    else
    {
        auditedNode->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
    }
    // pthread_exit(NULL);
}