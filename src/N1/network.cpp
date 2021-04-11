#include "network.hpp"

using namespace rapidxml;
using namespace std;

// pthread_mutex_t lockMsgList = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockNetworkComprometed = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockTrustedNodeNumber = PTHREAD_MUTEX_INITIALIZER;

network *network::instance = 0;

network::network()
{
    //PARSE XML
    rapidxml::file<> xmlFile("config.xml");
    rapidxml::xml_document<> doc;
    rapidxml::xml_node<> *root_node;
    rapidxml::xml_node<> *self_node;
    rapidxml::xml_node<> *network_node;

    doc.parse<0>(xmlFile.data());
    root_node = doc.first_node("config");

    self_node = root_node->first_node("node_self");
    network_node = root_node->first_node("network");

    //SELF NODE
    const char *ID_str = self_node->first_node("id")->value();
    int ID = atoi(ID_str);
    const char *ip = self_node->first_node("ip")->value();
    int port = atoi(self_node->first_node("port")->value());
    CryptoPP::RSA::PublicKey pub = get_pub(ID_str);
    CryptoPP::RSA::PrivateKey prv = get_prv(ID_str);

    //Configurations for select
    FD_ZERO(&readfds);          //clear the socket set
    maxFD = -1;                 //initialize maxFD
    networkComprometed = false; //Network is trusted

    try
    {

        self = new selfNode(ID, ip, port, pub, prv, "c54008913c9085a5a5b322e1f8eb050b843874c5d00811e1bfba2e9bbbb15a4b");

        //CREATE SERVER
        self->createServerSocket();
        //ADJACENT NODES
        simpleNode *tmp_node;
        // adj_node_num = atoi(network_node->first_node("adj_node_num")->value());
        otherNodeNumber = 0;

        for (xml_node<> *adj = network_node->first_node("node"); adj; adj = adj->next_sibling())
        {
            ID_str = adj->first_node("id")->value();
            ID = atoi(ID_str);
            ip = adj->first_node("ip")->value();
            port = atoi(adj->first_node("port")->value());
            pub = get_pub(ID_str);
            tmp_node = new simpleNode(ID, ip, port, pub);
            tmp_node->createClientSocket();
            // tmp_node->estConnection();
            otherNodes.push_back(tmp_node);
            otherNodeNumber++;
        }
        trustedNodeNumber = otherNodeNumber;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
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

simpleNode *network::getNode(int ID)
{
    for (auto &i : otherNodes)
    {
        if (i->getID() == ID)
        {
            return i;
        }
    }
    return NULL;
}

int network::getID()
{
    return self->getID();
}

void network::setID(int ID)
{
    self->setID(ID);
}
int network::getNodeNumber()
{
    return otherNodeNumber;
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
    for (auto const &i : otherNodes)
    {
        if (i->isTrusted())
            tmpTrustNum++;
    }
    pthread_mutex_lock(&lockTrustedNodeNumber);
    trustedNodeNumber = tmpTrustNum;
    pthread_mutex_unlock(&lockTrustedNodeNumber);
}

bool network::imTrusted()
{
    return self->isTrusted();
}
bool network::isTrsuted(int ID)
{
    for (auto const &i : otherNodes)
    {
        if (i->getID() == ID)
            return i->isTrusted();
    }
    return false;
}
int network::trustLvl(int ID)
{
    for (auto const &i : otherNodes)
    {
        if (i->getID() == ID)
            return i->isTrusted();
    }
    return false;
}
bool network::isNetworkComprometed()
{
    bool tmpNetworkComprometed = false;
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
int network::getMaxFD()
{
    return maxFD;
}

fd_set network::getSetOfSockets()
{
    return readfds;
}

void network::printNetwork()
{
    cout << "Self Node ID: " << self->getID() << " #Hash:" << self->getLastHash() << " #Is network comprometed: " << networkComprometed << endl;
    for (auto const &i : otherNodes)
    {
        std::cout << "Node ID: " << i->getID() << " #Trusted: " << i->isTrusted() << " #Hash: " << i->getLastHash() << endl;
    }
}

void network::connectToAllNodes()
{
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->isTrusted())
            {
                if (i->estConnection() == -1)
                {
                    // cout << "Error connection: " << i->getID() << endl;
                    //Decrease confidence if node is not avalible
                    i->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);
                }
                else
                {
                    //Add sockets for select
                    FD_SET(i->getSock(), &readfds);
                    if (i->getSock() > maxFD)
                        maxFD = i->getSock();
                    // cout << "Success connection: " << i->getID() << endl;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

bool network::connectToNode(int ID)
{
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->getID() == ID)
            {
                if (i->isTrusted())
                {
                    if (i->estConnection() == -1)
                    {
                        // cout << "Error connection: " << i->getID() << endl;
                        //Decrease confidence if node is not avalible
                        i->decreaseTrustLvlIn(DEFAULT_DECREASE_CNT);
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
        std::cerr << e.what() << '\n';
        return false;
    }
}

//If we are at this point, node is trusted
void network::reassembleSocket(int ID)
{
    try
    {
        for (auto const &i : otherNodes)
            if (i->getID() == ID)
                if (i->isTrusted())
                    i->createClientSocket();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void network::reassembleAllSockets()
{
    try
    {
        FD_ZERO(&readfds); //clear the socket set
        maxFD = -1;        //initialize maxFD
        for (auto const &i : otherNodes)
        {
            if (i->isTrusted())
            {
                i->createClientSocket();
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

bool network::validateMsg(string selfID, string clientID, int syncNumReceived, string MsgToVerify, string MsgSignature)
{
    //Verify if msg is for me
    int syncNumStored;
    simpleNode *sN = getNode(atoi(clientID.c_str()));

    if (atoi(selfID.c_str()) == self->getID())
    {
        //Verify if sync number is correct
        syncNumStored = sN->getSyncNum();
        if (syncNumReceived == syncNumStored)
        {
            //Verify if msg is correctly signed
            if (verify(MsgToVerify, hex2stream(MsgSignature), clientID))
            {
                //Increment sync number
                sN->incrementSyncNum();
                //Verification successful
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
        for (auto const &i : otherNodes)
        {
            if (i->getID() == destID && i->isTrusted())
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
                    cout << "Error sending: " << i->getID() << endl;
                else
                {
                    // cout << "Success sending: " << destID << endl;
                    i->incrementSyncNum();
                    return true;
                }
            }
        }
        return false;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
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
        for (auto const &i : otherNodes)
        {
            if (i->isTrusted())
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
                    cout << "Error sending: " << i->getID() << endl;
                else
                {
                    // cout << "Success sending: " << i->getID() << endl;
                    i->incrementSyncNum();
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
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

    if (resNum == 0)
        return 1;

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
            int tmpMaxFD = -1;  //initialize tmpMaxFD

            //Count all received connections
            for (auto const &i : otherNodes)
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
    //If no node is trusted return -1
    if (trustedNodeNumber < 2)
        return random;

    //If there are trusted nodes, pick one among them
    while (1)
    {
        //Self node must be counted as well +1; first node starts at 1 -> +1
        random = get_randomNumber(otherNodeNumber + 1 + 1);
        for (auto const &i : otherNodes)
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
    for (auto const &i : otherNodes)
    {
        i->resetTrustLvl();
    }
}

string network::recvString(int ID)
{
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->getID() == ID)
            {
                return i->recvString();
            }
        }
        return "ERR";
    }
    catch (const std::exception &e)
    {
        // std::cerr << e.what() << '\n';
        return "ERR";
    }
}
