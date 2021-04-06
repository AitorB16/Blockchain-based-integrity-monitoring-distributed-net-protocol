#include "server.hpp"

vector<string> recvVectStringSocket(int s)
{
    vector<string> vs;
    char buffer[1024];
    bzero(buffer, sizeof(buffer));
    try
    {
        recv(s, buffer, 1024, 0);
        vs = splitBuffer(buffer);
        return vs;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return vs;
    }
}

server::server(){

};
server::server(network *selfNetwork)
{
    server::selfNetwork = selfNetwork;
};

struct argStructSimpleNode
{
    simpleNode *sN;
    int s;
};

struct argStructNetworkAndNode
{
    network *net;
    int s;
};

void *timerThread(void *arg)
{

    struct argStructSimpleNode *args = (struct argStructSimpleNode *)arg;
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

    struct argStructNetworkAndNode *args = (struct argStructNetworkAndNode *)arg;
    argStructSimpleNode argSimpleNode;

    int clientSocket = args->s;
    simpleNode *sN;

    // sN->setSocket(clientSocket);

    network *net = args->net;
    vector<string> vectString;
    int msgCode, syncNumReceived, syncNumStored;

    string clientID, selfID, MsgToVerify, MsgSignature, content;

    bool msgValid = false;

    while (1)
    {
        //Clean buffers and vars
        bzero(sendBuffer, sizeof(sendBuffer));

        vectString = recvVectStringSocket(clientSocket);

        splitVectString(vectString, msgCode, clientID, selfID, syncNumReceived, MsgToVerify, MsgSignature, content);

        //get connected corresponeded node
        sN = net->getNode(atoi(clientID.c_str()));

        //If msg is not valid, close connection.
        msgValid = net->validateMsg(selfID, clientID, syncNumReceived, MsgToVerify, MsgSignature);

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
            //cout << n->getChangeFlag() <<endl;

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

            //As we use the same socket -> no problem with faked messages
            if (verify(MsgToVerify, hex2stream(MsgSignature), clientID))
            {
                if (sN->getChangeFlag())
                {
                    //Update hash of network node
                    sN->setCurrentHash(content);

                    //Desactivar flag usando cerraduras
                    sN->setChangeFlag(false);
                }
            }

            //Close connection
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
    argStructNetworkAndNode args;

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
