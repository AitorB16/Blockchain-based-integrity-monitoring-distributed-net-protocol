#include "server.hpp"

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
    char buffer[1024];
    char sendBuffer[1024];

    struct argStructNetworkAndNode *args = (struct argStructNetworkAndNode *)arg;
    argStructSimpleNode argSimpleNode;

    int clientSocket = args->s;
    simpleNode *sN;
    network *net = args->net;
    vector<string> vectString;
    int msgCode;

    bool verifyMsg = false;
    string clientID, selfID, randMsg, randHexSignedMsg, content;

    while (1)
    {
        bzero(buffer, sizeof(buffer));
        bzero(sendBuffer, sizeof(sendBuffer));

        recv(clientSocket, buffer, 1024, 0);

        vectString = splitBuffer(buffer); //from utils

        msgCode = atoi(vectString.at(0).c_str());  //MSG CODE
        clientID = vectString.at(1);               //Source ID
        selfID = vectString.at(2);                 //ID of current server
        randMsg = selfID + ";" + vectString.at(3); //RandomMsg !!TENER EN CUENTA QUE EL ID DEL SERVER ESTA FIRMADO JUNTO CON LOS CARACTERES ALEATORIOS
        randHexSignedMsg = vectString.at(4);       //RandomMsg signed
        content = vectString.at(5);                //Payload of the msg

        //get connected corresponeded node
        sN = net->getNode(atoi(clientID.c_str()));

        //Create new args of simpleNode
        argSimpleNode.sN = sN;
        argSimpleNode.s = clientSocket;

        //Verify is im destination
        if (atoi(selfID.c_str()) == net->getID())
        {
            switch (msgCode)
            {
            //Modify current hash request
            case 0:
                if (!net->isMsgRepeated(randMsg))
                {
                    if (verify(randMsg, hex2stream(randHexSignedMsg), clientID))
                    {
                        net->insertInReceivedMsgs(randMsg);
                        //Activar flag usando cerraduras
                        sN->setChangeFlag(true);
                        //cout << n->getChangeFlag() <<endl;

                        //Lanzar thread contador para anular cerradura
                        pthread_t tid;
                        if (pthread_create(&tid, NULL, timerThread, (void *)&argSimpleNode) != 0)
                        {
                            printf("Error");
                        }
                        //pthread_join(tid, NULL);

                        //Enviar mensaje aleatorio de vuelta
                        sendBuffer[0] = 'A';
                        send(clientSocket, sendBuffer, strlen(sendBuffer), 0);
                        cout << "SENT" << endl;
                    }
                }
                //Dont close connection, we spect to receive the new hash from same node
                break;

            //Update hash of simpleNode
            case 1:
                cout << "received" << endl;
                if (!net->isMsgRepeated(randMsg))
                {
                    if (verify(randMsg, hex2stream(randHexSignedMsg), clientID))
                    {
                        net->insertInReceivedMsgs(randMsg);
                        if (sN->getChangeFlag())
                        {
                            //Update hash of network node
                            sN->setCurrentHash(content);

                            //Desactivar flag usando cerraduras
                            sN->setChangeFlag(false);
                        }
                    }
                }
                //Close connection
                cout << "Disconnected" << endl;
                close(clientSocket);
                pthread_exit(NULL);
                break;
            default:
                break;
            }
        }
        else
        {
            //Attempt of message falsification
            cout << "Disconnected message was faked" << endl;
            close(clientSocket);
            pthread_exit(NULL);
        }
    }
}

int server::serverUP(int max_c)
{
    int newSocket;
    int sock = selfNetwork->getSelfNode()->getSock();
    sockaddr_in addr = selfNetwork->getSelfNode()->getAddr();
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