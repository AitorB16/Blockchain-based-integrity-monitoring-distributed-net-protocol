#include "auditor.hpp"

//Reset confidence

bool waitResponseFromAudited(int sock)
{
    struct timeval tv;
    tv.tv_sec = AUDITOR_SELECT_WAIT;
    tv.tv_usec = 0;

    int selectStatus;
    int counter = 0;
    fd_set FdSet;

    FD_ZERO(&FdSet);  //clear the socket set
    int maxFD = sock; //initialize tmpMaxFD
    FD_SET(sock, &FdSet);

    //just monitorize trusted sockets; low-eq maxFD descriptor
    selectStatus = select(maxFD + 1, &FdSet, NULL, NULL, &tv);
    //
    counter += selectStatus;

    //if timeout or received message number is gr eq to resNum -> break the loop
    if (selectStatus == 0)
        return true;

    return false;
}

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
    int sock, auditedID, msgCode, syncNumReceived, numRes;
    string audID, selfID, MsgToVerify, MsgSignature, content, msg;
    bool msgValid;
    vector<string> vs;
    simpleNode *auditedNode;

    argNetwork args;
    args.net = selfNetwork;

    pthread_t trustThread;

    //For select
    // struct timeval tv;
    // tv.tv_sec = AUDITOR_SELECT_WAIT;
    // tv.tv_usec = 0;

    // fd_set readfds;

    if (pthread_create(&trustThread, NULL, resetTrustLvlThread, (void *)&args) != 0)
    {
        printf("Error");
        exit(1);
    }

    while (1)
    {
        //Update trusted node number
        selfNetwork->updateTrustedNodeNumber();

        // sleep(DEF_TIMER_AUDIT);
        sleep(DEF_TIMER_AUDIT);

        //If network not trusted, kill thread
        if (selfNetwork->isNetworkComprometed())
        {
            cout << "STOPPING AUDITOR, NETWORK COMPROMETED" << endl;
            return -1;
        }

        //return -1;

        auditedID = selfNetwork->getTrustedRandomNode();
        //No nodes are trusted
        if (auditedID == -1)
        {
            selfNetwork->setNetworkToComprometed();
            //SET ALL nodes to no trust
            cout << "STOPPING AUDITOR, NO TRUSTED NODES LEFT; NETWORK COMPROMETED" << endl;
            return -1;
        }
        auditedNode = selfNetwork->getNode(auditedID);

        msgValid = false;

        if (selfNetwork->connectToNode(auditedID))
        {
            if (selfNetwork->sendString(2, auditedID, selfNetwork->getID()))
            {
                sock = auditedNode->getSock();
                //Response from node

                //We need a countback
                // select(sock....)

                // if (select(sock + 1, &readfds, NULL, NULL, &tv) == 0)
                // {
                //     auditedNode->decreaseTrustLvlIn(TRUST_LEVEL);
                //     selfNetwork->reassembleSocket(auditedID);
                //     cout << "AAAA" << endl;
                // }
                // else
                // {
                try
                {
                    msg = selfNetwork->recvString(auditedID);

                    vs = splitBuffer(msg.c_str());

                    splitVectString(vs, msgCode, audID, selfID, syncNumReceived, content, MsgToVerify, MsgSignature);
                    msgValid = selfNetwork->validateMsg(selfID, audID, syncNumReceived, MsgToVerify, MsgSignature);

                    // cout << content << endl;
                    if (!msgValid) //Or node doesnt answer
                    {

                        //Attempt of message falsification
                        cout << "Disconnected message was faked" << endl;
                        //Decrease confidence in node
                        auditedNode->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);
                        selfNetwork->reassembleSocket(auditedID);
                    }
                    //Something valid is received
                    else
                    {
                        selfNetwork->reassembleSocket(auditedID);

                        //If content is valid and current OK
                        if (content == auditedNode->getLastHash())
                        {
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
                            selfNetwork->connectToAllNodes();

                            //Send just to connected nodes
                            msg = MsgToVerify + ";" + MsgSignature;
                            // cout << msg << endl;
                            selfNetwork->sendStringToAll(3, selfNetwork->getID(), msg);

                            //Wait 2/3 of network to send OK Select; timeout 30sec

                            //PROBLEMS HERE
                            numRes = selfNetwork->waitResponses(selfNetwork->getTrustedNodeNumber() * THRESHOLD, AUDITOR_SELECT_WAIT);
                            // numRes -= selfNetwork->getTrustedNodeNumber() + 1;
                            // numRes = selfNetwork->receiveFromAll(selfNetwork->getTrustedNodeNumber() * THRESHOLD, "UPDATE_HASH");

                            cout << "NUM RES: " << numRes << endl;

                            //The network answers if hash value is valid and last.
                            if (numRes >= selfNetwork->getTrustedNodeNumber() * THRESHOLD)
                            {
                                cout << "Hash corrected" << endl;
                                auditedNode->updateHashList(content);
                            }
                            //The network doesnt answer if value doesnt need to be updated
                            else
                            {
                                cout << "Blame ended successfully" << endl;
                                auditedNode->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);
                            }

                            //Close connection
                            selfNetwork->reassembleAllSockets();
                        }
                    }
                }
                //Catch error from recv
                catch (const std::exception &e)
                {
                    auditedNode->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);
                    selfNetwork->reassembleSocket(auditedID);
                    cout << "Node doesnt trust me" << endl;
                }
                // }
            }
        }
        //Error connecting to audited node.
        else
        {
            auditedNode->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);
        }
    }
    return 0;
}