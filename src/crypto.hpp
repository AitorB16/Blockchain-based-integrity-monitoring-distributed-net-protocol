//crypto.hpp
#ifndef CRYPTO_H
#define CRYPTO_H

#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>
#include <cryptopp/hex.h>

#include "globals.hpp"

std::string hashText(std::string inputText);
void generate_keys(std::string key_ID);
CryptoPP::RSA::PublicKey get_pub(std::string key_ID);
CryptoPP::RSA::PrivateKey get_prv(std::string key_ID);
std::string sign(std::string msg, std::string key_ID);
bool verify(std::string msg, std::string sign_msg, std::string key_ID);

#endif
