//
// Created by Seyyidahmed on 4/8/18.
//

#ifndef CHATRC4_RC4_H
#define CHATRC4_RC4_H

#include <iostream>
#define N 256



class RC4 {
private:
    std::string key;
    unsigned char s[N];
    unsigned int i, j;
    void swap(unsigned int, unsigned int);
    unsigned char GenerateNextKeyByte();
    unsigned char xorByte(unsigned char, unsigned char);


public:
    RC4();
    RC4(std::string);
    void resetRC4(std::string newKey);
    void resetKey(std::string);
    void KSA();
    unsigned char encrypt_decrypt(unsigned char);
    std::string encrypt_decrypt(std::string);





};

#endif //CHATRC4_RC4_H