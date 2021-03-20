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
    net->printNetwork();

    if (pthread_create(&serverTid, NULL, serverThread, (void *)&args) != 0)
    {
        printf("Error");
        exit(1);
    }

    // const char *response;
    int option;

    std::string buffer, response;
    int dest;

    string IDNodeHash;
    bool flagValue;

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
        //Print Network
        case 1:
            net->printNetwork();
            break;
        //BCAST request to send + timeout + count
        case 2:
            if (!net->isNetworkComprometed())
            {
                //Connect to ALL
                net->connectToAllNodes();

                //Send request to ALL

                //Enviar solo a los nodos conectados //Code 0 REQUEST
                net->sendStringToAll(0, net->getID());

                //Wait 2/3 of network to send OK Select; timeout 30sec
                numRes = net->waitResponses(net->getTrustedNodeNumber() * THRESHOLD);

                cout << "NUM RES: " << numRes << endl;

                if (numRes >= net->getTrustedNodeNumber() * THRESHOLD)
                {
                    //Send hash
                    net->sendStringToAll(1, net->getID(), "HASH");

                    cout << "hash sent" << endl;
                }
                //Network is comprometed
                else
                {
                    cout << "Network is comprometed" << endl;
                    net->setNetworkToComprometed();
                    // exit(0);
                }

                //Close connection
                net->reassembleAllSockets();
            }
            break;
        case 3:
            cout << "enter node ID" << endl;
            cin >> dest;
            flagValue = net->getNode(dest)->getChangeFlag();
            cout << flagValue << endl;
            break;
        case 4:
            cout << "enter node ID" << endl;
            cin >> dest;
            IDNodeHash = net->getNode(dest)->getCurrentHash();
            cout << IDNodeHash << endl;
            break;
        //Print Network
        case 5:
            net->printNetwork();
            break;
        default:
            break;
        }
    }

    return 0;
}