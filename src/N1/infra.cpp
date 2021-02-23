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
        self = new nodo(ID, ip, port, pub, prv, "855f94917cc4ebb1faa9d523eea00cf9f6a8280a");

        //CREATE SERVER
        self->createServerSocket();
        //ADJACENT NODES
        nodo *tmp_node;
        // adj_node_num = atoi(network_node->first_node("adj_node_num")->value());
        adj_node_num = 0;

        for (xml_node<> *adj = network_node->first_node("node"); adj; adj = adj->next_sibling())
        {
            ID_str = adj->first_node("id")->value();
            ID = atoi(ID_str);
            ip = adj->first_node("ip")->value();
            port = atoi(adj->first_node("port")->value());
            pub = get_pub(ID_str);
            // CryptoPP::RSA::PrivateKey prv = get_prv(ID_str); Por ahora basura
            tmp_node = new nodo(ID, ip, port, pub, prv, "855f94917cc4ebb1faa9d523eea00cf9f6a8280a");
            tmp_node->createClientSocket();
            // tmp_node->estConnection();
            adj_nodes.push_back(tmp_node);
            adj_node_num++;
        }
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

void infra::printAdjNodes()
{
    for (auto const &i : adj_nodes)
    {
        std::cout << "Adj node ID: " << i->getID() << endl;
    }
}

void infra::initializeServer()
{
    try
    {
        self->serverUP(adj_node_num);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void infra::connectToAdjacents()
{
    try
    {
        for (auto const &i : adj_nodes)
        {
            if (i->estConnection() == -1)
                cout << "Error connection: " << i->getID() << endl;
            else
                cout << "Success connection: " << i->getID() << endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

void infra::connectToAdjacent(int ID)
{
    try
    {
        for (auto const &i : adj_nodes)
        {
            if (i->getID() == ID)
            {
                if (i->estConnection() == -1)
                    cout << "Error connection: " << i->getID() << endl;
                else
                    cout << "Success connection: " << ID << endl;
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
        for (auto const &i : adj_nodes)
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
