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
    FD_ZERO(&readfds); //clear the socket set
    maxFD = -1;        //initialize maxFD

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
int network::getMaxFD()
{
    return maxFD;
}

fd_set network::getSetOfSockets()
{
    return readfds;
}

void network::insertInReceivedMsgs(string s)
{
    pthread_mutex_lock(&lockMsgList);
    receivedMsgs.push_back(s);
    pthread_mutex_unlock(&lockMsgList);
}

bool network::isMsgRepeated(string s)
{
    bool isRepeated;
    pthread_mutex_lock(&lockMsgList);
    for (auto const &i : receivedMsgs)
    {
        if (i == s)
        {
            isRepeated = true;
            break;
        }
        else
            isRepeated = false;
    }
    pthread_mutex_unlock(&lockMsgList);
    return isRepeated;
}

void network::printOtherNodes()
{
    for (auto const &i : otherNodes)
    {
        std::cout << "Adj node ID: " << i->getID() << endl;
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

    //At least 1 sec is needed to generate another random string (cahotic system)
    sleep(1);

    //Put msg code and source
    buffer = to_string(code) + ";" + to_string(sourceID) + ";";

    //Elaborate random datagram
    msg = gen_random(RANDOM_STR_LEN);

    int descr = -1;
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->getID() == destID)
            {
                //Se especifica destino + firma para que otro nodo no use mismo mensaje
                msg = to_string(i->getID()) + ";" + msg;
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

    //At least 1 sec is needed to generate another random string (cahotic system)
    sleep(1);

    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->isTrusted())
            {
                //Put msg code and source
                buffer = to_string(code) + ";" + to_string(sourceID) + ";";

                //Elaborate random datagram SOMETIMES IT TAKES DELAY
                msg = gen_random(RANDOM_STR_LEN);

                //Se especifica destino + firma para que otro nodo no use mismo mensaje
                msg = to_string(i->getID()) + ";" + msg;
                signedMsg = sign(msg, std::to_string(sourceID));
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
        cout << "AAA" << endl;
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
