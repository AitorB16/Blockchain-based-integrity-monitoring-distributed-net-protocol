// g++ -std=c++1y crypto.cpp -I /home/shravan40/cryptopp/build -lcryptopp

#include <iostream>
#include <stdlib.h>
#include <cryptopp/integer.h>

#include "crypto.hpp"

int main(void)
{
    std::string key_ID = "1";
    generate_keys(key_ID);
    std::string enc_msg = encrypt("fasfasfsafsaf", key_ID);
    // std::cout << "c: " << enc_msg << std::endl;
    
    std::string recovered = decrypt(enc_msg, key_ID);
    std::cout << "recovered: " << recovered << std::endl;
    return 0;
}