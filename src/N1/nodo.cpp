#include "nodo.hpp"
using namespace std;

// nodo *n;

pthread_mutex_t lockFlag = PTHREAD_MUTEX_INITIALIZER;

nodo::nodo(){

};
nodo::nodo(int ID, const char *ip, int port, CryptoPP::RSA::PublicKey pub, CryptoPP::RSA::PublicKey prv)
{
    nodo::ID = ID;
    nodo::sock = 0;
    nodo::IP = ip;
    nodo::port = port;
    nodo::trusted = true;
    nodo::changeFlag = false;
    nodo::pub = pub;
    nodo::prv = prv;
    // nodo::hash_record.push_back(hash_record);
}

int nodo::getID()
{
    return nodo::ID;
}

void nodo::insertInReceiveMsgs(string s)
{
    nodo::receivedMsgs.push_back(s);
}

void nodo::setID(int ID)
{
    nodo::ID = ID;
}
bool nodo::getChangeFlag(){
    return nodo::changeFlag;
}
void nodo::setChangeFlag(bool flagValue){
    pthread_mutex_lock(&lockFlag);
    nodo::changeFlag= flagValue;
    pthread_mutex_unlock(&lockFlag);
}
sockaddr_in nodo::getAddr()
{
    return nodo::addr;
}
bool nodo::isTrusted()
{
    return nodo::trusted;
}

bool nodo::isMsgRepeated(string s)
{
    for (auto const &i : receivedMsgs)
        if (i == s)
            return true;
    return false;
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

void *timerThread(void *arg){

    struct arg_struct *args = (struct arg_struct *)arg;
    int clientSocket = args->s;
    nodo *n = args->n;

    cout << n->getID() << endl;

    //Default sleep time 5 sec
    sleep(5);
    n->setChangeFlag(false);
    cout << "timer" << n->getChangeFlag() << endl;
    pthread_exit(NULL);

}

void *socketThread(void *arg)
{
    char buffer[1024];
    char sendBuffer[1024];

    struct arg_struct *args = (struct arg_struct *)arg;

    int clientSocket = args->s;
    nodo *n = args->n;
    vector<string> vectString;
    int msgCode;

       cout << n->getID() << endl;

    bool verifyMsg = false;
    string clientID, randMsg, randHexSignedMsg;

    while (1)
    {
        bzero(buffer, sizeof(buffer));
        bzero(sendBuffer, sizeof(sendBuffer));

        recv(clientSocket, buffer, 1024, 0);

        vectString = splitBuffer(buffer); //from utils

        msgCode = atoi(vectString.at(0).c_str()); //MSG CODE
        clientID = vectString.at(1);              //Source ID
        randMsg = vectString.at(2);   
        randHexSignedMsg = vectString.at(3);

        // r = buffer;
        switch (msgCode)
        {
        //Close connection
        case 0:
            // printf("Disconnected %s:%d\n", inet_ntoa(n->getAddr().sin_addr), ntohs(n->getAddr().sin_port));
            cout << "Disconnected" << endl;
            close(clientSocket);
            pthread_exit(NULL);
            break;
        //Print received message
        case 1:
            cout << "From: " << clientID << " - " << randMsg << endl;
            // cout << "BUFF: " << buffer << endl;
            // send(clientSocket, buffer, strlen(buffer), 0);
            break;
        
        //Modify current hash request
        case 2:
            if (!n->isMsgRepeated(randMsg))
            {
                if (verify(randMsg, hex2stream(randHexSignedMsg), clientID))
                {
                    n->insertInReceiveMsgs(randMsg);
                    //Activar flag usando cerraduras
                    n->setChangeFlag(true);
                    //cout << n->getChangeFlag() <<endl;
                    
                    //Lanzar thread contador para anular cerradura
                    pthread_t tid;
                    pthread_create(&tid, NULL, timerThread, arg);
                    //pthread_join(tid, NULL);

                    //Return message
                    //Frimar mensaje de vuelta?

                    //send(clientSocket, sendBuffer, strlen(sendBuffer), 0);

                    //recibir nuevo hash
                }
            }

            cout << "Disconnected" << endl;
            close(clientSocket);
            sleep(7);
            cout << n->getChangeFlag() << endl;
            pthread_exit(NULL);

            break;
        default:
            break;
        }
    }
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
        //Se debería analizar la identidad de conexiones para evitar DDoS
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