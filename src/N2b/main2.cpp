// g++ -std=c++1y crypto.cpp -I /home/shravan40/cryptopp/build -lcryptopp

#include <iostream>
#include <stdlib.h>
#include <cryptopp/integer.h>

#include "crypto.hpp"

int main(void)
{
    std::string key_ID = "1";
    std::string msg = "SECRET";

    // generate_keys(key_ID);

    //ENCRYPT
    // std::string encripted_msg = encrypt(msg, key_ID);
    // std::cout << "c: " << enc_msg << std::endl;

    //DECRYPT
    // std::string recovered = decrypt(encripted_msg, key_ID);
    // std::cout << "recovered: " << recovered << std::endl;

    //SIGN
    std::string signed_msg = sign(msg, key_ID);

    //VERIFY SIGNATURE
    bool verification = verify(msg, signed_msg, key_ID);
    std::cout << "Verified: " << verification << std::endl;

    return 0;
}