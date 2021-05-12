#include "network.hpp"

using namespace rapidxml;
using namespace std;

network *network::instance = 0;

//move XML to utils
network::network()
{
    //PARSE XML
    rapidxml::file<> xmlFile(CONFIG_FILE);
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

    //EXECUTION MODE
    EXEC_MODE = atoi(execMode_node->value());

    //PASSWD SHA256
    passwdSHA256 = passwdSHA256_node->value();

    //SELF NODE
    const char *ID_str = self_node->first_node("id")->value();
    int ID = atoi(ID_str);
    char *ip = self_node->first_node("ip")->value();
    int port = atoi(self_node->first_node("port")->value());
    CryptoPP::RSA::PublicKey pub = get_pub(ID_str);
    CryptoPP::RSA::PrivateKey prv = get_prv(ID_str);

    //Configurations for select
    FD_ZERO(&readfds);          //clear the socket set
    maxFD = -1;                 //initialize maxFD
    networkComprometed = false; //Network is trusted

    //init mutexes
    pthread_mutex_init(&lockTrustedNodeNumber, NULL);
    pthread_mutex_init(&lockNetworkComprometed, NULL);
    pthread_mutex_init(&lockMaxFD, NULL);

    try
    {

        self = new selfNode(ID, ip, port, pub, prv);

        //CREATE SERVER
        self->createServerSocket();
        //NETWORK NODES
        netNode *tmp_node;
        // adj_node_num = atoi(network_node->first_node("adj_node_num")->value());
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
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
    }
}

network *network::getInstance()
{
    if (!instance)
        instance = new network();
    return instance;
}

selfNode *network::getSelfNode()
{
    return self;
}

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

// int network::getID()
// {
//     return self->getID();
// }

int network::getNetNodeNumber()
{
    return netNodeNumber;
}
int network::getTrustedNodeNumber()
{
    int tmpTrustedNodeNumber = 0;
    pthread_mutex_lock(&lockTrustedNodeNumber);
    tmpTrustedNodeNumber = trustedNodeNumber;
    pthread_mutex_unlock(&lockTrustedNodeNumber);
    return tmpTrustedNodeNumber;
}

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

bool network::isNetworkComprometed()
{
    bool tmpNetworkComprometed;
    pthread_mutex_lock(&lockNetworkComprometed);
    tmpNetworkComprometed = networkComprometed;
    pthread_mutex_unlock(&lockNetworkComprometed);
    return tmpNetworkComprometed;
}
void network::setNetworkToComprometed()
{
    pthread_mutex_lock(&lockNetworkComprometed);
    networkComprometed = true;
    pthread_mutex_unlock(&lockNetworkComprometed);
}
bool network::verifyPasswd(string inPswd)
{
    if (passwdSHA256 == stream2hex(hashText(inPswd)))
        return true;
    else
        return false;
}
int network::getMaxFD()
{
    int tmpMaxFD;
    pthread_mutex_lock(&lockMaxFD);
    tmpMaxFD = maxFD;
    pthread_mutex_unlock(&lockMaxFD);
    return tmpMaxFD;
}

void network::setMaxFD(int fd)
{
    pthread_mutex_lock(&lockMaxFD);
    maxFD = fd;
    pthread_mutex_unlock(&lockMaxFD);
}

void network::printNetwork()
{
    cout << "Self Node ID: " << self->getID() << " # Hash: " << self->getLastHash() << " # Network is OK" << endl;
    for (auto &i : netNodes)
    {
        if (i->isTrusted())
            std::cout << "TRUSTED - Node ID: " << i->getID() << " # Hash: " << i->getLastHash() << " # BCHash: " << i->getLastNodeBChain() << endl;
        else
            std::cout << "NOT TRUSTED - Node ID: " << i->getID() << " # Hash: " << i->getLastConflictiveHash() << " # BCHash: " << i->getLastNodeBChain() << endl;
    }
}

bool network::connectToAllNodes()
{
    int tmpMFD;
    try
    {
        pthread_mutex_lock(&lockMaxFD);
        tmpMFD = maxFD;
        pthread_mutex_unlock(&lockMaxFD);

        //No other global connections active
        if (tmpMFD == -1)
        {
            //To prevent other threads to enter a num !=-1 and release mutex
            pthread_mutex_lock(&lockMaxFD);
            maxFD = 0;
            pthread_mutex_unlock(&lockMaxFD);

            for (auto &i : netNodes)
            {
                if (i->isTrusted() && !i->isConnected()) //&& !i->getChangeFlag())
                {
                    if (i->estConnection() == -1)
                    {
                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Error connection: " << i->getID() << endl;
                        Logger("Error connection: " + to_string(i->getID()));
                        //Decrease confidence if node is not avalible
                        i->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
                    }
                    else
                    {
                        //Add sockets for select
                        FD_SET(i->getSock(), &readfds);
                        if (i->getSock() > maxFD)
                        {
                            maxFD = i->getSock();
                        }
                        // cout << "Success connection: " << i->getID() << endl;
                    }
                }
            }
            return true;
        }
        //Network busy, sockets no reassembled
        else
        {
            return false;
        }
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
        return false;
    }
}

bool network::connectToNode(int ID)
{
    try
    {
        for (auto &i : netNodes)
        {
            if (i->getID() == ID)
            {
                if (i->isTrusted() && !i->isConnected())
                {
                    if (i->estConnection() == -1)
                    {
                        if (EXEC_MODE == DEBUG_MODE)
                            cout << "Error connection: " << i->getID() << endl;
                        Logger("Error connection: " + to_string(i->getID()));
                        //Decrease confidence if node is not avalible
                        i->decreaseTrustLvlIn(TRUST_DECREASE_CONST);
                    }
                    else
                    {
                        // cout << "Success connection: " << ID << endl;
                        return true;
                    }
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

//If we are at this point, node is trusted
void network::reassembleSocket(int ID)
{
    try
    {
        for (auto &i : netNodes)
            if (i->getID() == ID)
                // if (i->isTrusted())
                i->resetClientSocket();
    }
    catch (const std::exception &e)
    {
        if (EXEC_MODE == DEBUG_MODE)
            cerr << e.what() << '\n';
        Logger(e.what());
    }
}

//Be careful, maybe we need an int intead a bool for isConnected()
void network::reassembleAllSockets()
{
    try
    {
        for (auto &i : netNodes)
        {
            if (i->isConnected()) // i->isTrusted() &&
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

bool network::validateMsg(int selfID, int clientID, int syncNumReceived, string MsgToVerify, string MsgSignature)
{
    //Verify if msg is for me
    int syncNumStored;
    netNode *nN = getNode(clientID);

    if (selfID == self->getID())
    {
        //Verify if sync number is correct
        syncNumStored = nN->getSyncNum();

        //Verify if msg is correctly signed
        if (verify(MsgToVerify, hex2stream(MsgSignature), to_string(clientID)))
        {
            //Standard situation
            if (syncNumReceived == syncNumStored)
            {
                //Increment sync number
                nN->setSyncNum(nN->getSyncNum() + 1);
                //Verification successful
                return true;
            }
            //Package lost occured, but greater values are valid -> resync
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

bool network::sendString(int code, int destID, int sourceID, string content)
{
    std::string buffer, msg, signedMsg, hexMsg;
    int syncNum;

    try
    {
        for (auto &i : netNodes)
        {
            if (i->getID() == destID && i->isTrusted() && i->isConnected())
            {

                //Get and increment Sync Number
                syncNum = i->getSyncNum();

                //Specify  msgcode + sourceID + destinationID + syncNum + content
                msg = to_string(code) + ";" + to_string(sourceID) + ";" + to_string(i->getID()) + ";" + to_string(syncNum) + ";" + content;

                signedMsg = sign(msg, std::to_string(sourceID));
                hexMsg = stream2hex(signedMsg);
                msg = msg + ";" + hexMsg + ";";
                buffer = msg;
                if (i->sendString(buffer.c_str()) == -1)
                {
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Error sending: " << i->getID() << endl;
                    Logger("Error sending: " + to_string(i->getID()));
                }
                else
                {
                    // cout << "Success sending: " << destID << endl;
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

void network::sendStringToAll(int code, int sourceID, string content)
{
    std::string buffer, msg, signedMsg, hexMsg;
    int syncNum;

    try
    {
        // random = gen_urandom(RANDOM_STR_LEN);
        for (auto &i : netNodes)
        {
            if (i->isTrusted() && i->isConnected()) //&& !i->getChangeFlag())
            {

                //Get and increment Sync Number
                syncNum = i->getSyncNum();

                //Specify  msgcode + sourceID + destinationID + syncNum + content
                msg = to_string(code) + ";" + to_string(sourceID) + ";" + to_string(i->getID()) + ";" + to_string(syncNum) + ";" + content;

                //PROBLEMS SINGING OVER SIGN, NEED HASHING???

                signedMsg = sign(msg, std::to_string(sourceID));
                hexMsg = stream2hex(signedMsg);
                msg = msg + ";" + hexMsg + ";";
                buffer = msg;

                if (i->sendString(buffer.c_str()) == -1)
                {
                    if (EXEC_MODE == DEBUG_MODE)
                        cout << "Error sending: " << i->getID() << endl;
                    Logger("Error sending: " + i->getID());
                }
                else
                {
                    // cout << "Success sending: " << i->getID() << endl;
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

int network::waitResponses(int resNum, int select_time)
{
    struct timeval tv;
    tv.tv_sec = select_time;
    tv.tv_usec = 0;

    int selectStatus;
    int counter = 0;
    fd_set tmpFdSet;

    int tmpMaxFD;

    //Just one node left
    // if (resNum == 0)
    //     return 1;

    while (1)
    {
        //just monitorize trusted sockets; low-eq maxFD descriptor
        selectStatus = select(maxFD + 1, &readfds, NULL, NULL, &tv);

        counter += selectStatus;

        //if timeout or received message number is gr eq to resNum -> break the loop
        if (selectStatus == 0 || counter >= resNum)
            break;
        else
        {
            //tmp vars for reseting values
            FD_ZERO(&tmpFdSet); //clear the socket set
            tmpMaxFD = -1;      //initialize tmpMaxFD

            //Count all received connections
            for (auto &i : netNodes)
            {
                if (i->isTrusted() && i->isConnected())
                {
                    if (!FD_ISSET(i->getSock(), &readfds))
                    {
                        if (i->getSock() > tmpMaxFD)
                            tmpMaxFD = i->getSock();
                        FD_SET(i->getSock(), &tmpFdSet);
                    }
                }
            }
            //Reset values for select
            readfds = tmpFdSet;
            maxFD = tmpMaxFD;
        }
    }
    return counter;
}

int network::getTrustedRandomNode()
{
    int random = -1;
    //If no enough nodes are trusted return -1
    if (trustedNodeNumber < netNodeNumber * THRESHOLD)
        return random;

    //If there are trusted nodes, pick one among them
    while (1)
    {
        //Self node must be counted as well +1; first node starts at 1 -> +1
        random = get_randomNumber(netNodeNumber + 1 + 1);
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

void network::resetTrustLvl()
{
    for (auto &i : netNodes)
    {
        i->resetTrustLvl();
    }
}