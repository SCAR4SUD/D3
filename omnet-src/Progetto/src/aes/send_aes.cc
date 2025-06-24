#include "aes.h"

// Utilizzata per vecchia logica 
AesEncryptedMessage encrypt_message_aes(const unsigned char* plaintext, size_t plaintext_len, unsigned char *aes_hsm_key) {
    if (plaintext_len > MAX_PLAINTEXT_LEN)
        handle_errors("plaintext troppo lungo");

    AesEncryptedMessage msg{};
    if (RAND_bytes(msg.iv, IV_LEN) != 1)
        handle_errors("generazione IV");

    strncpy(msg.aad, local_ecu_id.c_str(), ECUID_LEN);
    msg.aad[ECUID_LEN - 1] = '\0';

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) handle_errors("allocazione EVP_CIPHER_CTX");

    int len;
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
        handle_errors("init AES GCM");

    if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, aes_hsm_key, msg.iv) != 1)
        handle_errors("init chiave e IV");

    if (EVP_EncryptUpdate(ctx, nullptr, &len, reinterpret_cast<const unsigned char*>(msg.aad), ECUID_LEN) != 1)
        handle_errors("AAD");

    if (EVP_EncryptUpdate(ctx, msg.ciphertext, &len, plaintext, plaintext_len) != 1)
        handle_errors("cifratura");

    int ciphertext_len = len;
    if (EVP_EncryptFinal_ex(ctx, msg.ciphertext + len, &len) != 1) 
        handle_errors("finale cifratura");

    ciphertext_len += len;
    msg.ciphertext_len = ciphertext_len;
    if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, msg.tag) != 1)
        handle_errors("estrazione tag");

    EVP_CIPHER_CTX_free(ctx);
    return msg;
}

 std::string serialize_aes_message(const AesEncryptedMessage& msg, int sender_id, int receiver_id, int type) {

    std::string iv_b64  = base64_encode(msg.iv, IV_LEN);
    std::string ct_b64  = base64_encode(msg.ciphertext, msg.ciphertext_len);
    std::string tag_b64 = base64_encode(msg.tag, TAG_LEN);
    std::string aad_b64 = base64_encode(reinterpret_cast<const unsigned char*>(msg.aad), ECUID_LEN);

    rapidjson::Document doc;
    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("sender_id", sender_id, alloc);
    doc.AddMember("receiver_id", receiver_id, alloc);
    doc.AddMember("type", type, alloc);
    doc.AddMember("iv", rapidjson::Value().SetString(iv_b64.c_str(), alloc), alloc);
    doc.AddMember("ciphertext", rapidjson::Value().SetString(ct_b64.c_str(), alloc), alloc);
    doc.AddMember("tag", rapidjson::Value().SetString(tag_b64.c_str(), alloc), alloc);
    doc.AddMember("aad", rapidjson::Value().SetString(aad_b64.c_str(), alloc), alloc);

    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    doc.Accept(writer);
    return buf.GetString();
}


void send_message(const std::string& ip, int port, const std::string& json_msg) {
    /*
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) handle_errors("socket");

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0)
        handle_errors("IP non valido");

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        handle_errors("connessione");

    ssize_t sent = send(sock, json_msg.c_str(), json_msg.size(), 0);
    if (sent != (ssize_t)json_msg.size())
        handle_errors("invio incompleto");

    close(sock);
    */
}
