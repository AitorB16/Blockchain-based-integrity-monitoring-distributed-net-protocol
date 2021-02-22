#include "nodo.hpp"
using namespace std;
nodo::nodo(){

};
nodo::nodo(int ID, const char* ip, int port, CryptoPP::RSA::PublicKey pub, CryptoPP::RSA::PublicKey prv, std::string hash_record)
{
    nodo::ID = ID;

    memset(&addr, '0', sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, ip, &addr.sin_addr) <= 0)
    {
        cout << "Invalid address/ Address not supported" << endl;
    }

    nodo::port = port;
    nodo::trusted = true;
    nodo::pub = pub;
    nodo::prv = prv;
    nodo::hash_record.push_back(hash_record);
}

int nodo::getID(){
    return nodo::ID;
}