#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "../common/common.h"

#define NONCE_LEN 16
#define AES_KEY_SIG_MAXLEN 256
#define AES_KEY_ENC_MAXLEN 256
#define ENC_LEN_MAX 256
#define SIG_LEN_MAX 256

enum class KeyExchangeType {
    RSA = 0, //con HSM
    NS = 1
};

void init_keys(const char* path, EVP_PKEY*& ecu_priv_key);
std::string base64_encode(const unsigned char* input, size_t len);
size_t base64_decode(const std::string& input, unsigned char* output, size_t max_len);
int rsa_encrypt_evp(EVP_PKEY* pubkey, const unsigned char* plaintext, size_t plaintext_len, unsigned char* ciphertext, size_t* ciphertext_len);
int rsa_decrypt_evp(EVP_PKEY* privkey, const unsigned char* ciphertext, size_t ciphertext_len, unsigned char* plaintext, size_t* plaintext_len);
void random_nonce(unsigned char* nonce, size_t len);

#endif
