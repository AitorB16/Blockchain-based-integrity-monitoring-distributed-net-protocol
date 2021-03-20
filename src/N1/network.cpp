#include "network.hpp"

using namespace rapidxml;
using namespace std;

pthread_mutex_t lockMsgList = PTHREAD_MUTEX_INITIALIZER;

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

        self = new selfNode(ID, ip, port, pub, prv);

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
            if (i->isTrusted())
            {
                return i;
            }
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
    return trustedNodeNumber;
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
bool network::isNetworkComprometed()
{
    return networkComprometed;
}
void network::setNetworkToComprometed()
{
    networkComprometed = true;
}
int network::getMaxFD()
{
    return maxFD;
}

fd_set network::getSetOfSockets()
{
    return readfds;
}

// void network::insertInReceivedMsgs(string s)
// {
//     pthread_mutex_lock(&lockMsgList);
//     receivedMsgs.push_back(s);
//     pthread_mutex_unlock(&lockMsgList);
// }

// bool network::isMsgRepeated(string s)
// {
//     bool isRepeated;
//     pthread_mutex_lock(&lockMsgList);
//     for (auto const &i : receivedMsgs)
//     {
//         if (i == s)
//         {
//             isRepeated = true;
//             break;
//         }
//         else
//             isRepeated = false;
//     }
//     pthread_mutex_unlock(&lockMsgList);
//     return isRepeated;
// }

void network::printNetwork()
{
    cout << "Self Node ID: " << self->getID() << " #Hash:" << self->getCurrentHash() << " #Is network comprometed: " << networkComprometed << endl;
    for (auto const &i : otherNodes)
    {
        std::cout << "Node ID: " << i->getID() << " #Trusted: " << i->isTrusted() << " #Hash: " << i->getCurrentHash() << endl;
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
                    cout << "Error connection: " << i->getID() << endl;
                }
                else
                {
                    //Add sockets for select
                    FD_SET(i->getSock(), &readfds);
                    if (i->getSock() > maxFD)
                        maxFD = i->getSock();
                    cout << "Success connection: " << i->getID() << endl;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void network::connectToNode(int ID)
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
                        cout << "Error connection: " << i->getID() << endl;
                    else
                        cout << "Success connection: " << ID << endl;
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
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
void network::sendString(int code, int destID, int sourceID, string content)
{
    std::string buffer, msg, signedMsg, hexMsg;
    int syncNum;

    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->getID() == destID)
            {

                //Put msg code
                buffer = to_string(code) + ";";

                //Random msg
                // msg = std::string(random);

                //Get and increment Sync Number
                syncNum = i->getSyncNum();
                i->incrementSyncNum();

                //Se especifica origen + destino + syncNum
                msg = to_string(sourceID) + ";" + to_string(i->getID()) + ";" + to_string(syncNum);

                signedMsg = sign(msg, std::to_string(sourceID));
                hexMsg = stream2hex(signedMsg);
                buffer = buffer + msg + ";" + hexMsg + ";" + content + ";";
                if (i->sendString(buffer.c_str()) == -1)
                    cout << "Error sending: " << i->getID() << endl;
                else
                    cout << "Success sending: " << destID << endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
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
                //Put msg code
                buffer = to_string(code) + ";";

                //Random msg
                // msg = std::string(random);

                //Get and increment Sync Number
                syncNum = i->getSyncNum();
                i->incrementSyncNum();

                //Se especifica origen + destino + firma para que otro nodo no falsifique mensajes
                msg = to_string(sourceID) + ";" + to_string(i->getID()) + ";" + to_string(syncNum);

                //Se especifica firma
                signedMsg = sign(msg, to_string(sourceID));
                hexMsg = stream2hex(signedMsg);
                buffer = buffer + msg + ";" + hexMsg + ";" + content + ";";

                if (i->sendString(buffer.c_str()) == -1)
                    cout << "Error sending: " << i->getID() << endl;
                else
                    cout << "Success sending: " << i->getID() << endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
int network::waitResponses(int resNum)
{
    struct timeval tv;
    tv.tv_sec = DEFAULT_SELECT_WAIT;
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
        //
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

void network::recvString(int ID, const char *servResponse)
{
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->getID() == ID)
            {
                if (i->recvString() == -1)
                    cout << "Error reciving: " << i->getID() << endl;
                else
                    cout << "Success reciving: " << ID << endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}
