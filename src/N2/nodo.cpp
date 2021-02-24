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
    char buffer[1024];
    int newSocket, valread;
    int addrlen = sizeof(addr);

    pid_t childpid;

    // int pid = -1;

    //HAY QUE FORKEAR + VACIAR BUFFER
    if (listen(sock, max_c) < 0)
        throw std::invalid_argument("Error listening");

    // while (1)
    // {
    //     if ((new_socket = accept(sock, (struct sockaddr *)&addr,
    //                              (socklen_t *)&addrlen)) < 0)
    //     {
    //         cout << "ERROR" << endl;
    //     }
    //     //Fork

    //     // pthread_t tid[60];
    //     int i = 0;
    //     while (1)
    //     {

    //         // if (pthread_create(&tid[i++], NULL, socketThread(), &new_socket) != 0)
    //         //     printf("Failed to create thread\n");
    //         valread = recv(new_socket, &buffer, 1024, 0);
    //         cout << buffer << endl;

    //         // CLEAN BUFFER
    //         bzero(buffer, 1024);
    //     }
    //     // valread = recv(new_socket, &buffer, 1024, 0);
    //     // cout << buffer << endl;
    //     // sleep(5);
    //     // cout << "OUT" << endl;
    //     // buffer[1024] = {0};
    // }
    while (1)
    {
        newSocket = accept(sock, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
        if (newSocket < 0)
        {
            exit(1);
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

        if ((childpid = fork()) == 0)
        {
            close(sock);

            while (1)
            {
                recv(newSocket, buffer, 1024, 0);
                if (strcmp(buffer, ":exit") == 0)
                {
                    printf("Disconnected from %s:%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                    break;
                }
                else
                {
                    printf("Client: %s\n", buffer);
                    send(newSocket, buffer, strlen(buffer), 0);
                    bzero(buffer, sizeof(buffer));
                }
            }
        }
    }

    close(newSocket);
}

// void *socketThread(void *arg)
// {
//     int newSocket = *((int *)arg);
//     recv(newSocket, client_message, 2000, 0);

//     // Send message to the client socket
//     // pthread_mutex_lock(&lock);
//     char *message = (char*) malloc(sizeof(client_message) + 20);
//     strcpy(message, "Hello Client : ");
//     strcat(message, client_message);
//     strcat(message, "\n");
//     strcpy(buffer, message);
//     free(message);
//     // pthread_mutex_unlock(&lock);
//     sleep(1);
//     send(newSocket, buffer, 13, 0);
//     printf("Exit socketThread \n");
//     close(newSocket);
//     pthread_exit(NULL);
// }

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
    // valread = recv(new_socket, &buffer, 1024, 0);
    // const char *tmp_buff[1024];
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
            // printf("Server: \t%s\n", buffer);
            // strcpy(servResponse, tmp_buff);
            cout << buffer << endl;
            bzero(buffer, 1024);
        }
        return 0;
    }
}