#ifndef NS_EXCHANGE_H
#define NS_EXCHANGE_H

#include "../common/common.h"
#include "../crypto_utils/crypto_utils.h"
#include "../aes/aes.h"

extern unsigned char ns_session_key[AES_KEY_LEN];

std::string serialize_ns_session_request(int& sender_id, int& receiver_id, const std::string& nonce_b64);
void parse_ns_response_aes(const std::string& json_str, std::string& nonce_b64, std::string& ns_session_key_b64, std::string& ticket_b64, int& receiver_id, unsigned char *aes_hsm_key);
void ns_request_session_key(const std::string& hsm_ip, int hsm_port, const std::string& receiver_ns_id);
void ns_receive_ticket(std::string ticket_plain, int& sender_id, std::string& ns_session_key_b64, unsigned char *aes_hsm_key);
std::string serialize_ns_authentication_requests(const std::string& sender_id, const std::string& receiver_id, const std::string& nonce_b64);

#endif
