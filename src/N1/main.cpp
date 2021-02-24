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

    //Print adj nodes
    s->printAdjNodes();

    if (pthread_create(&serverTid, NULL, serverThread, (void *)&args) != 0)
    {
        printf("Error");
    }

    {
        const char *response;
        int option;

        char *buffer;
        int dest;

        while (1)
        {

            // waitpid(-1, NULL, WNOHANG); //KILL ZOMBIE PROCESSES

            cout << "Enter options" << endl;
            cin >> option;
            switch (option)
            {
            case 0:
                cout << "Exiting infra" << endl;
                exit(0);
                break;
            case 1:
                cout << s->imTrusted() << endl;
                break;
            case 2:
                cout << "enter node ID" << endl;
                cin >> dest;
                s->connectToAdjacent(dest);
                break;
            case 3:
                cout << "enter node ID" << endl;
                cin >> dest;
                s->sendString(dest,"0\n");
                s->reassembleSocket(dest); //Reassamble the socket for future reconnections

                break;
            case 4:
                cout << "enter dest ID" << endl;
                cin >> dest;
                cout << "enter msg" << endl;
                cin >> buffer;
                s->sendString(dest, buffer);
                break;
            default:
                break;
            }

            // s->sendString(2, ":exit");
            // s->recvString(2, response); //Hay que solucionar response
            // waitpid(-1, NULL, WNOHANG); //KILL ZOMBIE PROCESSES
        }
    }

    // s->connectToAdjacents();

    return 0;
}