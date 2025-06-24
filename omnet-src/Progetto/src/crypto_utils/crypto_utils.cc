#include "crypto_utils.h"

const char* ECU_PRIVKEY_PATH = "keys/ecu_private.pem";
EVP_PKEY* ecu_privkey = nullptr;

void random_nonce(unsigned char* nonce, size_t len) {
    if (RAND_bytes(nonce, len) != 1) handle_errors("Errore generazione nonce");
}

void init_keys(const char* path, EVP_PKEY*& ecu_priv_key) {
    FILE* f_priv = fopen(path, "r");
    if (!f_priv) handle_errors("apertura chiave privata ECU");
    ecu_priv_key = PEM_read_PrivateKey(f_priv, nullptr, nullptr, nullptr);
    fclose(f_priv);
    if (!ecu_priv_key) handle_errors("lettura chiave privata ECU");
}

int rsa_encrypt_evp(EVP_PKEY* pubkey, const unsigned char* plaintext, size_t plaintext_len, unsigned char* ciphertext, size_t* ciphertext_len) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pubkey, nullptr);
    if (!ctx) handle_errors("allocazione EVP_PKEY_CTX");

    if (EVP_PKEY_encrypt_init(ctx) <= 0)
        handle_errors("encrypt_init");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
        handle_errors("padding");

    if (EVP_PKEY_encrypt(ctx, nullptr, ciphertext_len, plaintext, plaintext_len) <= 0)
        handle_errors("encrypt (len)");

    if (EVP_PKEY_encrypt(ctx, ciphertext, ciphertext_len, plaintext, plaintext_len) <= 0)
        handle_errors("encrypt (data)");

    EVP_PKEY_CTX_free(ctx);
    return 1;
}

int rsa_decrypt_evp(EVP_PKEY* privkey, const unsigned char* ciphertext, size_t ciphertext_len, unsigned char* plaintext, size_t* plaintext_len) {
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(privkey, nullptr);
    if (!ctx) handle_errors("allocazione EVP_PKEY_CTX (decrypt)");

    if (EVP_PKEY_decrypt_init(ctx) <= 0)
        handle_errors("decrypt_init");

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
        handle_errors("padding decrypt");

    if (EVP_PKEY_decrypt(ctx, nullptr, plaintext_len, ciphertext, ciphertext_len) <= 0)
        handle_errors("decrypt (len)");

    int res = EVP_PKEY_decrypt(ctx, plaintext, plaintext_len, ciphertext, ciphertext_len);
    if (res <= 0) {
        handle_errors("decrypt (data)");
        return res;
    }

    EVP_PKEY_CTX_free(ctx);
    return 1;
}

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

size_t base64_decode(const std::string& input, unsigned char* output, size_t max_len) {
    BIO* bio = BIO_new_mem_buf(input.data(), input.length());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int decoded_len = BIO_read(bio, output, max_len);
    BIO_free_all(bio);
    if (decoded_len <= 0) {
        handle_errors("Errore decodifica Base64");
    }
    if (static_cast<size_t>(decoded_len) > max_len) {
        handle_errors("Decodifica Base64 troppo lunga per il buffer");
    }
    return static_cast<size_t>(decoded_len);
}

