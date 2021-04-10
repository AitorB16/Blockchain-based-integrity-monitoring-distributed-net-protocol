#include "server.hpp"

vector<string> recvVectStringSocket(int sock)
{
    vector<string> vs;
    char buffer[2048];
    bzero(buffer, sizeof(buffer));
    try
    {
        recv(sock, buffer, 2048, 0);
        vs = splitBuffer(buffer);
        return vs;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return vs;
    }
}

bool replyStringSocket(int code, simpleNode *sN, int sourceID, int sock, string content)
{

    string buffer, msg, signedMsg, hexMsg;
    int syncNum;

    try
    {
        //Put msg code
        // buffer = to_string(code) + ";";

        //Random msg
        // msg = std::string(random);
        //Get and increment Sync Number
        syncNum = sN->getSyncNum();
        sN->incrementSyncNum();

        //Se especifica MsgID + origen + destino + syncNum
        msg = to_string(code) + ";" + to_string(sourceID) + ";" + to_string(sN->getID()) + ";" + to_string(syncNum) + ";" + content;

        signedMsg = sign(msg, std::to_string(sourceID));
        hexMsg = stream2hex(signedMsg);
        msg = msg + ";" + hexMsg + ";";
        buffer = msg;
        if (send(sock, buffer.c_str(), strlen(buffer.c_str()), 0) == -1)
            cout << "Error sending: " << sN->getID() << endl;
        else
        {
            cout << "Success sending: " << sN->getID() << endl;
            return true;
        }

        return false;
    }
    catch (const std::exception &e)
    {
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

struct argSimplenodeSocket
{
    simpleNode *sN;
    int s;
};

struct argNetworkSocket
{
    network *net;
    int s;
};

void *timerThread(void *arg)
{

    struct argSimplenodeSocket *args = (struct argSimplenodeSocket *)arg;
    int clientSocket = args->s;
    simpleNode *sN = args->sN;

    //Default sleep time
    sleep(DEF_TIMER_WAIT);
    sN->setChangeFlag(false);
    pthread_exit(NULL);
}

void *socketThread(void *arg)
{
    char sendBuffer[1024];

    struct argNetworkSocket *args = (struct argNetworkSocket *)arg;
    argSimplenodeSocket argSimpleNode;

    int clientSocket = args->s;
    simpleNode *sN;

    // sN->setSocket(clientSocket);

    network *net = args->net;
    vector<string> vectString;
    int msgCode, syncNumReceived, syncNumStored;

    string clientID, selfID, MsgToVerify, MsgSignature, content;

    int susSyncNumReceived, susMsgCode;
    string suspectID, auditorID, susMsgToVerify, susMsgSignature, susContent;

    bool msgValid = false;

    while (1)
    {
        //Clean buffers and vars
        bzero(sendBuffer, sizeof(sendBuffer));

        vectString = recvVectStringSocket(clientSocket);

        //Read msgCode
        msgCode = atoi(vectString.at(0).c_str());

        //Regular cases
        if (msgCode != 3)
        {
            splitVectString(vectString, msgCode, clientID, selfID, syncNumReceived, content, MsgToVerify, MsgSignature);
        }
        //Blame case
        else
        {
            //Headers
            clientID = vectString.at(1);
            selfID = vectString.at(2);
            syncNumReceived = atoi(vectString.at(3).c_str());

            //Content
            susMsgCode = atoi(vectString.at(4).c_str());
            suspectID = vectString.at(5); //Suspicious ID
            auditorID = vectString.at(6); //ID of auditor
            susSyncNumReceived = atoi(vectString.at(7).c_str());
            susContent = vectString.at(8);      //Conflictive hash
            susMsgSignature = vectString.at(9); //Signed msg
            susMsgToVerify = to_string(susMsgCode) + ";" + suspectID + ";" + auditorID + ";" + to_string(susSyncNumReceived) + ";" + susContent;

            //Signature
            MsgSignature = vectString.at(10);
            MsgToVerify = to_string(msgCode) + ";" + clientID + ";" + selfID + ";" + to_string(syncNumReceived) + ";" + susMsgToVerify + ";" + susMsgSignature;
        }

        //If not trusted, close connection
        if (!net->getNode(atoi(clientID.c_str()))->isTrusted())
        {
            //Attempt of message falsification
            cout << "Disconnected not trusted" << endl;
            close(clientSocket);
            pthread_exit(NULL);
        }

        //get connected corresponeded node
        sN = net->getNode(atoi(clientID.c_str()));

        msgValid = net->validateMsg(selfID, clientID, syncNumReceived, MsgToVerify, MsgSignature);

        //If msg is not valid, close connection.
        if (!msgValid)
        {

            //Attempt of message falsification
            cout << "Disconnected message was faked" << endl;
            close(clientSocket);
            pthread_exit(NULL);
        }

        //Create new args of simpleNode
        argSimpleNode.sN = sN;
        argSimpleNode.s = clientSocket;

        switch (msgCode)
        {
        //Modify current hash request
        case 0:

            //Activar flag usando cerraduras
            sN->setChangeFlag(true);

            //Lanzar thread contador para anular cerradura
            pthread_t tid;
            if (pthread_create(&tid, NULL, timerThread, (void *)&argSimpleNode) != 0)
            {
                cout << "Error" << endl;
            }
            //pthread_join(tid, NULL);

            //Enviar mensaje ACK de vuelta
            strcpy(sendBuffer, "ACK");
            send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
            cout << "SENT" << endl;

            //Dont close connection, we spect to receive the new hash from same node
            break;

        //Update hash of simpleNode
        case 1:
            cout << "received" << endl;

            if (sN->getChangeFlag())
            {
                //Update hash of network node
                sN->updateHashList(content);

                //Desactivar flag usando cerraduras
                sN->setChangeFlag(false);
            }

            //Close connection
            cout << "Disconnected" << endl;
            close(clientSocket);
            pthread_exit(NULL);
            break;
        //Audit req
        case 2:
            cout << "received" << endl;
            // net->sendString(2, atoi(clientID.c_str()), atoi(selfID.c_str()), net->getSelfNode()->getCurrentHash());
            replyStringSocket(2, sN, atoi(selfID.c_str()), clientSocket, net->getSelfNode()->getLastHash());
            cout << "Disconnected" << endl;
            close(clientSocket);
            pthread_exit(NULL);
            break;
            //Modify current hash request
        case 3:
            //If im the suspicious node, dont do anything
            if (suspectID == selfID)
            {
                cout << "Im being audited" << endl;
            }
            //If im not the suspicous node
            else
            {
                //The content isnt on the list
                if (!net->getNode(atoi(suspectID.c_str()))->isHashRepeated(susContent))
                {
                    //Non repudiation from suspicious
                    if (verify(susMsgToVerify, hex2stream(susMsgSignature), suspectID))
                    {
                        //Decrease confidence on suspicious
                        net->getNode(atoi(suspectID.c_str()))->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);

                        //Preventive, if I lost this update, decrease confidence on auditor.
                        //RANDOMIZE MOD 2 = 0 AUDITOR DECREASEMENT
                        // net->getNode(atoi(auditorID.c_str()))->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);
                    }
                    //Msg faked by auditor
                    else
                    {
                        //Decrease complelty confidence on auditor
                        net->getNode(atoi(auditorID.c_str()))->decreaseTrustLvlIn(TRUST_LEVEL);
                    }
                }
                //OK the auditor node isnt updated
                else if (net->getNode(atoi(suspectID.c_str()))->getLastHash() == susContent)
                {
                    //Decrease 1 unit confidence on auditor to prevent DDoS
                    net->getNode(atoi(auditorID.c_str()))->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);

                    //Enviar mensaje UPDATE_HASH de vuelta
                    cout << "SEND UPDATE" << net->getNode(atoi(suspectID.c_str()))->getLastHash() << endl;
                    // cout << susContent << endl;
                    // replyStringSocket(3, sN, atoi(selfID.c_str()), clientSocket, "UPDATE_HASH");
                    strcpy(sendBuffer, "UPDATE_HASH");
                    send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
                }
                //Auditor send me an old msg
                else
                {
                    //Decrease complelty confidence on auditor
                    net->getNode(atoi(auditorID.c_str()))->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);
                }
            }
            //Close connection
            //AUDITOR_SELECT_WAIT + 1
            sleep(3);
            cout << "Disconnected" << endl;
            close(clientSocket);
            pthread_exit(NULL);
            break;
        default:
            //Close connection
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

    cout << "Server UP" << endl;

    while (1)
    {
        //Se debería analizar la identidad de conexiones para evitar DDoS
        newSocket = accept(sock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
        if (newSocket < 0)
            return -1;

        printf("Connection accepted from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        args.net = selfNetwork;
        args.s = newSocket;

        if (pthread_create(&tid[i++], NULL, socketThread, (void *)&args) != 0)
        {
            printf("Error");
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
    }

    close(newSocket);
    return 0;
}
