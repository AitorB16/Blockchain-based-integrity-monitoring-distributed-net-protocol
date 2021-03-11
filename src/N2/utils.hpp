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

using namespace std;

vector<string> splitBuffer(const char* buffer);
std::string stream2hex(const std::string str);
std::string hex2stream(const std::string hexstr);
std::string gen_random(const int len);

#endif