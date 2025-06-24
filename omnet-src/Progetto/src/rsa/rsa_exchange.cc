#include "rsa_exchange.h"

std::string serialize_rsa_request(int id) {
    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("type", static_cast<int>(KeyExchangeType::RSA), alloc);
    doc.AddMember("id", id, alloc);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    return buffer.GetString();
}

void parse_rsa_response(const std::string& json_str, unsigned char* aes_key_enc) {
    rapidjson::Document doc;
    if (doc.Parse(json_str.c_str()).HasParseError()) {
        handle_errors("Errore parsing JSON in parse_rsa_response");
    }

    if (!doc.HasMember("aes_key_enc") || !doc["aes_key_enc"].IsString()) {
        handle_errors("Campo 'aes_key_enc' mancante o non stringa");
    }
    base64_decode(doc["aes_key_enc"].GetString(), aes_key_enc, AES_KEY_ENC_MAXLEN);
}


// Scambio chiave di sessione ECU-HSM
void rsa_exchange_with_hsm() { // Funzione rsa_exchange con hsm
    /*
    std::string json_msg = serialize_rsa_request(); // Crea chiamata per inizio rsa_exchange
    std::string json_response = send_receive_json(hsm_ip, hsm_port, json_msg); // Manda chiamata e riceve chiave crittografata

    unsigned char aes_key_enc[AES_KEY_ENC_MAXLEN]; 
    parse_rsa_response(json_response, aes_key_enc); // Estrae chiave crittografata dal pacchetto

    size_t aes_key_len = AES_KEY_LEN;
    rsa_decrypt_evp(ecu_privkey, aes_key_enc, AES_KEY_ENC_MAXLEN, aes_hsm_key, &aes_key_len); // Decrypt chiave

    if (aes_key_len != AES_KEY_LEN)
        handle_errors("Chiave ricevuta con lunghezza errata");

    std::cout << "[" << local_ecu_id << "] Chiave ricevuta e decifrata con successo (HSM).\n";
    */
}


