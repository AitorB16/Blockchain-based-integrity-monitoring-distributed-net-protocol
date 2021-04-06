#include "auditor.hpp"

//Reset confidence

bool waitResponseFromAudited(int sock)
{
    struct timeval tv;
    tv.tv_sec = AUDITOR_SELECT_WAIT;
    tv.tv_usec = 0;

    int selectStatus;
    int counter = 0;
    fd_set FdSet;

    FD_ZERO(&FdSet);  //clear the socket set
    int maxFD = sock; //initialize tmpMaxFD
    FD_SET(sock, &FdSet);

    //just monitorize trusted sockets; low-eq maxFD descriptor
    selectStatus = select(maxFD + 1, &FdSet, NULL, NULL, &tv);
    //
    counter += selectStatus;

    //if timeout or received message number is gr eq to resNum -> break the loop
    if (selectStatus == 0)
        return true;

    return false;
}

auditor::auditor(){

};
auditor::auditor(network *selfNetwork)
{
    auditor::selfNetwork = selfNetwork;
};

int auditor::auditorUP()
{
    int sock, auditedID, msgCode, syncNumReceived;
    string audID, selfID, MsgToVerify, MsgSignature, content, msg;
    bool msgValid;
    vector<string> vs;
    simpleNode *auditedNode;

    while (1)
    {
        // sleep(DEF_TIMER_AUDIT);

        //If network not trusted, kill thread
        //return -1;

        auditedID = selfNetwork->getTrustedRandomNode();
        auditedNode = selfNetwork->getNode(auditedID);

        msgValid = false;

        if (selfNetwork->connectToNode(auditedID))
        {
            if (selfNetwork->sendString(2, auditedID, selfNetwork->getID()))
            {
                sock = auditedNode->getSock();
                //Node responds
                if (waitResponseFromAudited(sock))
                {
                    //We trust the connection?
                    msg = selfNetwork->recvString(auditedID);
                    vs = splitBuffer(msg.c_str());
                    splitVectString(vs, msgCode, audID, selfID, syncNumReceived, MsgToVerify, MsgSignature, content);
                    msgValid = selfNetwork->validateMsg(selfID, audID, syncNumReceived, MsgToVerify, MsgSignature);
                    if (!msgValid)
                    {

                        //Attempt of message falsification
                        cout << "Disconnected message was faked" << endl;
                        // close(clientSocket);
                        // pthread_exit(NULL);
                    }
                }
            }
        }
        selfNetwork->reassembleSocket(auditedID);

        //recv

        //Send to all except x.

        //Close connection
        // net->reassembleSocket(auditedID);

        sleep(1);
    }
    return 0;
}