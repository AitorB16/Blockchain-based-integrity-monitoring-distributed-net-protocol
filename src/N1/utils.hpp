//utils
#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include <vector>
#include <cryptopp/rsa.h>
#include <string>
#include <iomanip>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <cstdio>
#include <fstream>

using namespace std;

vector<string> splitBuffer(const char *buffer);
string stream2hex(const std::string str);
string hex2stream(const std::string hexstr);
string gen_random(const int len);
char * gen_urandom(int len);
int get_randomNumber(int maxNum);
void splitVectString(vector<string> vectString, int &msgCode, int &clientID, int &selfID, int &syncNumReceived, string &content, string &MsgToVerify, string &MsgSignature);
void splitVectStringBlame(vector<string> vectString, int &msgCode, int &clientID, int &selfID, int &syncNumReceived, int &susMsgCode, int &suspectID, int &auditorID, int &susSyncNumReceived, string &susContent, string &susMsgSignature, string &susMsgToVerify, string &MsgSignature, string &MsgToVerify);
string getCurrentDateTime(string s);
void Logger(string logMsg);

#endif