// g++ -std=c++1y crypto.cpp -I /home/shravan40/cryptopp/build -lcryptopp

#include <iostream>
#include <stdlib.h>
#include <cryptopp/integer.h>

// #include "crypto.hpp"
#include "infra.hpp"
using namespace std;

int main(void)
{

    infra *s = s->getInstance();

    cout << s->getID() << endl;

    // s->setID(2);

    cout << s->getID() << endl;

    s->printAdjNodes();

    return 0;
}