#ifndef AES_H
#define AES_H

#include "../common/common.h"
#include "../crypto_utils/crypto_utils.h"
//#include "socket_utils.h"

#define IV_LEN 12
#define TAG_LEN 16
#define MAX_PLAINTEXT_LEN 16384

struct AesEncryptedMessage {
    unsigned char iv[IV_LEN];
    unsigned char tag[TAG_LEN];
    unsigned char ciphertext[MAX_PLAINTEXT_LEN];
    int ciphertext_len;
    char aad[ECUID_LEN]; 
};


AesEncryptedMessage encrypt_message_aes(const unsigned char* plaintext, size_t plaintext_len, unsigned char *aes_hsm_key);
void send_message(const std::string& ip, int port, const std::string& json_msg);
std::string serialize_aes_message(const AesEncryptedMessage& msg, int sender_id, int receiver_id, int type);
std::string receive_message(int port);
const unsigned char* decrypt_message_aes(const rapidjson::Document& message, size_t& out_len, unsigned char *aes_hsm_key);

#endif 
