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
    s->serverUP();

    pthread_exit(NULL);
}

//Auditor thread start
void *auditorThread(void *arg)
{
    struct argNetwork *args = (struct argNetwork *)arg;

    network *net = args->net;

    auditor *a;
    a = new auditor(net);
    a->auditorUP();

    pthread_exit(NULL);
}

int main(void)
{

    pthread_t serverTid, auditorTid;

    network *net = net->getInstance();

    argNetwork args;
    args.net = net;

    int numRes;

    //Print other nodes
    if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
    {
        cout << net->getID() << endl;
        net->printNetwork();
    }

    //Launch the server
    if (pthread_create(&serverTid, NULL, serverThread, (void *)&args) != 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Error creating server thread" << endl;
        exit(1);
    }

    //Launch the auditor
    if (pthread_create(&serverTid, NULL, auditorThread, (void *)&args) != 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Error creating auditor thread" << endl;
        exit(1);
    }

    //Launch thread updating selfhash

    //SYNC NUMBERS REQ

    // const char *response;
    int option;

    std::string buffer, response;

    string input;
    int dest;

    // netNode nN;

    // string IDNodeHash;
    // bool flagValue;

    //WE NEED A SYNC METHOD
    sleep(5);

    while (1)
    {

        if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
        {

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
                if (net->isNetworkComprometed())
                {
                    cout << "I'm not trusted by the network, not recieving updates anymore; my data is not valid" << endl;
                }
                else
                {
                    net->printNetwork();
                }
                break;
            //BCAST request to send + timeout + count
            case 2:
                if (!net->isNetworkComprometed())
                {
                    //pause auditor
                    net->getSelfNode()->setChangeFlag(true);

                    if (EXEC_MODE == INTERACTIVE_MODE || EXEC_MODE == DEBUG_MODE)
                        cout << "Stopping auditor..." << endl;

                    //WAIT FOR AUDITOR_INTERVAL SECONDS TO PAUSE AUDITOR CORRECTLY
                    sleep(AUDITOR_INTERVAL + 1);

                    //WAIT UNTIL NETWORK IS FULLY OPERATIONAL?

                    //Connect to ALL
                    if (net->connectToAllNodes())
                    {
                        //Send request to ALL

                        //Just send to connected nodes
                        net->sendStringToAll(0, net->getID());

                        //Wait 2/3 of network to send OK Select; timeout RESPONSE_DELAY_MAX
                        numRes = net->waitResponses(net->getTrustedNodeNumber() * THRESHOLD, RESPONSE_DELAY_MAX);

                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "NUM RES: " << numRes << endl;

                        if (numRes >= net->getTrustedNodeNumber() * THRESHOLD)
                        {
                            net->reassembleAllSockets();

                            //In this part just sleep for DEF_TIMER_WAIT, and let the hashUpdate thread work
                            cout << "!!! You have " << HASH_UPDATE_TIMESPACE << " seconds, to work" << endl;
                            cout << "Enter new hash value: " << endl;
                            cin >> input;
                            net->getSelfNode()->updateHashList(input);

                            net->connectToAllNodes();

                            //Send hash
                            net->sendStringToAll(1, net->getID(), net->getSelfNode()->getLastHash());
                            cout << "hash sent" << endl;
                        }
                        //Network is comprometed
                        else
                        {
                            cout << "Network is comprometed" << endl;
                            net->setNetworkToComprometed();
                        }

                        //Close connection
                        net->reassembleAllSockets();
                    }
                    else
                    {
                        //MaxFD !=-1 or error connecting to node
                        cout << "Network is busy, try again later" << endl;
                    }
                }
                else
                {
                    cout << "I'm not trusted by the network, cant send data" << endl;
                }

                //resume auditor (in future we will need a thread)
                net->getSelfNode()->setChangeFlag(false);
                break;
            //Update selfhash manualy
            case 3:
                if (net->isNetworkComprometed())
                {
                    cout << "I'm not trusted by the network, not recieving updates anymore; my data is not valid" << endl;
                }
                else
                {
                    cout << "enter new hash" << endl;
                    cin >> input;
                    net->getSelfNode()->updateHashList(input);
                }
                break;
            //Print hash record and conflictive hash record
            case 4:
                if (net->isNetworkComprometed())
                {
                    cout << "I'm not trusted by the network, not recieving updates anymore; my data is not valid" << endl;
                }
                else
                {
                    cout << "enter node ID" << endl;
                    cin >> dest;
                    if (dest != net->getSelfNode()->getID())
                    {
                        cout << endl
                             << "GOOD HASH RECORD" << endl;
                        net->getNode(dest)->printHashList();
                        cout << endl
                             << "BAD HASH RECORD" << endl;
                        net->getNode(dest)->printConflictiveHashList();
                        cout << endl;
                    }
                }
                break;

            default:
                break;
            }
        }
        //Prevent silent mode burning resources
        if (EXEC_MODE == SILENT_MODE)
            sleep(TIME_SPACE_BEFORE_AUDIT);
        //Alternatively, we could add a cin waiting to code 0 - exit
    }

    return 0;
}