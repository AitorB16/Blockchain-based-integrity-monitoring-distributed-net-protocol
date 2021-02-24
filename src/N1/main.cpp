#include "main.hpp"
using namespace std;

int main(void)
{

    infra *s = s->getInstance();

    cout << s->getID() << endl;

    // s->setID(2);

    s->printAdjNodes();

    pid_t pid = fork();

    if (pid == 0)
    {
        // SERVER INDEPENDENT PID FORK
        // cout << "Server UP" << endl;
        s->initializeServer();
    }
    else
    {
        const char *response;
        int option;

        // sleep(20);
        // s->connectToAdjacent(2); //if success...
        //CLIENT

        while (1)
        {

            // waitpid(-1, NULL, WNOHANG); //KILL ZOMBIE PROCESSES

            cout << "Enter options" << endl;
            cin >> option;
            switch (option)
            {
            case 0:
                cout << "killing server" << endl;
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0); //KILL SERVER (if no thread running OK) all threads should be individualy killed.
                break;
            case 1:
                cout << "Exiting infra" << endl;
                kill(pid, SIGKILL);
                waitpid(pid, NULL, 0);
                exit(0);
                break;
            case 2:
                cout << s->imTrusted() << endl;
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