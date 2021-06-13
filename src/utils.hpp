//utils
#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <ctime>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include "globals.hpp"

using namespace std;

vector<string> splitBuffer(const char *buffer);
int getRandomNumber(int maxNum);
void splitVectString(vector<string> vectString, int &msgCode, int &clientID, int &selfID, int &syncNumReceived, string &content, string &MsgToVerify, string &MsgSignature);
void splitVectStringBlame(vector<string> vectString, int &msgCode, int &clientID, int &selfID, int &syncNumReceived, int &susMsgCode, int &suspectID, int &auditorID, int &susSyncNumReceived, string &susContent, string &susMsgSignature, string &susMsgToVerify, string &MsgSignature, string &MsgToVerify);
string getCurrentDateTime(string s);
void Logger(string logMsg);

#endif