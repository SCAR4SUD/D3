#include "ns_exchange.h"

unsigned char ns_session_key[AES_KEY_LEN];

std::string serialize_ns_session_request(int& sender_id, int& receiver_id, const std::string& nonce_b64) {
    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("type", NS_REQUEST, alloc);
    doc.AddMember("sender_id",
        sender_id,
        alloc);
    doc.AddMember("receiver_id",
        receiver_id,
        alloc);
    doc.AddMember("nonce",
        rapidjson::Value().SetString(nonce_b64.c_str(), static_cast<rapidjson::SizeType>(nonce_b64.length()), alloc),
        alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    return buffer.GetString();
}

void parse_ns_response_aes(const std::string& json_str, std::string& nonce_b64, std::string& ns_session_key_b64, std::string& ticket_b64, int& receiver_id, unsigned char *aes_hsm_key)
{
    rapidjson::Document doc;
    if (doc.Parse(json_str.c_str()).HasParseError())
        handle_errors("JSON non valido");

    size_t plain_len = 0;
    const unsigned char* plaintext = decrypt_message_aes(doc, plain_len, aes_hsm_key);
    if (!plaintext || plain_len == 0) {
        handle_errors("Decrittazione AES fallita");
        return;
    }

    rapidjson::Document doc_plain;
    if (doc_plain.Parse(reinterpret_cast<const char*>(plaintext), plain_len).HasParseError())
        handle_errors("JSON interno non valido");

    if (
        !doc_plain.HasMember("nonce") ||
        !doc_plain["nonce"].IsString() ||
        !doc_plain.HasMember("ns_session_key_enc") ||
        !doc_plain["ns_session_key_enc"].IsString() ||
        !doc_plain.HasMember("ticket_enc") ||
        !doc_plain["ticket_enc"].IsString() ||
        !doc_plain.HasMember("receiver_id") ||
        !doc_plain["receiver_id"].IsInt()
    )
        handle_errors("Campi mancanti nella risposta decifrata");

    
    nonce_b64             = doc_plain["nonce"].GetString();
    ns_session_key_b64    = doc_plain["ns_session_key_enc"].GetString();
    ticket_b64            = doc_plain["ticket_enc"].GetString();
    receiver_id           = doc_plain["receiver_id"].GetInt();
}


// Inizio NS
// A -> S {A, B, Na}
// S -> A {Na, Kab, B, {Kab, A}Kbs }Kas
// A -> B {Kab, A}Kbs

// MUTUA AUTENTICAZIONE
// B -> A {Nb}Kab
// A -> B {Nb-1}Kab


std::string serialize_ns_authentication_requests(const std::string& sender_id, const std::string& receiver_id, const std::string& nonce_b64)
{
    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("type", NS_AUTH_REQUEST, alloc);
    doc.AddMember("sender_rsa_id",
        rapidjson::Value().SetString(sender_id.c_str(), static_cast<rapidjson::SizeType>(sender_id.length()), alloc),
        alloc);
    doc.AddMember("receiver_rsa_id",
        rapidjson::Value().SetString(receiver_id.c_str(), static_cast<rapidjson::SizeType>(receiver_id.length()), alloc),
        alloc);
    doc.AddMember("nonce",
        rapidjson::Value().SetString(nonce_b64.c_str(), static_cast<rapidjson::SizeType>(nonce_b64.length()), alloc),
        alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    return buffer.GetString();
}

void ns_request_session_key(const std::string& hsm_ip, int hsm_port, const std::string& receiver_ns_id) {   //Funzione per il chiamante
    /*

    if (receiver_ns_id.empty()) {
        handle_errors("receiver_rsa_id richiesto per lo scambio ECU - ECU");
    }

    unsigned char nonce[NONCE_LEN];
    random_nonce(nonce, NONCE_LEN); // Generazione nonce
    
    std::string nonce_b64 = base64_encode(nonce, NONCE_LEN); // Serializzazione per invio
    std::string json_msg = serialize_ns_session_request(local_ecu_id, receiver_ns_id, nonce_b64); // Serializzazione per invio

    std::string json_response = send_receive_json(hsm_ip, hsm_port, json_msg); // Invio al server

    std::string nonce_b64_resp, ns_session_key_b64, ticket_b64, resp_receiver_id;
    parse_ns_response_aes(json_response, nonce_b64_resp, ns_session_key_b64, ticket_b64, resp_receiver_id); // Parse e decrypt annidato della risposta

    if (resp_receiver_id != receiver_ns_id) {                                           // Confronto receiver originale, con resp_receiver
        handle_errors("Receiver ID dalla risposta non corrisponde a quello richiesto");  
    }

    unsigned char nonce_resp[NONCE_LEN];
    base64_decode(nonce_b64_resp.c_str(), nonce_resp, NONCE_LEN);
    if (memcmp(nonce_resp, nonce, NONCE_LEN) != 0)                  // Confronto nonce per verifica
        handle_errors("Nonce non corrisponde nella risposta da HSM");

    size_t ns_key_len = base64_decode(ns_session_key_b64.c_str(), ns_session_key, AES_KEY_LEN);
    if (ns_key_len != AES_KEY_LEN)
        handle_errors("Lunghezza chiave errata nella decodifica della session key");

    send_ticket_to_peer(ECU2_IP, ECU2_PORT, ticket_b64); //Invio del ticket ricevuto dall'hsm, con destinatario ecu2

    std::cout << "[" << local_ecu_id << "] Chiave condivisa con [" << receiver_ns_id << "] ottenuta via Needham-Schroeder.\n";
    */
}

void ns_receive_ticket(std::string ticket_json_str, int& sender_id, std::string& ns_session_key_b64, unsigned char *aes_hsm_key) {
    rapidjson::Document ticket;                                           // Parsing ticket 
    if (ticket.Parse(ticket_json_str.c_str()).HasParseError()) {
        handle_errors("Ticket JSON non valido");
    }

    size_t plain_len = 0;
    const unsigned char* plaintext = decrypt_message_aes(ticket, plain_len, aes_hsm_key);    // Decrypt ticket
    if (!plaintext || plain_len == 0) {
        handle_errors("Decrittazione ticket AES fallita");
    }

    std::string ticket_plain(reinterpret_cast<const char*>(plaintext), plain_len);
    rapidjson::Document doc;
    if (doc.Parse(ticket_plain.c_str()).HasParseError())                              // Parsing interno di ticket_plain
        handle_errors("Ticket JSON interno non valido");
    if (!doc.HasMember("sender_id") || !doc["sender_id"].IsInt())
        handle_errors("Campo sender_id mancante nel ticket");
    if (!doc.HasMember("ns_session_key_b64") || !doc["ns_session_key_b64"].IsString())
        handle_errors("Campo ns_session_key_enc mancante nel ticket");

    sender_id = doc["sender_id"].GetInt();                            // Sender retreiving
    ns_session_key_b64 = doc["ns_session_key_b64"].GetString();                   // Key retreiving
}
