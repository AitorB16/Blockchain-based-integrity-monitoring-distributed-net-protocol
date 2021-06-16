#include "main.hpp"
using namespace std;

/* Server thread method */
void *serverThread(void *arg)
{
    ;

    network *net = net->getInstance();

    server *s;
    s = new server(net);
    s->serverUP();

    pthread_exit(NULL);
}

/* Linker thread method */
void *linkerThread(void *arg)
{

    linker_process();

    pthread_exit(NULL);
}

/* Auditor thread method */
void *auditorThread(void *arg)
{

    network *net = net->getInstance();

    auditor *a;
    a = new auditor(net);
    a->auditorUP();

    pthread_exit(NULL);
}

/* Method to */
int updateSelfHash(int timeToWork)
{
    // double tsim = 0.0;
    // struct timespec t0, t1;

    int numRes;
    string currentHash;
    network *net = net->getInstance();

    // clock_gettime(CLOCK_REALTIME, &t0);

    /* Connect to trusted nodes */
    net->connectToAllNodes();

    /* Send request to the network */
    net->sendStringToAll(0, net->getSelfNode()->getID(), to_string(timeToWork));

    /* Wait 2/3 of network to send OK Select; timeout RESPONSE_DELAY_MAX */
    numRes = net->waitResponses(net->getNetNodeNumber() * THRESHOLD, RESPONSE_DELAY_MAX);

    // clock_gettime(CLOCK_REALTIME, &t1);

    // tsim = (t1.tv_sec - t0.tv_sec) * 1000 + (t1.tv_nsec - t0.tv_nsec) / (double)1e6;

    // cout << "Sending time: " << tsim << endl;

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

        /* Sleep the specified time, letting the interactor work */
        cout << "!!! You have " << timeToWork << " seconds, to work" << endl;
        sleep(timeToWork);

        if (net->getSelfNode()->getLastHash() != currentHash)
        {
            net->getSelfNode()->updateNodeBChain(net->getSelfNode()->getLastHash());
        }

        net->connectToAllNodes();

        /* Send new hash */
        net->sendStringToAll(1, net->getSelfNode()->getID(), net->getSelfNode()->getLastHash());

        if (EXEC_MODE == DEBUG_MODE)
            cout << "Hash sent - Hash: " << net->getSelfNode()->getLastHash() << endl;
        Logger("Hash sent - Hash: " + net->getSelfNode()->getLastHash());
        net->reassembleAllSockets();
        return 0;
    }
    /* Network is compromised */
    else
    {
        cout << "Network is compromised" << endl;
        Logger("Network is compromised");
        net->setNetworkToCompromised();
        net->reassembleAllSockets();
        return 1;
    }
}

int main(int argc, char *argv[])
{

    if (argc > 1)
        ID = atoi(argv[1]);

    pthread_t serverTid, auditorTid, linkerTid;

    network *net = net->getInstance();

    int option, dest;

    std::string buffer, response, input;

    /* Print network while start-up */
    if (EXEC_MODE == DEBUG_MODE)
    {
        cout << net->getSelfNode()->getID() << endl;
        net->printNetwork();
    }

    /* Launch the server */
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

    /* sleep random number to not overload the system + a minimum time to let the system deploy */
    sleep(getRandomNumber(TIME_SPACE_BEFORE_AUDIT) + TIME_SPACE_BEFORE_AUDIT / 3);

    //Sleep more to achieve network consisntency before launching the auditor */
    if (updateSelfHash(HASH_UPDATE_TIMESPACE_MAX) == 0)
    {
        /* Wait until the whole system is deployed ... + 1 + TOLERANCE */
        sleep(TIME_SPACE_BEFORE_AUDIT + TIME_SPACE_BEFORE_AUDIT / 3 + 1 + DEPLOY_TIME);

        /* Launch the auditor */
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

        cout << "Enter options:\n 0 - Close the program.\n 1 - Print network overview.\n 2 - Print specific info of a node.\n 3 - Update self hash, work-frame." << endl;
        cin >> option;

        switch (option)
        {
        /* Close infraestructure */
        case 0:
            cout << "Exiting infra" << endl;
            Logger("Exiting infra");
            exit(0);
            break;
        /* Print Network overview */
        case 1:
            if (net->isNetworkCompromised())
            {
                cout << "I'm not trusted by the network, not recieving updates anymore; my data is not valid" << endl;
                Logger("I'm not trusted by the network, not recieving updates anymore; my data is not valid");
            }
            net->printNetwork();
            break;
        /* Print detailed information of a given node */
        case 2:
            if (net->isNetworkCompromised())
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
                net->getNode(dest)->printTroublesomeHashList();
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
        /* Working time frame opening process */
        case 3:
            cout << "Enter the passwd to open a work-frame..." << endl;
            cin >> input;
            if (net->verifyPasswd(input))
            {
                if (!net->isNetworkCompromised())
                {
                    cout << "Enter time to work in seconds, MAX: " << HASH_UPDATE_TIMESPACE_MAX << " min: " << HASH_UPDATE_TIMESPACE_MIN << endl;
                    cin >> input;

                    /* pause auditor */
                    net->pauseAuditor();

                    if (updateSelfHash(atoi(input.c_str())) == 0)
                    {
                        /* resume auditor */
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
        default:
            break;
        }
    }
    return 0;
}