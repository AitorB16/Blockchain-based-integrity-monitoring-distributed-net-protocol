#include "main.hpp"
using namespace std;

void *serverThread(void *arg)
{
    // struct argNetwork *args = (struct argNetwork *)arg;
    // network *net = args->net;

    network *net = net->getInstance();

    server *s;
    s = new server(net);
    s->serverUP();

    pthread_exit(NULL);
}

//Linker thread start
void *linkerThread(void *arg)
{
    // struct argNetwork *args = (struct argNetwork *)arg;
    // network *net = args->net;

    thread_process();

    pthread_exit(NULL);

    //dbus_error:
    //      sd_bus_slot_unref(slot);
    //      sd_bus_unref(bus);
    //      return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

//Auditor thread start
void *auditorThread(void *arg)
{
    // struct argNetwork *args = (struct argNetwork *)arg;
    // network *net = args->net;

    network *net = net->getInstance();

    auditor *a;
    a = new auditor(net);
    a->auditorUP();

    pthread_exit(NULL);
}

int updateSelfHash(int timeToWork)
{
    int numRes;
    string currentHash;
    network *net = net->getInstance();

    // if (net->connectToAllNodes())
    // {
    net->connectToAllNodes();
    //Send request to ALL

    //Just send to connected nodes
    net->sendStringToAll(0, net->getSelfNode()->getID(), to_string(timeToWork));

    //Wait 2/3 of network to send OK Select; timeout RESPONSE_DELAY_MAX
    numRes = net->waitResponses(net->getNetNodeNumber() * THRESHOLD, RESPONSE_DELAY_MAX);

    if (EXEC_MODE == DEBUG_MODE)
        cout << "Num replies: " << numRes << endl;

    if (numRes >= net->getNetNodeNumber() * THRESHOLD)
    {
        net->reassembleAllSockets();

        if (timeToWork > HASH_UPDATE_TIMESPACE_MAX || timeToWork < HASH_UPDATE_TIMESPACE_MIN)
        {
            timeToWork = HASH_UPDATE_TIMESPACE_MAX;
        }
        currentHash = net->getSelfNode()->getLastHash();
        //In this part just sleep for DEF_TIMER_WAIT, and let the hashUpdate thread work
        cout << "!!! You have " << timeToWork << " seconds, to work" << endl;
        // cout << "Enter new hash value: " << endl;
        // cin >> input;
        // Logger("Hash updated: " + input);
        // net->getSelfNode()->updateHashList(input);
        sleep(timeToWork);

        //Some work is done
        if (net->getSelfNode()->getLastHash() != currentHash)
        {
            net->getSelfNode()->updateNodeBChain(net->getSelfNode()->getLastHash());
        }

        net->connectToAllNodes();

        //Send hash
        net->sendStringToAll(1, net->getSelfNode()->getID(), net->getSelfNode()->getLastHash());

        if (EXEC_MODE == DEBUG_MODE)
            cout << "Hash sent - Hash: " << net->getSelfNode()->getLastHash() << endl;
        Logger("Hash sent - Hash: " + net->getSelfNode()->getLastHash());
        net->reassembleAllSockets();
        return 0;
    }
    //Network is comprometed
    else
    {
        cout << "Network is comprometed" << endl;
        Logger("Network is comprometed");
        net->setNetworkToComprometed();
        net->reassembleAllSockets();
        return 1;
    }
    // }
    // else
    // {
    //     cout << "Network is busy, try again later" << endl;
    //     Logger("Network is busy, try again later");
    //     return 1;
    // }
}

int main(void)
{

    pthread_t serverTid, auditorTid, linkerTid;

    network *net = net->getInstance();

    // argNetwork args;
    // args.net = net;

    int option, dest;

    std::string buffer, response, input;

    //Print other nodes
    if (EXEC_MODE == DEBUG_MODE)
    {
        cout << net->getSelfNode()->getID() << endl;
        net->printNetwork();
    }

    //Launch the server
    //pthread_create(&serverTid, NULL, serverThread, (void *)&args)
    if (pthread_create(&serverTid, NULL, serverThread, NULL) != 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Error creating server thread" << endl;
        Logger("Error creating server thread");
        exit(1);
    }

    //Launch dbus linker
    if (pthread_create(&linkerTid, NULL, linkerThread, NULL) != 0)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cout << "Error creating linker thread" << endl;
        Logger("Error creating linker thread");
        exit(1);
    }

    cout << "Deploying the network... wait please" << endl;

    //sleep random number to not overload the system + a minimum time to let the system deploy
    sleep(getRandomNumber(TIME_SPACE_BEFORE_AUDIT) + DEPLOY_TIME + TIME_SPACE_BEFORE_AUDIT / 3);

    //Sync method to achieve consisntency before launching the auditor
    if (updateSelfHash(HASH_UPDATE_TIMESPACE_MAX) == 0)
    {
        //Wait until the whole system is deployed ... + 1 + TOLERANCE
        sleep(TIME_SPACE_BEFORE_AUDIT + TIME_SPACE_BEFORE_AUDIT / 3 + 1);

        //Launch the auditor
        if (pthread_create(&auditorTid, NULL, auditorThread, NULL) != 0)
        {
            if (EXEC_MODE == DEBUG_MODE)
                cout << "Error creating auditor thread" << endl;
            Logger("Error creating auditor thread");
            exit(1);
        }
    }

    while (1)
    {

        // if (EXEC_MODE == DEBUG_MODE || EXEC_MODE == INTERACTIVE_MODE)
        // {

        cout << "Enter options" << endl;
        cin >> option;

        switch (option)
        {
        //Close infraestructure
        case 0:
            cout << "Exiting infra" << endl;
            Logger("Exiting infra");
            exit(0);
            break;
        //Print Network
        case 1:
            if (net->isNetworkComprometed())
            {
                cout << "I'm not trusted by the network, not recieving updates anymore; my data is not valid" << endl;
                Logger("I'm not trusted by the network, not recieving updates anymore; my data is not valid");
            }
            net->printNetwork();
            break;
        //BCAST request to send + timeout + count
        case 2:
            //Wait for user interaction
            cout << "Enter the passwd to open a work-frame..." << endl;
            cin >> input;
            if (net->verifyPasswd(input))
            {
                if (!net->isNetworkComprometed())
                {
                    cout << "Enter time to work in seconds, MAX: " << HASH_UPDATE_TIMESPACE_MAX << endl;
                    cin >> input;

                    //pause auditor
                    net->pauseAuditor();

                    if (updateSelfHash(atoi(input.c_str())) == 0)
                    {
                        //resume auditor
                        net->resumeAuditor();
                    }
                }
                else
                {
                    cout << "I'm not trusted by the network, cant send data" << endl;
                    Logger("I'm not trusted by the network, cant send data");
                }
            }
            else
            {
                cout << "Invalid passwd" << endl;
                Logger("Invalid passwd");
            }
            break;
        case 3:
            if (net->isNetworkComprometed())
            {
                cout << "I'm not trusted by the network, not recieving updates anymore; my data is not valid" << endl;
                Logger("I'm not trusted by the network, not recieving updates anymore; my data is not valid");
            }
            cout << "enter node ID" << endl;
            cin >> dest;
            if (dest != net->getSelfNode()->getID())
            {
                cout << endl;
                cout << "BLOCKCHAIN RECORD" << endl;
                net->getNode(dest)->printNodeBchain();
                cout << endl;
                cout << "GOOD HASH RECORD" << endl;
                net->getNode(dest)->printHashList();
                cout << endl;
                cout << "BAD HASH RECORD" << endl;
                net->getNode(dest)->printConflictiveHashList();
                cout << endl;
            }
            else
            {
                cout << endl;
                cout << "BLOCKCHAIN RECORD" << endl;
                net->getSelfNode()->printNodeBchain();
                cout << endl;
                cout << "HASH RECORD" << endl;
                net->getSelfNode()->printHashList();
                cout << endl;
            }
            break;

        default:
            break;
        }
    }
    return 0;
}