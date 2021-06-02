#include "server.hpp"

vector<string> recvVectStringSocket(int sock)
{
    vector<string> vs;
    char buffer[4096];
    bzero(buffer, sizeof(buffer));
    try
    {
        recv(sock, buffer, 4096, 0);
        vs = splitBuffer(buffer);
        return vs;
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
        return vs;
    }
}

bool replyStringSocket(int code, netNode *nN, int sourceID, int sock, string content)
{

    string buffer, msg, signedMsg, hexMsg;
    int syncNum;

    try
    {
        syncNum = nN->getSyncNum();
        nN->setSyncNum(nN->getSyncNum() + 1);

        //Se especifica MsgID + origen + destino + syncNum
        msg = to_string(code) + ";" + to_string(sourceID) + ";" + to_string(nN->getID()) + ";" + to_string(syncNum) + ";" + content;

        signedMsg = sign(msg, std::to_string(sourceID));
        hexMsg = signedMsg;
        msg = msg + ";" + hexMsg + ";";
        buffer = msg;
        if (send(sock, buffer.c_str(), strlen(buffer.c_str()), 0) == -1)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Srv - Error sending: " << nN->getID() << endl;
            Logger("Srv - Error sending: " + to_string(nN->getID()));
        }
        else
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Srv - Success sending: " << nN->getID() << endl;
            return true;
        }

        return false;
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
        return false;
    }
}

struct argNetNodeTime
{
    netNode *nN;
    int timeToWork;
};

struct argSocket
{
    int s;
};

void *timerThread(void *arg)
{

    struct argNetNodeTime *args = (struct argNetNodeTime *)arg;
    // int clientSocket = args->s;
    netNode *nN = args->nN;
    int timeToWork = args->timeToWork;

    //Default wait time + 2, or + 1 (tolerate small delays)
    if (timeToWork > HASH_UPDATE_TIMESPACE_MAX || timeToWork < HASH_UPDATE_TIMESPACE_MIN)
    {
        sleep(HASH_UPDATE_TIMESPACE_MAX + 2);
    }
    else
    {
        sleep(timeToWork + 2);
    }

    //If changeflag ative, set it to false
    if (nN->getChangeFlag())
    {
        nN->setChangeFlag(false);

        if (EXEC_MODE == DEBUG_MODE)
            cout << "Srv - Change flag desactivated (time run out) - ID: " << nN->getID() << endl;
        Logger("Srv - Change flag desactivated (time run out) - ID: " + to_string(nN->getID()));
    }

    pthread_exit(NULL);
}

void *socketThread(void *arg)
{
    char sendBuffer[4096];

    struct argSocket *args = (struct argSocket *)arg;
    int clientSocket = args->s;

    argNetNodeTime argNNodeTime;

    netNode *nN;

    network *net = net->getInstance();
    vector<string> vectString;
    int msgCode, clientID, selfID, syncNumReceived, syncNumStored, suspectID, auditorID, susSyncNumReceived, susMsgCode;

    string MsgToVerify, MsgSignature, content, susMsgToVerify, susMsgSignature, susContent;

    bool msgValid = false;

    pthread_t timerTid;

    //SELECT TO PREVENT BLOCKING RESOURCES
    struct timeval tv;
    tv.tv_sec = RESPONSE_DELAY_MAX;
    tv.tv_usec = 0;

    fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(clientSocket, &fdSet);

    //Timeout
    if (0 == select(clientSocket + 1, &fdSet, NULL, NULL, &tv))
    {
        //Client didnt send msg
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Srv - Client run out of time" << endl;
        Logger("Srv - Client run out of time");
        close(clientSocket);
        pthread_exit(NULL);
    }

    //Clean buffers and vars
    bzero(sendBuffer, sizeof(sendBuffer));

    //Receive msg
    vectString = recvVectStringSocket(clientSocket);

    //Regular case
    if (vectString.size() == 6)
    {
        splitVectString(vectString, msgCode, clientID, selfID, syncNumReceived, content, MsgToVerify, MsgSignature);
    }
    //Blame case
    else if (vectString.size() == 11)
    {
        splitVectStringBlame(vectString, msgCode, clientID, selfID, syncNumReceived, susMsgCode, suspectID, auditorID, susSyncNumReceived, susContent, susMsgSignature, susMsgToVerify, MsgSignature, MsgToVerify);
    }
    //Msg not identified
    else
    {
        //Someone is connecting fakely
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Srv - Disconnected not valid" << endl;
        Logger("Srv - Disconnected not valid");
        sleep(RESPONSE_DELAY_MAX + 1);
        close(clientSocket);
        pthread_exit(NULL);
    }

    //get connected corresponeded node
    nN = net->getNode(clientID);

    //If not trusted, close connection
    if (!nN->isTrusted())
    {
        //No trusted node trying to connect
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Srv - Disconnected not trusted, ID " << clientID << endl;
        Logger("Srv - Disconnected not trusted, ID " + to_string(clientID));
        //prevent sending ACK
        sleep(RESPONSE_DELAY_MAX + 1);
        close(clientSocket);
        pthread_exit(NULL);
    }

    //Validate received msg
    msgValid = net->validateMsg(selfID, clientID, syncNumReceived, MsgToVerify, MsgSignature);

    //If msg is not valid, close connection.
    if (!msgValid)
    {
        //Attempt of message falsification

        //Dont decrease trustlvl; someone could be replacing client's identity
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Srv - Disconnected message was faked- ID: " << clientID << " SyncNumStored: " << nN->getSyncNum() << " SyncNumReceived: " << syncNumReceived << endl;
        Logger("Srv - Disconnected message was faked - ID: " + to_string(clientID) + " SyncNumStored: " + to_string(nN->getSyncNum()) + " SyncNumReceived: " + to_string(syncNumReceived));
        //prevent sending ACK
        sleep(RESPONSE_DELAY_MAX + 1);
        close(clientSocket);
        pthread_exit(NULL);
    }

    switch (msgCode)
    {
    //Modify current hash request
    case 0:

        //Activate flag using locks
        nN->setChangeFlag(true);
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Srv - Change flag activated of - ID: " << clientID << endl;
        Logger("Srv - Change flag activated of - ID: " + to_string(clientID));

        //Create new args of netNode
        argNNodeTime.nN = nN;
        argNNodeTime.timeToWork = atoi(content.c_str());

        //Launch timer to desactive flag
        if (pthread_create(&timerTid, NULL, timerThread, (void *)&argNNodeTime) != 0)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Srv - Error creating flag thread" << endl;
            Logger("Srv - Error creating flag thread");
        }

        //Decrease trustlvl to limit requests and prevent DDOS -- random 2 to make decreasement slower
        // if (get_randomNumber(2) == 0)
        //     nN->decreaseTrustLvlIn(TRUST_DECREASE_CONST);

        //Reply with ACK msg
        strcpy(sendBuffer, "ACK");
        if (send(clientSocket, sendBuffer, strlen(sendBuffer), 0) == -1)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Srv - Error sending ACK reply - ID: " << clientID << endl;
            Logger("Srv - Error sending ACK reply - ID: " + to_string(clientID));
        }
        else
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Srv - Success sending ACK reply - ID: " << clientID << endl;
            Logger("Srv - Success sending ACK reply - ID: " + to_string(clientID));
        }

        sleep(RESPONSE_DELAY_MAX + 1);
        //Closing socket is same as ACK
        close(clientSocket);

        pthread_exit(NULL);
        break;

    //Update hash of netNode
    case 1:
        //Flag must be previously active
        if (nN->getChangeFlag())
        {
            //Update hash of network node if a change has happened
            if (nN->getLastHash() != content)
            {
                // if (nN->getLastConflictiveHash() != content)
                // {
                nN->updateHashList(content);
                nN->updateNodeBChain(content);

                if (EXEC_MODE == DEBUG_MODE)
                    cout << "Srv - New hash value of - ID: " << clientID << " Hash: " << content << endl;
                Logger("Srv - New hash value of - ID: " + to_string(clientID) + " Hash: " + content);
                // }
                // else
                // {
                //     if (EXEC_MODE == DEBUG_MODE)
                //         cout << "Srv - Last conflictive hash values eq to received - ID: " << clientID << " Hash: " << content << endl;
                //     Logger("Srv - Last hash values eq to received - ID: " + to_string(clientID) + " Hash: " + content);
                // }
            }
            else
            {
                if (EXEC_MODE == DEBUG_MODE)
                    cout << "Srv - Last hash values eq to received - ID: " << clientID << " Hash: " << content << endl;
                Logger("Srv - Last hash values eq to received - ID: " + to_string(clientID) + " Hash: " + content);
            }
            //Desactive flag
            nN->setChangeFlag(false);
        }
        //No request done or time run out
        else
        {
            // if (get_randomNumber(2) == 0)
            // nN->decreaseTrustLvlIn(TRUST_DECREASE_C2);
            nN->increaseIncidenceNum(INCIDENT_INCREASE);
        }

        //Close connection
        close(clientSocket);
        pthread_exit(NULL);
        break;
    //Audit request
    case 2:
        //Send my info
        replyStringSocket(2, nN, selfID, clientSocket, net->getSelfNode()->getLastHash());
        close(clientSocket);
        pthread_exit(NULL);
        break;
    //Blame
    case 3:
        //If im the suspicious node, dont do anything
        if (suspectID == selfID)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Srv - Im being audited" << endl;
            Logger("Srv - Im being audited");
        }
        //If im not the suspicous node
        else
        {
            //The content isnt the last in the list
            if (net->getNode(suspectID)->getLastHash() != susContent)
            {
                //Non repudiation from suspicious
                if (verify(susMsgToVerify, susMsgSignature, to_string(suspectID)))
                {
                    //The blamed node is already not trusted
                    if (!net->getNode(suspectID)->isTrusted())
                    {
                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Srv - Blamed node is already not trusted - ID: " << suspectID << " Hash: " << susContent << endl;
                        Logger("Srv - Srv - Blamed node is already not trusted - ID: " + to_string(suspectID) + " Hash: " + susContent);
                    }
                    //Insert conflictive hash in list
                    else if (net->getNode(suspectID)->getLastConflictiveHash() != susContent)
                    {
                        net->getNode(suspectID)->updateConflictiveHashList(susContent);
                        net->getNode(suspectID)->updateNodeBChain(susContent);

                        //Decrease confidence on suspicious
                        net->getNode(suspectID)->decreaseTrustLvlIn(TRUST_DECREASE);
                        //Preventive, not leting arbitary decreases
                        net->getNode(auditorID)->decreaseTrustLvlIn(TRUST_DECREASE);

                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Srv - Last conflictive Hash updated - ID: " << suspectID << " Hash: " << susContent;
                        Logger("Srv - Last conflictive Hash updated - ID: " + to_string(suspectID) + " Hash: " + susContent);
                    }
                    else
                    {
                        //Decrease confidence on suspicious
                        net->getNode(suspectID)->decreaseTrustLvlIn(TRUST_DECREASE);
                        //Preventive, not leting arbitary decreases
                        net->getNode(auditorID)->decreaseTrustLvlIn(TRUST_DECREASE);

                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Srv - Last conflictive hash values eq to blamed - ID: " << suspectID << " Hash: " << susContent << endl;
                        Logger("Srv - Last conflictive hash values eq to blamed - ID: " + to_string(suspectID) + " Hash: " + susContent);
                    }
                }
                //Msg faked by auditor
                else
                {
                    //Decrease confidence on auditor
                    // net->getNode(auditorID)->decreaseTrustLvlIn(TRUST_DECREASE_C2);
                    nN->increaseIncidenceNum(INCIDENT_INCREASE);
                }
            }
            //OK the auditor node isnt updated
            // else if (net->getNode(suspectID)->getLastHash() == susContent)
            else
            {
                //Decrease 1 unit confidence on auditor to prevent DDoS
                // net->getNode(auditorID)->decreaseTrustLvlIn(TRUST_DECREASE_C2);
                nN->increaseIncidenceNum(INCIDENT_INCREASE);

                //Reply with UPDATE_HASH msg
                strcpy(sendBuffer, "UPDATE_HASH");
                if (send(clientSocket, sendBuffer, strlen(sendBuffer), 0) == -1)
                {
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Srv - Error sending update hash reply - ID: " << auditorID << endl;
                    Logger("Srv - Error sending update hash reply - ID: " + to_string(auditorID));
                }
                else
                {
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Srv - Success sending update hash reply - ID: " << auditorID << endl;
                    Logger("Srv - Success sending update hash reply - ID: " + to_string(auditorID));
                }
            }
        }
        //Close connection
        //RESPONSE_DELAY_MAX + 1
        sleep(RESPONSE_DELAY_MAX + 1);
        // cout << "Srv - Disconnected" << endl;
        close(clientSocket);
        pthread_exit(NULL);
        break;
    default:
        //Close connection
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Srv - Disconnected" << endl;
        close(clientSocket);
        pthread_exit(NULL);
        break;
    }
}

server::server(){

};
server::server(network *selfNetwork)
{
    server::selfNetwork = selfNetwork;
};

int server::serverUP()
{
    int newSocket;
    int sock = selfNetwork->getSelfNode()->getSock();
    sockaddr_in addr = selfNetwork->getSelfNode()->getAddr();
    int max_c = selfNetwork->getNetNodeNumber(); //At the begining all nodes are trusted
    int addrlen = sizeof(addr);
    int i = 0;
    argSocket args;

    pthread_t tid[max_c];

    //LISTEN
    if (listen(sock, max_c) < 0)
        return 1;

    cout << "Srv - Server UP" << endl;
    Logger("Srv - Server UP");

    while (1)
    {

        //Should identity verification be done to prevent DDoS?
        newSocket = accept(sock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);

        if (newSocket < 0)
            return 1;

        // printf("Connection accepted from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        args.s = newSocket;

        if (pthread_create(&tid[i++], NULL, socketThread, (void *)&args) != 0)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Srv - Error creating thread to serve client" << endl;
            Logger("Srv - Error creating thread to serve client");
        }

        if (i >= max_c)
        {
            i = 0;
            while (i < max_c)
            {
                pthread_join(tid[i++], NULL);
            }
            i = 0;
        }
        //In case network comprometed, end loop
        if (selfNetwork->isNetworkComprometed())
        {
            break;
        }
    }

    //Close server socket
    close(sock);
    cout << "Srv - SERVER STOPPED" << endl;
    Logger("Srv - SERVER STOPPED");
    //Wait max life time of child thread before killing parent thread
    sleep(HASH_UPDATE_TIMESPACE_MAX);
    // pthread_exit(NULL);
    return 1;
}
