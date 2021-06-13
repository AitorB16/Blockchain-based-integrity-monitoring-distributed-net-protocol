#include "generate_keys.hpp"
// using namespace CryptoPP;

void generate_keys(std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(prng, 2048);

    CryptoPP::RSA::PrivateKey prv(params);
    CryptoPP::RSA::PublicKey pub(params);

    std::string name = "../RSA_keys/RSA_prv" + key_ID + ".der";
    CryptoPP::FileSink output1(name.c_str()); //Convert to char*
    prv.DEREncode(output1);

    name = "../RSA_keys/RSA_pub" + key_ID + ".der";
    CryptoPP::FileSink output2(name.c_str());
    pub.DEREncode(output2);
}


int main(int argc, char *argv[])
{
    generate_keys(argv[1]);
}

