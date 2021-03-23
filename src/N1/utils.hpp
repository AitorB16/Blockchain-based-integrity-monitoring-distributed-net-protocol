//utils
#ifndef UTILS_HPP
#define UTILS_HPP

#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

vector<string> splitBuffer(const char *buffer);
string stream2hex(const std::string str);
string hex2stream(const std::string hexstr);
string gen_random(const int len);
char * gen_urandom(int len);
int get_randomNumber(int maxNum);

#endif