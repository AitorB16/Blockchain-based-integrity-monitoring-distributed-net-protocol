#include "nodo.hpp"
using namespace std;
nodo::nodo(){

};
nodo::nodo(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub, CryptoPP::RSA::PublicKey prv, std::string hash_record)
{
    nodo::ID = ID;
    nodo::sock = 0;
    nodo::IP = ip;
    nodo::port = port;
    nodo::trusted = true;
    nodo::pub = pub;
    nodo::prv = prv;
    nodo::hash_record.push_back(hash_record);
}

int nodo::getID()
{
    return nodo::ID;
}

void nodo::setID(int ID)
{
    nodo::ID = ID;
}

void nodo::createClientSocket()
{
    if ((nodo::sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw std::invalid_argument("Socket creation error");
    }
    memset(&addr, '0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, nodo::IP, &addr.sin_addr) <= 0)
    {
        throw std::invalid_argument("Invalid address/ Address not supported");
    }
}

void nodo::createServerSocket()
{
    int opt = 1;
    // Creating socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        throw std::invalid_argument("Socket creation error");
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        throw std::invalid_argument("Error socketopt");
    }
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, nodo::IP, &addr.sin_addr) <= 0)
    {
        throw std::invalid_argument("Invalid address/ Address not supported");
    }

    // Forcefully attaching socket to the port 8080
    if (bind(sock, (struct sockaddr *)&addr,
             sizeof(addr)) < 0)
    {
        throw std::invalid_argument("Bind failed");
    }
}
void nodo::serverUP(int max_c)
{
    int new_socket, valread;
    int addrlen = sizeof(addr);
    char buffer[1024] = {0};

    while (1)
    {
        //HAY QUE FORKEAR + VACIAR BUFFER
        if (listen(sock, max_c) < 0)
            throw std::invalid_argument("Error listening");

        if ((new_socket = accept(sock, (struct sockaddr *)&addr,
                                 (socklen_t *)&addrlen)) < 0)
        {
            cout << "ERROR" << endl;
        }
        valread = recv(new_socket, &buffer, 1024, 0);
        cout << buffer << endl;
        // sleep(5);
        // cout << "OUT" << endl;
        // buffer[1024] = {0};
    }
}

int nodo::estConnection()
{
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        return -1;
    }
    else
        return 0;
}

int nodo::sendString(const char *codigo)
{
    return send(sock, codigo, strlen(codigo), 0);
}