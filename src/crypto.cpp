#include "crypto.hpp"
// using namespace CryptoPP;

/* Apply SHA256 to input string + hex encode */
std::string hashText(std::string inputText)
{
    CryptoPP::SHA256 hash;
    std::string digest;

    CryptoPP::StringSource s(inputText, true, new CryptoPP::HashFilter(hash, new CryptoPP::HexEncoder(new CryptoPP::StringSink(digest))));
    return digest;
}

/* Load pub key of node ID from a binary file */
CryptoPP::RSA::PublicKey get_pub(std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    CryptoPP::RSA::PublicKey pub;

    std::string name = "../RSA_keys/RSA_pub" + key_ID + ".der";

    CryptoPP::FileSource input(name.c_str(), true);
    pub.BERDecode(input);

    return pub;
}

/* Load prv key of node ID from a binary file */
CryptoPP::RSA::PrivateKey get_prv(std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    CryptoPP::RSA::PrivateKey prv;

    std::string name = "../RSA_keys/RSA_prv" + key_ID + ".der";

    CryptoPP::FileSource input(name.c_str(), true);
    prv.BERDecode(input);

    return prv;
}

/* Sign and hex-encode msg with Node ID's prv key */
std::string sign(std::string msg, std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    std::string s;

    //import priv
    CryptoPP::RSA::PrivateKey prv = get_prv(key_ID);

    // Sign and Encode
    CryptoPP::RSASSA_PKCS1v15_SHA_Signer signer(prv);

    CryptoPP::StringSource ss1(msg, true,
                               new CryptoPP::SignerFilter(prng, signer,
                                                          new CryptoPP::HexEncoder(
                                                              new CryptoPP::StringSink(s))) // SignerFilter
    );                                                                                      // StringSource                                                                                     // StringSource                                                                       // StringSource

    return s;
}

/* Verify signed msg against original msg, with Node ID's pub key */
bool verify(std::string msg, std::string sign_msg, std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    //import public key
    CryptoPP::RSA::PublicKey pub = get_pub(key_ID);

    CryptoPP::RSASSA_PKCS1v15_SHA_Verifier verifier(pub);

    // decode signature
    std::string decodedSignature;
    CryptoPP::StringSource ss(sign_msg, true,
                              new CryptoPP::HexDecoder(
                                  new CryptoPP::StringSink(decodedSignature)));

    bool result = false;

    //Verify
    CryptoPP::StringSource ss2(decodedSignature + msg, true,
                               new CryptoPP::SignatureVerificationFilter(verifier,
                                                                         new CryptoPP::ArraySink((byte *)&result,
                                                                                                 sizeof(result))));

    return result;
}