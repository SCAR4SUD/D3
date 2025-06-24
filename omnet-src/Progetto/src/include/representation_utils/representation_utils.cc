#include "../../include/representation_utils/representation_utils.h"

#include <iostream>

namespace sca {

std::string base64_encode(const unsigned char* input, size_t len) {
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    b64 = BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, input, len);
    BIO_flush(b64);
    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(b64, &buffer_ptr);
    std::string encoded(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(b64);
    return encoded;
}

size_t base64_decode(
    const std::string& input, 
    unsigned char* output, 
    size_t max_len
) {
    BIO* bio = BIO_new_mem_buf(input.data(), input.length());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int decoded_len = BIO_read(bio, output, max_len);
    BIO_free_all(bio);
    if (decoded_len <= 0) {
        std::cerr << "Errore decodifica Base64" << std::endl;
        return -1;
    }
    if (static_cast<size_t>(decoded_len) > max_len) {
        std::cerr << "Decodifica Base64 troppo lunga per il buffer" << std::endl;
        return -1;
    }
    return static_cast<size_t>(decoded_len);
}

}