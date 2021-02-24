#include "main.hpp"
using namespace std;

int main(void)
{

    infra *s = s->getInstance();

    cout << s->getID() << endl;

    // s->setID(2);

    s->printAdjNodes();

    int pid = fork();
    if (pid == 0)
    {
        // SERVER INDEPENDENT PID FORK

        s->initializeServer();
        cout << "Server UP" << endl;
    }
    else
    {
        const char *response;
        s->connectToAdjacent(1); //if success...
        //CLIENT
        while (1)
        {

            s->sendString(1, ":exit");
            s->recvString(1, response); //Hay que solucionar response
            break;
        }
    }

    // s->connectToAdjacents();

    return 0;
}
