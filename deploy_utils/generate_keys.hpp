//generate_keys.hpp
#ifndef GENERATE_KEYS_H
#define GENERATE_KEYS_H



#include <cryptopp/rsa.h>
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>


void generate_keys(std::string key_ID);

#endif

