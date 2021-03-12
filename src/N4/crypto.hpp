//crypto.hpp
#ifndef CRYPTO_H
#define CRYPTO_H

#include <cryptopp/rsa.h>
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>

void generate_keys(std::string key_ID);

CryptoPP::RSA::PublicKey get_pub(std::string key_ID);

CryptoPP::RSA::PrivateKey get_prv(std::string key_ID);

std::string encrypt(std::string msg, std::string key_ID);

std::string decrypt(std::string encr_msg, std::string key_ID);

std::string sign(std::string msg, std::string key_ID);

bool verify(std::string msg, std::string sign_msg, std::string key_ID);

#endif

// class crypto {
//     private:
//         CryptoPP::Integer pubk;
//         CryptoPP::Integer privk;
//     public:
//         // int function crypto();
//         CryptoPP::Integer function encrypt(std::string encr_msg);
//         CryptoPP::Integer function decrypt(std::string decrp_msg);

// };
