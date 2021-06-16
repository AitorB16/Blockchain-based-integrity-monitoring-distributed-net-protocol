#include "network.hpp"

using namespace rapidxml;
using namespace std;

network *network::instance = 0;

/* Private constructor */
network::network()
{
    /* config.xml parsing */
    string path = string(CONFIG_FOLDER) + "config" + ".xml";
    if (ID != -1)
        path = string(CONFIG_FOLDER) + "config" + to_string(ID) + ".xml";

    rapidxml::file<> xmlFile(path.c_str());
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<> *root_node;
    rapidxml::xml_node<> *execMode_node;
    rapidxml::xml_node<> *passwdSHA256_node;
    rapidxml::xml_node<> *self_node;
    rapidxml::xml_node<> *network_node;

    doc.parse<0>(xmlFile.data());
    root_node = doc.first_node("config");
    execMode_node = root_node->first_node("execution_mode");
    passwdSHA256_node = root_node->first_node("passwd_sha256");
    self_node = root_node->first_node("node_self");
    network_node = root_node->first_node("network");

    /* Execution mode */
    EXEC_MODE = atoi(execMode_node->value());

    /* SHA256 resume hash */
    passwdSHA256 = passwdSHA256_node->value();

    /* Self node info */
    const char *ID_str = self_node->first_node("id")->value();
    int ID = atoi(ID_str);
    char *ip = self_node->first_node("ip")->value();
    int port = atoi(self_node->first_node("port")->value());
    CryptoPP::RSA::PublicKey pub = get_pub(ID_str);
    CryptoPP::RSA::PrivateKey prv = get_prv(ID_str);

    /* Clear Select() parameters */
    FD_ZERO(&readfds);          //clear the socket set
    maxFD = -1;                 //initialize maxFD
    networkCompromised = false; //Network is trusted

    /* Init mutexes */
    pthread_mutex_init(&lockTrustedNodeNumber, NULL);
    pthread_mutex_init(&lockNetworkCompromised, NULL);
    try
    {

        self = new selfNode(ID, ip, port, pub, prv);

        /* Create server structure */
        self->createServerSocket();

        /* Create structure of network nodes */
        netNode *tmp_node;
        netNodeNumber = 0;

        for (xml_node<> *adj = network_node->first_node("node"); adj; adj = adj->next_sibling())
        {
            ID_str = adj->first_node("id")->value();
            ID = atoi(ID_str);
            ip = adj->first_node("ip")->value();
            port = atoi(adj->first_node("port")->value());
            pub = get_pub(ID_str);
            tmp_node = new netNode(ID, ip, port, pub);
            tmp_node->createClientSocket();
            netNodes.push_back(tmp_node);
            netNodeNumber++;
        }

        trustedNodeNumber = netNodeNumber;

        /* Set dynamic GLOBALS dependent of netNodeNumber  */

        TRUST_LEVEL = netNodeNumber * THRESHOLD;

        TIME_RESET_INCIDENTS = (2 * netNodeNumber + netNodeNumber / 2) * AUDITOR_INTERVAL;

        MAX_INCIDENTS = netNodeNumber / 3;
        if (MAX_INCIDENTS < 1)
            MAX_INCIDENTS = 1;

        for (auto &i : netNodes)
        {
            i->setTrustLvl(TRUST_LEVEL);
        }
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
    }
}

/* Singleton public getInstance */
network *network::getInstance()
{
    if (!instance)
        instance = new network(); /* Call constructor if no instance created */
    return instance;
}

/* Get self node */
selfNode *network::getSelfNode()
{
    return self;
}

/* Get node corresponding to a given ID */
netNode *network::getNode(int ID)
{
    for (auto &i : netNodes)
    {
        if (i->getID() == ID)
        {
            return i;
        }
    }
    return NULL;
}

/* Get node number in network */
int network::getNetNodeNumber()
{
    return netNodeNumber; /* Self node is not counted */
}

/* Get trusted node numner */
int network::getTrustedNodeNumber()
{
    int tmpTrustedNodeNumber = 0;
    pthread_mutex_lock(&lockTrustedNodeNumber);
    tmpTrustedNodeNumber = trustedNodeNumber;
    pthread_mutex_unlock(&lockTrustedNodeNumber);
    return tmpTrustedNodeNumber;
}

/* Check nodes' tust lvl to update trsted node number */
void network::updateTrustedNodeNumber()
{
    int tmpTrustNum = 0;
    for (auto &i : netNodes)
    {
        if (i->isTrusted())
            tmpTrustNum++;
    }
    pthread_mutex_lock(&lockTrustedNodeNumber);
    trustedNodeNumber = tmpTrustNum;
    pthread_mutex_unlock(&lockTrustedNodeNumber);
}

/* Return network trust status */
bool network::isNetworkCompromised()
{
    bool tmpNetworkCompromised;
    pthread_mutex_lock(&lockNetworkCompromised);
    tmpNetworkCompromised = networkCompromised;
    pthread_mutex_unlock(&lockNetworkCompromised);
    return tmpNetworkCompromised;
}

/* Set network to compromised */
void network::setNetworkToCompromised()
{
    pthread_mutex_lock(&lockNetworkCompromised);
    networkCompromised = true;
    pthread_mutex_unlock(&lockNetworkCompromised);
}

/* Print network overview */
void network::printNetwork()
{
    cout << "Self Node ID: " << self->getID() << " # Hash: " << self->getLastHash() << " # Network is OK" << endl;
    for (auto &i : netNodes)
    {
        if (i->isTrusted())
            std::cout << "TRUSTED - Node ID: " << i->getID() << " # Hash: " << i->getLastHash() << " # BCHash: " << i->getLastNodeBChain() << endl;
        else
            std::cout << "NOT TRUSTED - Node ID: " << i->getID() << " # Hash: " << i->getLastTroublesomeHash() << " # BCHash: " << i->getLastNodeBChain() << endl;
    }
}

/* Verify input psswd by hashing it */
bool network::verifyPasswd(string inPswd)
{
    if (passwdSHA256 == hashText(inPswd))
        return true;
    else
        return false;
}

/* Connect to all trusted nodes by iteration */
bool network::connectToAllNodes()
{
    try
    {
        /* maxFD is -1 always at this point */
        for (auto &i : netNodes)
        {
            if (i->isTrusted())
            {
                if (i->estConnection() == -1)
                {
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Error connection: " << i->getID() << endl;
                    Logger("Error connection: " + to_string(i->getID()));
                    /* Decrease confidence if node is not avalible */
                    i->increaseIncidentNum(INCIDENT_INCREASE);
                }
                else
                {
                    /* Add sockets for select */
                    FD_SET(i->getSock(), &readfds);
                    if (i->getSock() > maxFD)
                    {
                        maxFD = i->getSock();
                    }
                }
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
        return false;
    }
}

/* Connect to a given ID trusted node */
bool network::connectToNode(int ID)
{
    try
    {
        for (auto &i : netNodes)
        {
            if (i->getID() == ID && i->isTrusted())
            {
                if (i->estConnection() == -1)
                {
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Error connection: " << i->getID() << endl;
                    Logger("Error connection: " + to_string(i->getID()));
                    /* Decrease confidence if node is not avalible */
                    i->increaseIncidentNum(INCIDENT_INCREASE);
                }
                else
                {
                    return true;
                }
            }
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

/* Reassemble a node ID socekt after closing connection */
void network::reassembleSocket(int ID)
{
    try
    {
        for (auto &i : netNodes)
            if (i->getID() == ID)
                i->resetClientSocket();
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
    }
}

/* Reassemble all nodes socekts after closing connection */
void network::reassembleAllSockets()
{
    try
    {
        for (auto &i : netNodes)
        {
            if (i->isConnected())
            {
                i->resetClientSocket();
            }
        }
        FD_ZERO(&readfds); //clear the socket set
        maxFD = -1;        //initialize maxFD
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
    }
}

/* Message validation, given self ID, sender ID, received syncNum, received datagram without signature and received signature */
bool network::validateMsg(int selfID, int clientID, int syncNumReceived, string MsgToVerify, string MsgSignature)
{
    /* Verify if msg is for me */
    int syncNumStored;
    netNode *nN = getNode(clientID);

    if (selfID == self->getID())
    {
        /* Verify if sync number is correct */
        syncNumStored = nN->getSyncNum();

        /* Verify if msg is correctly signed */
        if (verify(MsgToVerify, MsgSignature, to_string(clientID)))
        {
            /* Standard situation */
            if (syncNumReceived == syncNumStored)
            {
                /* Increment sync number */
                nN->setSyncNum(nN->getSyncNum() + 1);
                /* Verification successful */
                return true;
            }
            /* Package lost occured, but greater values are valid -> resync */
            else if (syncNumReceived > syncNumStored)
            {
                nN->setSyncNum(syncNumReceived + 1);
                if (EXEC_MODE == DEBUG_MODE)
                    cout << "Package lost, resyncing - ID: " << clientID << endl;
                Logger("Package lost, resyncing - ID: " + to_string(clientID));
                return true;
            }
        }
    }
    return false;
}

/* Elaborate and send a datagram to a given ID destination node */
bool network::sendString(int code, int destID, int sourceID, string content)
{
    std::string buffer, msg, signedMsg;
    int syncNum;

    try
    {
        for (auto &i : netNodes)
        {
            if (i->getID() == destID && i->isConnected())
            {

                /* Get Sync Number */
                syncNum = i->getSyncNum();

                /* Specify  msgcode + sourceID + destinationID + syncNum + content */
                msg = to_string(code) + ";" + to_string(sourceID) + ";" + to_string(i->getID()) + ";" + to_string(syncNum) + ";" + content;

                signedMsg = sign(msg, std::to_string(sourceID));
                msg = msg + ";" + signedMsg + ";";
                buffer = msg;
                if (i->sendString(buffer.c_str()) == -1)
                {
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Error sending: " << i->getID() << endl;
                    Logger("Error sending: " + to_string(i->getID()));
                }
                else
                {
                    /* Increment Sync Number */
                    i->setSyncNum(i->getSyncNum() + 1);
                    return true;
                }
            }
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

/* Elaborate and send a datagram to all trusted nodes */
void network::sendStringToAll(int code, int sourceID, string content)
{
    std::string buffer, msg, signedMsg;
    int syncNum;

    try
    {
        for (auto &i : netNodes)
        {
            if (i->isConnected())
            {

                /* Get Sync Number */
                syncNum = i->getSyncNum();

                /* Specify  msgcode + sourceID + destinationID + syncNum + content */
                msg = to_string(code) + ";" + to_string(sourceID) + ";" + to_string(i->getID()) + ";" + to_string(syncNum) + ";" + content;

                signedMsg = sign(msg, std::to_string(sourceID)); //IF PROBLEMS SINGING OVER SIGN; HASH AND THEN SIGN
                msg = msg + ";" + signedMsg + ";";
                buffer = msg;

                if (i->sendString(buffer.c_str()) == -1)
                {
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Error sending: " << i->getID() << endl;
                    Logger("Error sending: " + i->getID());
                }
                else
                {
                    /* Increment Sync Number */
                    i->setSyncNum(i->getSyncNum() + 1);
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
    }
}

/* Wait resNum responses to arrive in selectTime seconds */
int network::waitResponses(int resNum, int selectTime)
{
    struct timeval tv;
    tv.tv_sec = selectTime;
    tv.tv_usec = 0;

    int selectStatus;
    int counter = 0;
    fd_set tmpFdSet;

    int tmpMaxFD;

    while (1)
    {
        /* Just monitorize trusted sockets; low-eq maxFD descriptor */
        selectStatus = select(maxFD + 1, &readfds, NULL, NULL, &tv);

        counter += selectStatus;

        /* If timeout or received message number is gr eq to resNum -> break the loop */
        if (selectStatus == 0 || counter >= resNum)
            break;
        else
        {
            /* tmp vars for reseting values */
            FD_ZERO(&tmpFdSet); //clear the socket set
            tmpMaxFD = -1;      //initialize tmpMaxFD

            /* Count all received connections */
            for (auto &i : netNodes)
            {
                if (i->isConnected())
                {
                    if (!FD_ISSET(i->getSock(), &readfds))
                    {
                        if (i->getSock() > tmpMaxFD)
                            tmpMaxFD = i->getSock();
                        FD_SET(i->getSock(), &tmpFdSet);
                    }
                }
            }
            /* Reset values for select */
            readfds = tmpFdSet;
            maxFD = tmpMaxFD;
        }
    }
    return counter;
}

/* Get a trusted random node ID */
int network::getTrustedRandomNode()
{
    int random = -1;
    /* If no enough nodes are trusted return -1 */
    if (trustedNodeNumber < netNodeNumber * THRESHOLD)
        return random;

    /* If there are trusted nodes, pick one among them */
    while (1)
    {
        /* Self node must be counted as well +1; first node starts at 1 -> +1 */
        random = getRandomNumber(netNodeNumber + 1 + 1);
        for (auto &i : netNodes)
        {
            if (i->getID() == random)
            {
                if (i->isTrusted() && !i->getChangeFlag())
                {
                    return random;
                }
            }
        }
    }
}

/* Call reset incident number of all nodes */
void network::resetincidentNum()
{
    for (auto &i : netNodes)
    {
        i->resetincidentNum();
    }
}

/* Pause auditor by setting self change flag to true */
void network::pauseAuditor()
{
    self->setChangeFlag(true);
    cout << "Pausing auditor..." << endl;
    Logger("Pausing auditor...");

    /* Wait for AUDITOR_INTERVAL + 1 seconds to pause auditor correctly */
    sleep(AUDITOR_INTERVAL + 1);
}

/* Resume auditor by setting self change flag to false */
void network::resumeAuditor()
{
    self->setChangeFlag(false);
    if (EXEC_MODE == DEBUG_MODE)
        cout << "Auditor resumed" << endl;
    Logger("Auditor resumed");
}