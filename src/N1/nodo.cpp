#include "nodo.hpp"
using namespace std;

// nodo *n;

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
sockaddr_in nodo::getAddr()
{
    return nodo::addr;
}
bool nodo::imTrusted()
{
    return nodo::trusted;
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

struct arg_struct
{
    nodo *n;
    int s;
};

void *socketThread(void *arg)
{
    char buffer[1024];
    struct arg_struct *args = (struct arg_struct *)arg;

    int clientSocket = args->s;
    nodo *n = args->n;

    while (1)
    {
        bzero(buffer, sizeof(buffer));
        recv(clientSocket, buffer, 1024, 0);
        // r = buffer;
        if (strcmp(buffer, "0\n") == 0)
        {
            printf("Disconnected %s:%d\n", inet_ntoa(n->getAddr().sin_addr), ntohs(n->getAddr().sin_port));
            break;
        }
        else
        //All the control
        {
            cout << "BUFF: " << buffer << endl;
            send(clientSocket, buffer, strlen(buffer), 0);
        }
    }
    close(clientSocket);
    pthread_exit(NULL);
}

int nodo::serverUP(int max_c)
{
    int newSocket;
    int addrlen = sizeof(addr);
    int i = 0;
    arg_struct args;

    pthread_t tid[max_c];

    //LISTEN
    if (listen(sock, max_c) < 0)
        return -1;

    cout << "Server UP" << endl;

    while (1)
    {
        newSocket = accept(sock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
        if (newSocket < 0)
            return -1;

        printf("Connection accepted from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        args.n = this;
        args.s = newSocket;

        if (pthread_create(&tid[i++], NULL, socketThread, (void *)&args) != 0)
        {
            printf("Error");
        }

        if (i >= max_c)
        {
            i = 0;
            while (i < max_c)
            {
                pthread_join(tid[i++], NULL);
            }
            i = 0;
        }
        // waitpid(-1, NULL, WNOHANG); //KILL ZOMBIE PROCESSES
    }

    close(newSocket);
    return 0;
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

int nodo::recvString(const char *servResponse)
{
    char buffer[1024];

    if (recv(sock, buffer, 1024, 0) < 0)
    {
        printf("[-]Error in receiving data.\n");
        return -1;
    }
    else
    {
        if (strcmp(buffer, ":exit") == 0)
        {
            close(sock);
            cout << "disconnected from server" << endl;
        }
        else
        {
            cout << buffer << endl;
            bzero(buffer, 1024);
        }
        return 0;
    }
}