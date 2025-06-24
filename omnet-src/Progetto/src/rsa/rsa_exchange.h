#ifndef RSA_EXCHANGE_H
#define RSA_EXCHANGE_H

#include "../common/common.h"
#include "../crypto_utils/crypto_utils.h"
//#include "socket_utils.h"

extern const char* ECU_PRIVKEY_PATH;
extern EVP_PKEY* ecu_privkey;


std::string serialize_rsa_request(int id);
void parse_rsa_response(const std::string& json_str, unsigned char* aes_key_enc);

void rsa_exchange_with_hsm(const std::string& hsm_ip, int hsm_port);
void ns_request_session_key(const std::string& hsm_ip, int hsm_port, const std::string& receiver_rsa_id);
void send_ticket_to_peer(const std::string& ecu2_ip, int port, const std::string& ticket_base64);
std::string ns_listen_for_ticket(int listen_port);
void ns_receive_ticket(const std::string& ticket_json_b64);

#endif

// RSA_EXCHANGE AUTH
/*

// extern const char* ECU_PUBKEY_PATH;
// extern const char* HSM_CERT_PATH;
// extern EVP_PKEY* ecu_pubkey;
// extern EVP_PKEY* hsm_pubkey;
*/
