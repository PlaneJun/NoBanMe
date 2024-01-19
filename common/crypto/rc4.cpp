//
// Created by Seyyidahmed on 4/8/18.
//

#include "RC4.h"
RC4::RC4() {
    this->i = this->j = 0;
}

RC4::RC4(std::string key) {
    if (key.length() > N) {
        std::cerr << "Key's Length must less or equal to " << N;
        exit(-1);
    }
    this->i = this->j = 0;
    this->key = key;
}

void RC4::KSA() {
    if (this->key.empty() or this->key.length() > N) {
        std::cout << "Key-scheduling algorithm failed" << std::endl;
        std::cerr << "[!] You must enter a valid key" << std::endl;
        exit(-1);
    }
    for (int k = 0; k < N; ++k)
        this->s[k] = (unsigned char)k;

    this->i = this->j = 0;
    for (; i < N; ++i) {
        j = (j + s[i] + key[i % key.length()]) % N;
        swap(i, j);
    }
    this->i = this->j = 0;
}

void RC4::swap(unsigned int a, unsigned int b) {
    unsigned char tmp = this->s[a];
    this->s[a] = this->s[b];
    this->s[b] = tmp;
}

unsigned char RC4::GenerateNextKeyByte() {
    this->i = (this->i + 1) % N;
    this->j = (this->j + this->s[i]) % N;
    swap(i, j);
    return this->s[(this->s[i] + this->s[j]) % N];
}
unsigned char RC4::xorByte(unsigned char b, unsigned char c) {
    return b ^ c;
}

unsigned char RC4::encrypt_decrypt(unsigned char p) {
    return xorByte(p, this->GenerateNextKeyByte());
}



std::string RC4::encrypt_decrypt(std::string plainText) {
    std::string cipherText = "";
    for (int k = 0; k < plainText.length(); ++k) {
        cipherText += encrypt_decrypt((unsigned char)plainText[k]);
    }
    return cipherText;
}
void RC4::resetRC4(std::string newKey) {
    this->key = "";
    this->i = this->j = 0;
    resetKey(newKey);
}

void RC4::resetKey(std::string newKey) {
    this->key = newKey;
}