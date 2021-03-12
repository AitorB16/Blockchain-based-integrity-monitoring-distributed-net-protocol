#include "crypto.hpp"
// using namespace CryptoPP;

void generate_keys(std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(prng, 2048);

    CryptoPP::RSA::PrivateKey prv(params);
    CryptoPP::RSA::PublicKey pub(params);

    std::string name = "./RSA_keys/RSA_prv" + key_ID + ".der";
    CryptoPP::FileSink output1(name.c_str()); //Convert to char*
    prv.DEREncode(output1);

    name = "./RSA_keys/RSA_pub" + key_ID + ".der";
    CryptoPP::FileSink output2(name.c_str());
    pub.DEREncode(output2);
}

CryptoPP::RSA::PublicKey get_pub(std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    CryptoPP::RSA::PublicKey pub;

    std::string name = "./RSA_keys/RSA_pub" + key_ID + ".der";

    CryptoPP::FileSource input(name.c_str(), true);
    pub.BERDecode(input);

    return pub;
}

CryptoPP::RSA::PrivateKey get_prv(std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    CryptoPP::RSA::PrivateKey prv;

    std::string name = "./RSA_keys/RSA_prv" + key_ID + ".der";

    CryptoPP::FileSource input(name.c_str(), true);
    prv.BERDecode(input);

    return prv;
}

std::string encrypt(std::string msg, std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    std::string c;

    //import public key
    CryptoPP::RSA::PublicKey pub = get_pub(key_ID);

    //ENCRYPTION
    CryptoPP::RSAES_OAEP_SHA_Encryptor e(pub);
    CryptoPP::StringSource ss1(msg, true,
                               new CryptoPP::PK_EncryptorFilter(prng, e,
                                                                new CryptoPP::StringSink(c)) // PK_EncryptorFilter

    );

    return c;
}

std::string decrypt(std::string encr_msg, std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    std::string r;

    //import priv
    CryptoPP::RSA::PrivateKey prv = get_prv(key_ID);

    // Decryption
    CryptoPP::RSAES_OAEP_SHA_Decryptor d(prv);

    CryptoPP::StringSource ss2(encr_msg, true,
                               new CryptoPP::PK_DecryptorFilter(prng, d,
                                                                new CryptoPP::StringSink(r)) // PK_DecryptorFilter
    );                                                                                       // StringSource                                                                       // StringSource

    return r;
}

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
                                                          new CryptoPP::StringSink(s)) // SignerFilter
    );                                                                                 // StringSource                                                                                     // StringSource                                                                       // StringSource

    return s;
}

bool verify(std::string msg, std::string sign_msg, std::string key_ID)
{
    CryptoPP::AutoSeededRandomPool prng;

    // std::string c;

    //import public key
    CryptoPP::RSA::PublicKey pub = get_pub(key_ID);

    CryptoPP::RSASSA_PKCS1v15_SHA_Verifier verifier(pub);

    // CryptoPP::StringSource ss2(msg + sign_msg, true,
    //                  new CryptoPP::SignatureVerificationFilter(
    //                      verifier, NULL,
    //                     CryptoPP::SignatureVerificationFilter::THROW_EXCEPTION) // SignatureVerificationFilter
    // );                                                                 // StringSource

    bool result = false;
    //   Verifier verifier(publicKey);
    CryptoPP::StringSource ss2(sign_msg + msg, true,
                               new CryptoPP::SignatureVerificationFilter(verifier,
                                                                         new CryptoPP::ArraySink((byte *)&result,
                                                                                                 sizeof(result))));

    return result;
}
