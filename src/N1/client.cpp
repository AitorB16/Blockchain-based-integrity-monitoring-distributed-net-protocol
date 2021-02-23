// client.cpp file
// g++ -std=c++1y crypto.cpp -I /home/shravan40/cryptopp/build -lcryptopp

#include <iostream>
#include <cryptopp/sha.h>
#include <cryptopp/filters.h> 

//#include <libcrypto++>
int main()
{

    
    // SHA
    CryptoPP::SHA1 sha1;
    std::string source = "Hello";
    std::string hash = "";
    CryptoPP::StringSource(source, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash))));
    std::cout << hash;

    return 0;
}