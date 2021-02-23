#include <cryptopp/rsa.h>
#include <cryptopp/integer.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>
#include <cryptopp/files.h>

std::string encrypt(std::string msg)
{
    // CryptoPP::AutoSeededRandomPool prng;
    // CryptoPP::RSA::PrivateKey privKey1;

    // privKey1.GenerateRandomWithKeySize(prng, 2048);
    // CryptoPP::RSA::PublicKey pubKey1(privKey1);
    std::string c;

    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::InvertibleRSAFunction params;
    params.GenerateRandomWithKeySize(prng, 2048);

    CryptoPP::RSA::PrivateKey prv(params);
    CryptoPP::RSA::PublicKey pub(params);

    CryptoPP::FileSink output1("RSA_prv.dat");
    prv.DEREncode(output1);
    CryptoPP::FileSink output2("RSA_pub.dat");
    pub.DEREncode(output2);

    // CryptoPP::Integer n("0xbeaadb3d839f3b5f"), e("0x11"), d("0x21a5ae37b9959db9");

    // CryptoPP::RSA::PublicKey pubKey;
    // pubKey.Initialize(n, e);

    // ///////////////////////////////////////////////////////////////

    // CryptoPP::Integer m, c;
    // std::string message = msg;

    // // std::cout << "message: " << message << std::endl;

    // // Treat the message as a big endian byte array
    // m = CryptoPP::Integer((const byte *)message.data(), message.size());
    // // std::cout << "m: " << std::hex << m << std::endl;

    // // Encrypt
    // c = pubKey.ApplyFunction(m);

    // std::cout << "c: " << std::hex << c << std::endl;

    // CryptoPP::RSA::PublicKey pubKey;
    // pubKey.Initialize(params.GetModulus(), params.GetPublicExponent());

    CryptoPP::RSAES_OAEP_SHA_Encryptor e(pub);
    CryptoPP::StringSource ss1(msg, true,
                               new CryptoPP::PK_EncryptorFilter(prng, e,
                                                                new CryptoPP::StringSink(c)) // PK_EncryptorFilter

    );

    // CryptoPP::Integer m, c;
    // m = CryptoPP::Integer((const byte *)msg.data(), msg.size());

    // c = pub.ApplyFunction(m);

    return c;
}

std::string decrypt(std::string encr_msg)
{
    std::string r;

    CryptoPP::AutoSeededRandomPool prng;

    CryptoPP::RSA::PrivateKey prv;
    // CryptoPP::RSA::PublicKey pub;

    CryptoPP::FileSource input1("RSA_prv.dat", true);
    prv.BERDecode(input1);

    // CryptoPP::FileSink input2("RSA_pub.dat", true);
    // pub.BERDecode(input2);

    // Decryption
    CryptoPP::RSAES_OAEP_SHA_Decryptor d(prv);

    CryptoPP::StringSource ss2(encr_msg, true,
                     new CryptoPP::PK_DecryptorFilter(prng, d,
                                            new CryptoPP::StringSink(r)) // PK_DecryptorFilter
    );                                                                 // StringSource                                                                       // StringSource

    // CryptoPP::Integer r;
    // std::string recovered;

    // // Decrypt
    // r = prv.ApplyFunction(encr_msg);
    // // cout << "r: " << hex << r << endl;

    // // Round trip the message
    // size_t req = r.MinEncodedSize();
    // recovered.resize(req);
    // r.Encode((byte *)recovered.data(), recovered.size());

    // CryptoPP::Integer n("0xbeaadb3d839f3b5f"), e("0x11"), d("0x21a5ae37b9959db9");
    // CryptoPP::AutoSeededRandomPool prng;

    // CryptoPP::RSA::PrivateKey privKey;
    // privKey.Initialize(n, e, d);

    // ///////////////////////////////////////////////////////////////

    // CryptoPP::Integer c("0x3f47c32e8e17e291"), r;
    // std::string recovered;

    // // Decrypt
    // r = privKey.CalculateInverse(prng, c);
    // // cout << "r: " << hex << r << endl;

    // // Round trip the message
    // size_t req = r.MinEncodedSize();
    // recovered.resize(req);
    // r.Encode((byte *)recovered.data(), recovered.size());

    return r;

    // cout << "recovered: " << recovered << endl;
}
