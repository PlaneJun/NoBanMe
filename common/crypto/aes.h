#ifndef _AES_H
#define _AES_H

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif

#define BLOCK_SIZE 16

typedef struct
{
    uint32 erk[64];     /* encryption round keys */
    uint32 drk[64];     /* decryption round keys */
    int nr;             /* number of rounds */
}
aes_context;

namespace aes {
    int  aes_set_key(aes_context* ctx, uint8* key, int nbits);
    void aes_encrypt(aes_context* ctx, uint8 input[16], uint8 output[16]);
    void aes_decrypt(aes_context* ctx, uint8 input[16], uint8 output[16]);

    std::string encrypt(const std::string& plaintext, const std::string& skey);
    std::string decrypt(const std::string& basestr, const std::string& skey); std::string decrypt(const std::string& basestr, const std::string& skey);
};


#endif /* aes.h */