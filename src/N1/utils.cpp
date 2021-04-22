#include "utils.hpp"

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

//Convert string to hex -> ASCII chars to send over network
std::string stream2hex(const std::string str)
{
    std::string hexstr;
    bool capital = false;
    hexstr.resize(str.size() * 2);
    const size_t a = capital ? 'A' - 1 : 'a' - 1;

    for (size_t i = 0, c = str[0] & 0xFF; i < hexstr.size(); c = str[i / 2] & 0xFF)
    {
        hexstr[i++] = c > 0x9F ? (c / 16 - 9) | a : c / 16 | '0';
        hexstr[i++] = (c & 0xF) > 9 ? (c % 16 - 9) | a : c % 16 | '0';
    }
    return hexstr;
}

// Convert string of hex numbers to its equivalent char-stream
std::string hex2stream(const std::string hexstr)
{
    std::string str;
    str.resize((hexstr.size() + 1) / 2);

    for (size_t i = 0, j = 0; i < str.size(); i++, j++)
    {
        str[i] = (hexstr[j] & '@' ? hexstr[j] + 9 : hexstr[j]) << 4, j++;
        str[i] |= (hexstr[j] & '@' ? hexstr[j] + 9 : hexstr[j]) & 0xF;
    }
    return str;
}

string gen_random(const int len)
{

    string tmp_s;
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    // /DEV/URANDOM
    srand((unsigned)time(NULL) * getpid());

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];

    return tmp_s;
}
char *gen_urandom(int len)
{
    char *myRandomData;
    myRandomData = (char *)malloc(len);
    size_t randomDataLen = 0;
    int randomData = open("/dev/random", O_RDONLY);
    if (randomData < 0)
    {
        // something went wrong
    }
    else
    {

        while (randomDataLen < sizeof myRandomData)
        {
            ssize_t result = read(randomData, myRandomData + randomDataLen, (sizeof myRandomData) - randomDataLen);
            if (result < 0)
            {
                // something went wrong
            }
            randomDataLen += result;
        }
        close(randomData);
    }

    return myRandomData;
    //free(myRandomData);
}

int get_randomNumber(int maxNum)
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

void splitVectString(vector<string> vectString, int &msgCode, int &clientID, int &selfID, int &syncNumReceived, string &content, string &MsgToVerify, string &MsgSignature)
{

    msgCode = atoi(vectString.at(0).c_str());         //MSG CODE
    clientID = atoi(vectString.at(1).c_str());                      //Source ID
    selfID = atoi(vectString.at(2).c_str());                        //ID of current server
    syncNumReceived = atoi(vectString.at(3).c_str()); //SyncNum
    content = vectString.at(4);                       //Payload of the msg
    MsgSignature = vectString.at(5);                  //Msg signed

    MsgToVerify = to_string(msgCode) + ";" + to_string(clientID) + ";" + to_string(selfID) + ";" + to_string(syncNumReceived) + ";" + content; //RandomMsg !!TENER EN CUENTA QUE EL ID DEL SERVER ESTA FIRMADO JUNTO CON LOS CARACTERES ALEATORIOS
}

void splitVectStringBlame(vector<string> vectString, int &msgCode, int &clientID, int &selfID, int &syncNumReceived, int &susMsgCode, int &suspectID, int &auditorID, int &susSyncNumReceived, string &susContent, string &susMsgSignature, string &susMsgToVerify, string &MsgSignature, string &MsgToVerify)
{
    //Headers
    msgCode = atoi(vectString.at(0).c_str());         //MSG CODE
    clientID = atoi(vectString.at(1).c_str());                      //Source ID
    selfID = atoi(vectString.at(2).c_str());                        //ID of current server
    syncNumReceived = atoi(vectString.at(3).c_str()); //SyncNum

    //Content
    susMsgCode = atoi(vectString.at(4).c_str());         //Mscg Code from suspicious reply (2)
    suspectID = atoi(vectString.at(5).c_str());                        //Suspicious ID
    auditorID = atoi(vectString.at(6).c_str());                        //ID of auditor
    susSyncNumReceived = atoi(vectString.at(7).c_str()); //SyncNum from suspicious reply
    susContent = vectString.at(8);                       //Conflictive hash
    susMsgSignature = vectString.at(9);                  //Signed msg
    susMsgToVerify = to_string(susMsgCode) + ";" + to_string(suspectID) + ";" + to_string(auditorID) + ";" + to_string(susSyncNumReceived) + ";" + susContent;

    //Signature
    MsgSignature = vectString.at(10);
    MsgToVerify = to_string(msgCode) + ";" + to_string(clientID) + ";" + to_string(selfID) + ";" + to_string(syncNumReceived) + ";" + susMsgToVerify + ";" + susMsgSignature;
}