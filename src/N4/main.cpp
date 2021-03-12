#include "main.hpp"
using namespace std;

struct argNetwork
{
    network *net;
};

//Server thread start
void *serverThread(void *arg)
{
    struct argNetwork *args = (struct argNetwork *)arg;

    network *net = args->net;

    server *s;
    s = new server(net);
    s->serverUP(net->getNodeNumber());

    pthread_exit(NULL);
}

int main(void)
{

    pthread_t serverTid;

    network *net = net->getInstance();

    argNetwork args;
    args.net = net;

    int numRes;

    cout << net->getID() << endl;

    //Print other nodes
    net->printOtherNodes();

    if (pthread_create(&serverTid, NULL, serverThread, (void *)&args) != 0)
    {
        printf("Error");
    }

    {
        // const char *response;
        int option;

        std::string buffer, response;
        int dest;

        while (1)
        {

            // waitpid(-1, NULL, WNOHANG); //KILL ZOMBIE PROCESSES

            cout << "Enter options" << endl;
            cin >> option;
            switch (option)
            {
            //Close infraestructure
            case 0:
                cout << "Exiting infra" << endl;
                exit(0);
                break;
            //Is infra trusted
            case 1:
                cout << net->imTrusted() << endl;
                break;
            //Connect to ID node
            case 2:
                cout << "enter node ID" << endl;
                cin >> dest;
                net->connectToNode(dest);
                // net->connectToAdjacents();
                break;
            //Disconnect from ID + reassamble socket
            case 3:
                cout << "enter node ID" << endl;
                cin >> dest;
                net->sendString(0, dest, net->getID());
                net->reassembleSocket(dest); //Reassamble the socket for future reconnections
                break;
            //Send string to ID node
            case 4:
                cout << "enter node ID" << endl;
                cin >> dest;
                buffer = "AAAAAAAAAAAAAAA";
                net->sendString(1, dest, net->getID(), buffer);
                break;
            //BCAST request to send + timeout + count
            case 5:
                //Connect to ALL
                net->connectToAllNodes();

                //Send request to ALL
                // maxFD = net->sendStringToAll(buffer.c_str());
                net->sendStringToAll(2, net->getID());

                //Wait 2/3 of network to send OK Select; timeout 30sec
                numRes = net->waitResponses(net->getTrustedNodeNumber() * THRESHOLD);

                if (numRes >= net->getTrustedNodeNumber() * THRESHOLD)
                {
                    //Send hash
                    cout << "PPRRR" << endl;
                    net->sendStringToAll(3, net->getID(), "HASH");
                }

                cout << numRes << endl;

                //Close connection
                net->reassembleAllSockets();
                break;
            case 6:
                bool flagValue;
                cout << "enter node ID" << endl;
                cin >> dest;
                net->getNode(dest)->getChangeFlag(&flagValue);
                cout << flagValue;
                break;
            default:
                break;
            }

            // net->sendString(2, ":exit");
            // net->recvString(2, response); //Hay que solucionar response
            // waitpid(-1, NULL, WNOHANG); //KILL ZOMBIE PROCESSES
        }
    }

    return 0;
}