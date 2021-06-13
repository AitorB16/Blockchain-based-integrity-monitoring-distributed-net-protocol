#include "utils.hpp"

/* Split buffer into vector string, by ; separator */
vector<string> splitBuffer(const char *buffer)
{
    vector<string> strings;
    string delimeter = ";";
    size_t pos = 0;
    string s = buffer;
    while ((pos = s.find(delimeter)) != std::string::npos)
    {
        strings.push_back(s.substr(0, pos));
        s.erase(0, pos + delimeter.length());
    }
    return strings;
}

/* Get random number, seed /dev/urandom */
int getRandomNumber(int maxNum)
{
    int num;
    unsigned char buffer[1];
    int fd = open("/dev/urandom", O_RDONLY);
    read(fd, buffer, 1);
    close(fd);
    num = (int)buffer[0];
    num = num % maxNum;
    return num;
}

/* Procedure split vector string into param fields */
void splitVectString(vector<string> vectString, int &msgCode, int &clientID, int &selfID, int &syncNumReceived, string &content, string &MsgToVerify, string &MsgSignature)
{

    msgCode = atoi(vectString.at(0).c_str());         //MSG CODE
    clientID = atoi(vectString.at(1).c_str());        //Source ID
    selfID = atoi(vectString.at(2).c_str());          //ID of current server
    syncNumReceived = atoi(vectString.at(3).c_str()); //SyncNum
    content = vectString.at(4);                       //Payload of the msg
    MsgSignature = vectString.at(5);                  //Msg signed

    MsgToVerify = to_string(msgCode) + ";" + to_string(clientID) + ";" + to_string(selfID) + ";" + to_string(syncNumReceived) + ";" + content; //RandomMsg !!TENER EN CUENTA QUE EL ID DEL SERVER ESTA FIRMADO JUNTO CON LOS CARACTERES ALEATORIOS
}

/* Procedure split double encapsulated vector string into param fields */
void splitVectStringBlame(vector<string> vectString, int &msgCode, int &clientID, int &selfID, int &syncNumReceived, int &susMsgCode, int &suspectID, int &auditorID, int &susSyncNumReceived, string &susContent, string &susMsgSignature, string &susMsgToVerify, string &MsgSignature, string &MsgToVerify)
{
    /* Headers*/
    msgCode = atoi(vectString.at(0).c_str());         //MSG CODE
    clientID = atoi(vectString.at(1).c_str());        //Source ID
    selfID = atoi(vectString.at(2).c_str());          //ID of current server
    syncNumReceived = atoi(vectString.at(3).c_str()); //SyncNum

    /* Content */
    susMsgCode = atoi(vectString.at(4).c_str());         //Mscg Code from suspicious reply (2)
    suspectID = atoi(vectString.at(5).c_str());          //Suspicious ID
    auditorID = atoi(vectString.at(6).c_str());          //ID of auditor
    susSyncNumReceived = atoi(vectString.at(7).c_str()); //SyncNum from suspicious reply
    susContent = vectString.at(8);                       //Conflictive hash
    susMsgSignature = vectString.at(9);                  //Signed msg
    susMsgToVerify = to_string(susMsgCode) + ";" + to_string(suspectID) + ";" + to_string(auditorID) + ";" + to_string(susSyncNumReceived) + ";" + susContent;

    /* Signature */
    MsgSignature = vectString.at(10);
    MsgToVerify = to_string(msgCode) + ";" + to_string(clientID) + ";" + to_string(selfID) + ";" + to_string(syncNumReceived) + ";" + susMsgToVerify + ";" + susMsgSignature;
}

/* Return current date and time to logs */
string getCurrentDateTime(string s)
{
    time_t now = time(0);
    struct tm tstruct;
    char buf[80];
    tstruct = *localtime(&now);
    if (s == "now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if (s == "date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return string(buf);
};

/* Write a given string into current log file */
void Logger(string logMsg)
{
    string filePath = "../logs/" + getCurrentDateTime("date") + ".txt";
    if (ID != -1)
        filePath = "../logs/" + to_string(ID) + string("_") + getCurrentDateTime("date") + ".txt";
    string now = getCurrentDateTime("now");
    ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
    ofs << now << '\t' << logMsg << '\n';
    ofs.close();
}