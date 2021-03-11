#include "infra.hpp"

using namespace rapidxml;
using namespace std;

infra *infra::instance = 0;

infra::infra()
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
    try
    {
        self = new nodo(ID, ip, port, pub, prv);

        //CREATE SERVER
        self->createServerSocket();
        //ADJACENT NODES
        nodo *tmp_node;
        // adj_node_num = atoi(network_node->first_node("adj_node_num")->value());
        otherNodeNumber = 0;

        for (xml_node<> *adj = network_node->first_node("node"); adj; adj = adj->next_sibling())
        {
            ID_str = adj->first_node("id")->value();
            ID = atoi(ID_str);
            ip = adj->first_node("ip")->value();
            port = atoi(adj->first_node("port")->value());
            pub = get_pub(ID_str);
            // CryptoPP::RSA::PrivateKey prv = get_prv(ID_str); Por ahora basura
            tmp_node = new nodo(ID, ip, port, pub, prv);
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

infra *infra::getInstance()
{
    if (!instance)
        instance = new infra();
    return instance;
}

int infra::getID()
{
    return self->getID();
}

void infra::setID(int ID)
{
    self->setID(ID);
}

int infra::getTrustedNodeNumber()
{
    return trustedNodeNumber;
}

bool infra::imTrusted()
{
    return self->isTrusted();
}

void infra::printOtherNodes()
{
    for (auto const &i : otherNodes)
    {
        std::cout << "Adj node ID: " << i->getID() << endl;
    }
}

void infra::initializeServer()
{
    try
    {
        self->serverUP(otherNodeNumber);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void infra::connectToAllNodes()
{
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->isTrusted())
            {
                if (i->estConnection() == -1)
                    cout << "Error connection: " << i->getID() << endl;
                else
                    cout << "Success connection: " << i->getID() << endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void infra::connectToNode(int ID)
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
void infra::reassembleSocket(int ID)
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

void infra::reassembleAllSockets()
{
    try
    {
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
void infra::sendString(int ID, const char *msg)
{
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->getID() == ID)
            {
                if (i->sendString(msg) == -1)
                    cout << "Error sending: " << i->getID() << endl;
                else
                    cout << "Success sending: " << ID << endl;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void infra::sendStringToAll(const char *msg)
{
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->isTrusted())
            {

                if (i->sendString(msg) == -1)
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

void infra::recvString(int ID, const char *servResponse)
{
    try
    {
        for (auto const &i : otherNodes)
        {
            if (i->getID() == ID)
            {
                if (i->recvString(servResponse) == -1)
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
