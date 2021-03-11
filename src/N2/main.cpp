#include "main.hpp"
using namespace std;

struct arg_struct
{
    infra *inf;
};

//Server thread start
void *serverThread(void *arg)
{
    struct arg_struct *args = (struct arg_struct *)arg;

    infra *inf = args->inf;
    inf->initializeServer();
    pthread_exit(NULL);
}

int main(void)
{

    pthread_t serverTid;

    infra *s = s->getInstance();

    arg_struct args;
    args.inf = s;

    cout << s->getID() << endl;

    //Print other nodes
    s->printOtherNodes();

    if (pthread_create(&serverTid, NULL, serverThread, (void *)&args) != 0)
    {
        printf("Error");
    }

    {
        // const char *response;
        int option;

        std::string buffer, response, msg, signedMsg, hexMsg;
        int dest;

        while (1)
        {

            // waitpid(-1, NULL, WNOHANG); //KILL ZOMBIE PROCESSES

            cout << "Enter options" << endl;
            cin >> option;
            switch (option)
            {
            //Close infraestructure
            case 0:
                cout << "Exiting infra" << endl;
                exit(0);
                break;
            //Is infra trusted
            case 1:
                cout << s->imTrusted() << endl;
                break;
            //Connect to ID node
            case 2:
                cout << "enter node ID" << endl;
                cin >> dest;
                s->connectToNode(dest);
                // s->connectToAdjacents();
                break;
            //Disconnect from ID + reassamble socket
            case 3:
                buffer = "0;" + to_string(s->getID()) + ";";
                cout << "enter node ID" << endl;
                cin >> dest;
                s->sendString(dest, buffer.c_str());
                s->reassembleSocket(dest); //Reassamble the socket for future reconnections
                break;
            //Send string to ID node
            case 4:
                cout << "enter dest ID" << endl;
                cin >> dest;
                cout << "enter msg" << endl;
                cin >> buffer;
                buffer = "1;" + to_string(s->getID()) + ";" + buffer + ";";
                s->sendString(dest, buffer.c_str());
                break;
            //BCAST request to send + timeout + count
            case 5:
                //Connect to ALL
                s->connectToAllNodes();

                //Elaborate random datagram
                msg = gen_random(randomStrLen);

                //Sign request
                signedMsg = sign(msg, std::to_string(s->getID()));
                hexMsg = stream2hex(signedMsg);
                buffer = "2;" + to_string(s->getID()) + ";" + msg + ";" + hexMsg + ";";

                //Send request to ALL
                s->sendStringToAll(buffer.c_str());

                //Wait 2/3 of network to send OK Select

                //Send hash

                //Close connection

                // buffer = "0;NULL;";
                // s->sendStringToAll(buffer.c_str());
                s->reassembleAllSockets();
                break;
            default:
                break;
            }

            // s->sendString(2, ":exit");
            // s->recvString(2, response); //Hay que solucionar response
            // waitpid(-1, NULL, WNOHANG); //KILL ZOMBIE PROCESSES
        }
    }

    return 0;
}