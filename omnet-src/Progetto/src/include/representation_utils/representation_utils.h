#ifndef REPRESENTATION_UTILS_H
#define REPRESENTATION_UTILS_H

#include <string>

#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/err.h>

namespace sca {

std::string base64_encode(const unsigned char* input, size_t len);
size_t base64_decode(
    const std::string& input,
    unsigned char* output,
    size_t max_len
);

}

#endif