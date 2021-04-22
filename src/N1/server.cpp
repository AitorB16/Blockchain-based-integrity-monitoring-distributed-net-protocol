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
        if (DEBUG_MODE)
            std::cerr << e.what() << '\n';
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
        nN->incrementSyncNum();

        //Se especifica MsgID + origen + destino + syncNum
        msg = to_string(code) + ";" + to_string(sourceID) + ";" + to_string(nN->getID()) + ";" + to_string(syncNum) + ";" + content;

        signedMsg = sign(msg, std::to_string(sourceID));
        hexMsg = stream2hex(signedMsg);
        msg = msg + ";" + hexMsg + ";";
        buffer = msg;
        if (send(sock, buffer.c_str(), strlen(buffer.c_str()), 0) == -1)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Error sending: " << nN->getID() << endl;
        }
        else
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Success sending: " << nN->getID() << endl;
            return true;
        }

        return false;
    }
    catch (const std::exception &e)
    {
        if (DEBUG_MODE)
            std::cerr << e.what() << '\n';
        return false;
    }
}

server::server(){

};
server::server(network *selfNetwork)
{
    server::selfNetwork = selfNetwork;
};

struct argNetnodeSocket
{
    netNode *nN;
    int s;
};

struct argNetworkSocket
{
    network *net;
    int s;
};

void *timerThread(void *arg)
{

    struct argNetnodeSocket *args = (struct argNetnodeSocket *)arg;
    // int clientSocket = args->s;
    netNode *nN = args->nN;

    //Default wait time + 2, or + 1 (tolerate small delays)
    sleep(HASH_UPDATE_TIMESPACE + 2);

    //If changeflag ative, set it to false
    if (nN->getChangeFlag())
        nN->setChangeFlag(false);

    pthread_exit(NULL);
}

void *socketThread(void *arg)
{
    char sendBuffer[4096];

    struct argNetworkSocket *args = (struct argNetworkSocket *)arg;
    argNetnodeSocket argNetNode;

    int clientSocket = args->s;
    netNode *nN;

    network *net = args->net;
    vector<string> vectString;
    int msgCode, clientID, selfID, syncNumReceived, syncNumStored, suspectID, auditorID, susSyncNumReceived, susMsgCode;

    string MsgToVerify, MsgSignature, content, susMsgToVerify, susMsgSignature, susContent;

    bool msgValid = false;

    while (1)
    {
        //USE SELECT TO PREVENT BLOCKING RESOURCES

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
                cout << "Disconnected not valid" << endl;
            close(clientSocket);
            pthread_exit(NULL);
        }

        //If not trusted, close connection
        if (!net->getNode(clientID)->isTrusted())
        {
            //Attempt of message falsification
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Disconnected not trusted, ID " << clientID << endl;
            close(clientSocket);
            pthread_exit(NULL);
        }

        //get connected corresponeded node
        nN = net->getNode(clientID);

        //Validate received msg
        msgValid = net->validateMsg(selfID, clientID, syncNumReceived, MsgToVerify, MsgSignature);

        //If msg is not valid, close connection.
        if (!msgValid)
        {
            //Attempt of message falsification

            //Dont decrease trustlvl; someone could be replacing client's identity
            close(clientSocket);
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Disconnected message was faked (in server)" << nN->getSyncNum() << endl;
            pthread_exit(NULL);
        }

        //Create new args of netNode
        argNetNode.nN = nN;
        argNetNode.s = clientSocket;

        switch (msgCode)
        {
        //Modify current hash request
        case 0:

            //Activate flag using locks
            nN->setChangeFlag(true);

            //Launch timer to desactive flag
            pthread_t tid;
            if (pthread_create(&tid, NULL, timerThread, (void *)&argNetNode) != 0)
            {
                if (EXEC_MODE == DEBUG_MODE)
                    cout << "Error creating flag thread" << endl;
            }

            //Decrease trustlvl to limit requests and prevent DDOS -- random 2 to make decreasement slower
            // if (get_randomNumber(2) == 0)
            //     nN->decreaseTrustLvlIn(TRUST_DECREASE_CONST);

            //Closing socket is same as ACK
            close(clientSocket);
            pthread_exit(NULL);
            break;

        //Update hash of netNode
        case 1:
            //Flag must be previously active
            if (nN->getChangeFlag())
            {
                //Update hash of network node
                nN->updateHashList(content);

                //Desactive flag
                nN->setChangeFlag(false);
            }
            //No request done or time run out
            else
            {
                // if (get_randomNumber(2) == 0)
                nN->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
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
                    cout << "Im being audited" << endl;
            }
            //If im not the suspicous node
            else
            {
                //The content isnt on the list
                if (!net->getNode(suspectID)->isHashRepeated(susContent))
                {
                    //Non repudiation from suspicious
                    if (verify(susMsgToVerify, hex2stream(susMsgSignature), to_string(suspectID)))
                    {
                        //Insert conflictive hash in list
                        if (!net->getNode(suspectID)->isConflictiveHashRepeated(susContent))
                            net->getNode(suspectID)->updateConflictiveHashList(susContent);

                        //Decrease confidence on suspicious
                        net->getNode(suspectID)->decreaseTrustLvlIn(TRUST_DECREASE_CONST);

                        //Preventive, if I lost this update, decrease confidence on auditor.
                        //MAKE DE AUDITOR DECREASE AT trusted node number * 2/3 SPEED OF SUSPECT
                        if (get_randomNumber(net->getTrustedNodeNumber() * THRESHOLD) == 0)
                            net->getNode(auditorID)->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
                    }
                    //Msg faked by auditor
                    else
                    {
                        //Decrease confidence on auditor
                        net->getNode(auditorID)->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
                    }
                }
                //OK the auditor node isnt updated
                else if (net->getNode(suspectID)->getLastHash() == susContent)
                {
                    //Decrease 1 unit confidence on auditor to prevent DDoS
                    net->getNode(auditorID)->decreaseTrustLvlIn(TRUST_DECREASE_CONST);

                    //Reply with UPDATE_HASH msg
                    strcpy(sendBuffer, "UPDATE_HASH");
                    if (send(clientSocket, sendBuffer, strlen(sendBuffer), 0) == -1)
                    {
                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Error sending: " << auditorID << endl;
                    }
                    else
                    {
                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Success sending: " << auditorID << endl;
                    }
                }
                //Auditor sent me an old msg
                else
                {
                    //Decrease confidence on auditor
                    net->getNode(auditorID)->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
                }
            }
            //Close connection
            //RESPONSE_DELAY_MAX + 1
            sleep(RESPONSE_DELAY_MAX + 1);
            // cout << "Disconnected" << endl;
            close(clientSocket);
            pthread_exit(NULL);
            break;
        default:
            //Close connection
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Disconnected" << endl;
            close(clientSocket);
            pthread_exit(NULL);
            break;
        }
    }
}

int server::serverUP()
{
    int newSocket;
    int sock = selfNetwork->getSelfNode()->getSock();
    sockaddr_in addr = selfNetwork->getSelfNode()->getAddr();
    int max_c = selfNetwork->getNodeNumber(); //At the begining all nodes are trusted
    int addrlen = sizeof(addr);
    int i = 0;
    argNetworkSocket args;

    pthread_t tid[max_c];

    //LISTEN
    if (listen(sock, max_c) < 0)
        return -1;

    if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
        cout << "Server UP" << endl;

    while (1)
    {

        //Should identity verification be done to prevent DDoS?
        newSocket = accept(sock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);

        if (newSocket < 0)
            return -1;

        // printf("Connection accepted from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        args.net = selfNetwork;
        args.s = newSocket;

        if (pthread_create(&tid[i++], NULL, socketThread, (void *)&args) != 0)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Error creating thread to serve client" << endl;
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
    if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
        cout << "SERVER STOPPED" << endl;
    //Wait max life time of child thread before killing parent thread
    sleep(HASH_UPDATE_TIMESPACE);
    // pthread_exit(NULL);
    return -1;
}
