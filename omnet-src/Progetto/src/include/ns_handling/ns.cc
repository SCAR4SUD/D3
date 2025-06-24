#include "ns.h"

#include <string>
#include <iostream>
#include "../rapidjson/document.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/error/en.h"

#include "../cryptoki.h"
#include "../sca.h"

std::string ns_response(rapidjson::Document& doc, sca::Session& session) {
    using namespace rapidjson;

    if (!doc.HasMember("type") || !doc["type"].IsInt()) 
        std::cerr << "Errore parsing JSON (ns_response)" << "\n\n";
    
    CK_ULONG sender_id = doc["sender_id"].GetInt();
    CK_ULONG receiver_id = doc["receiver_id"].GetInt();
/*
    unsigned char iv[IV_LEN];
    unsigned char tag[TAG_LEN];
    unsigned char ciphertext[16384];
    unsigned char aad[AAD_NIST_MAX_SIZE];

    sca::base64_decode(doc["iv"].GetString(),            iv,         IV_LEN);
    sca::base64_decode(doc["tag"].GetString(),           tag,        TAG_LEN);
    size_t cipher_text_len = sca::base64_decode(doc["ciphertext"].GetString(),    ciphertext, sizeof(ciphertext));
    sca::base64_decode(doc["aad"].GetString(),           aad,        AAD_NIST_MAX_SIZE);

    CK_ULONG encrypted_data_len = IV_LEN + cipher_text_len + TAG_LEN;
    CK_BYTE_PTR encrypted_data = new CK_BYTE[encrypted_data_len];
    memcpy(encrypted_data, iv, IV_LEN);
    memcpy(encrypted_data + IV_LEN, ciphertext, cipher_text_len);
    memcpy(encrypted_data + IV_LEN + cipher_text_len, tag, TAG_LEN);
*/
    CK_OBJECT_HANDLE req_ecu_hsm_key{0};
    CK_ULONG req_ecu_hsm_key_count{0};
    session.findKey("ECU"+std::to_string(sender_id), &req_ecu_hsm_key, &req_ecu_hsm_key_count);
    //std::cout << "req_ecu_hsm_key: " << req_ecu_hsm_key << "\n\n" << std::flush;

    CK_OBJECT_HANDLE rcv_ecu_hsm_key{0};
    CK_ULONG rcv_ecu_hsm_key_count{0};
    session.findKey("ECU"+std::to_string(receiver_id), &rcv_ecu_hsm_key, &rcv_ecu_hsm_key_count);
    //std::cout << "rcv_ecu_hsm_key: " << rcv_ecu_hsm_key << "\n\n" << std::flush;

/*
    CK_BYTE_PTR decrypted_data;
    CK_ULONG decrypted_data_len;
    if(!session.decryptAesGCM(
        req_ecu_hsm_key, 
        encrypted_data,
        encrypted_data_len,
        std::string((char *)aad, ECUID_LEN),
        decrypted_data,
        &decrypted_data_len
    ))
        std::cerr << "\033[1;31m[ERROR] failed first decryptionn\033[0m" << std::endl;

    std::string json_request((const char *)decrypted_data, decrypted_data_len);
    //std::cout << json_request << "\n\n";
    rapidjson::Document doc_request;
    if (doc_request.Parse(json_request.c_str()).HasParseError()) 
        std::cerr << "Errore parsing JSON" << "\n\n";
*/
    //std::string nonce_b64 = doc_request["nonce"].GetString();
    std::string nonce_b64 = doc["nonce"].GetString();

    std::string session_key_label = "ECU" + std::to_string(sender_id) +
                                    "ECU" + std::to_string(receiver_id);
    CK_OBJECT_HANDLE session_key_handle;
    CK_ULONG session_key_count = {0};
    session.findKey(session_key_label, &session_key_handle, &session_key_count);
    if(session_key_count != 0)
        session.destroyObject(session_key_handle);
    session.createSessionKey(session_key_label, &session_key_handle);

    CK_BYTE_PTR session_key{nullptr};
    CK_ULONG session_key_len{0};
    session.getKeyValue(session_key_handle, session_key, &session_key_len);
    std::string session_key_b64 = sca::base64_encode(
        (const unsigned char*)session_key,
        session_key_len
    );
    session.destroyObject(session_key_handle);

    // generate message to B
    
    rapidjson::Document message_to_b;
    message_to_b.SetObject();
    auto& alloc = message_to_b.GetAllocator();

    message_to_b.AddMember("sender_id", sender_id, alloc);
    message_to_b.AddMember(
        "ns_session_key_b64", 
        rapidjson::Value().SetString(session_key_b64.c_str(), session_key_b64.length()), 
        alloc
    );

    rapidjson::StringBuffer buffer_message_to_b;
    rapidjson::Writer<rapidjson::StringBuffer> writer_to_b(buffer_message_to_b);
    message_to_b.Accept(writer_to_b);

    std::string message_to_b_str = buffer_message_to_b.GetString();
    //std::cout << "message_to_b_str: " << message_to_b_str << "\n\n";

    CK_BYTE_PTR iv_b{nullptr};
    CK_ULONG iv_b_len{0};
    CK_BYTE_PTR tag_b{nullptr};
    CK_ULONG tag_b_len{0};
    CK_BYTE_PTR aad_b{nullptr};
    CK_ULONG aad_b_len{0};
    memset(aad_b, 0, aad_b_len);

    CK_BYTE_PTR ciphertext_b{nullptr};
    CK_ULONG ciphertext_len_b{0};
    
    session.encryptAesGCMParam(
        rcv_ecu_hsm_key,
        (CK_BYTE_PTR)message_to_b_str.c_str(),
        message_to_b_str.length(),
        ciphertext_b,
        &ciphertext_len_b,
        aad_b,
        &aad_b_len,
        tag_b,
        &tag_b_len,
        iv_b,
        &iv_b_len
    );

    std::string ciphertext_b_b64 = sca::base64_encode(ciphertext_b, ciphertext_len_b);
    //std::cout << "ciphertext_b_b64: " << ciphertext_b_b64 << "\n\n";

    rapidjson::Document aes_message_to_b;
    aes_message_to_b.SetObject();
    auto& alloc_aes_b = aes_message_to_b.GetAllocator(); 

    std::string iv_b_str = sca::base64_encode(iv_b, iv_b_len);
    std::string tag_b_str = sca::base64_encode(tag_b, tag_b_len);
    std::string aad_b_str = sca::base64_encode(aad_b, aad_b_len);

    aes_message_to_b.AddMember(
        "type", 
        HSM_NS_RESPONSE_RECEIVER, 
        alloc_aes_b
    );
    aes_message_to_b.AddMember(
        "aad", 
        rapidjson::Value().SetString(aad_b_str.c_str(), aad_b_str.length()), 
        alloc_aes_b
    );
    aes_message_to_b.AddMember(
        "iv", 
        rapidjson::Value().SetString(iv_b_str.c_str(), iv_b_str.length()), 
        alloc_aes_b
    );
    aes_message_to_b.AddMember(
        "ciphertext", 
        rapidjson::Value().SetString(ciphertext_b_b64.c_str(), ciphertext_b_b64.length()), 
        alloc_aes_b
    );
    aes_message_to_b.AddMember(
        "tag", 
        rapidjson::Value().SetString(tag_b_str.c_str(), tag_b_str.length()), 
        alloc_aes_b
    );

    rapidjson::StringBuffer buffer_aes_message_to_b;
    rapidjson::Writer<rapidjson::StringBuffer> aes_writer_to_b(buffer_aes_message_to_b);
    aes_message_to_b.Accept(aes_writer_to_b);

    std::string aes_message_to_b_str = buffer_aes_message_to_b.GetString();
    //std::cout << "aes_message_to_b_str: " << aes_message_to_b_str << "\n\n";

    // message to A

    std::string aes_message_to_b_str_b64 = sca::base64_encode(
        (unsigned char *)aes_message_to_b_str.c_str(), 
        aes_message_to_b_str.length()
    );

    rapidjson::Document message_to_a;
    message_to_a.SetObject();
    auto& alloc_a = message_to_a.GetAllocator(); 

    message_to_a.AddMember(
        "nonce", 
        rapidjson::Value().SetString(nonce_b64.c_str(), nonce_b64.length()), 
        alloc_a);
    message_to_a.AddMember(
        "ns_session_key_enc", 
        rapidjson::Value().SetString(session_key_b64.c_str(), session_key_b64.length()), alloc_a
    );
    message_to_a.AddMember(
        "ticket_enc", 
        rapidjson::Value().SetString(aes_message_to_b_str_b64.c_str(), aes_message_to_b_str_b64.length()), 
        alloc_a
    );
    message_to_a.AddMember("receiver_id", receiver_id, alloc_a);

    rapidjson::StringBuffer buffer_message_to_a;
    rapidjson::Writer<rapidjson::StringBuffer> writer_to_a(buffer_message_to_a);
    message_to_a.Accept(writer_to_a);

    std::string message_to_a_str = buffer_message_to_a.GetString();
    //std::cout << "message_to_a_str: " << message_to_a_str << "\n\n";


    CK_BYTE_PTR iv_a{nullptr};
    CK_ULONG iv_a_len{0};
    CK_BYTE_PTR tag_a{nullptr};
    CK_ULONG tag_a_len{0};
    CK_BYTE_PTR aad_a{nullptr};
    CK_ULONG aad_a_len{0};
    CK_BYTE_PTR ciphertext_a{nullptr};
    CK_ULONG ciphertext_len_a{0};
    

    session.encryptAesGCMParam(
        req_ecu_hsm_key,
        (CK_BYTE_PTR)message_to_a_str.c_str(),
        message_to_a_str.length(),
        ciphertext_a,
        &ciphertext_len_a,
        aad_a,
        &aad_a_len,
        tag_a,
        &tag_a_len,
        iv_a,
        &iv_a_len
    );

    std::string ciphertext_a_b64 = sca::base64_encode(ciphertext_a, ciphertext_len_a);
    //std::cout << "ciphertext_a_b64: " << ciphertext_a_b64 << "\n\n" << std::flush;

    rapidjson::Document aes_message_to_a;
    aes_message_to_a.SetObject();
    auto& alloc_aes_a = aes_message_to_a.GetAllocator(); 

    std::string iv_a_str = sca::base64_encode(iv_a, iv_a_len);
    std::string tag_a_str = sca::base64_encode(tag_a, tag_a_len);
    std::string aad_a_str = sca::base64_encode(aad_a, aad_a_len);

    aes_message_to_a.AddMember(
        "type", 
        HSM_NS_RESPONSE_SENDER, 
        alloc_aes_a
    );
    aes_message_to_a.AddMember(
        "sender_id", 
        0, 
        alloc_aes_a
    );
    aes_message_to_a.AddMember(
        "receiver_id", 
        sender_id, 
        alloc_aes_a
    );
    aes_message_to_a.AddMember(
        "aad", 
        rapidjson::Value().SetString(aad_a_str.c_str(), aad_a_str.length()), 
        alloc_aes_a
    );
    aes_message_to_a.AddMember(
        "iv", 
        rapidjson::Value().SetString(iv_a_str.c_str(), iv_a_str.length()), 
        alloc_aes_a
    );
    aes_message_to_a.AddMember(
        "ciphertext", 
        rapidjson::Value().SetString(ciphertext_a_b64.c_str(), ciphertext_a_b64.length()), 
        alloc_aes_a
    );
    aes_message_to_a.AddMember(
        "tag", 
        rapidjson::Value().SetString(tag_a_str.c_str(), tag_a_str.length()), 
        alloc_aes_a
    );

    rapidjson::StringBuffer buffer_aes_message_to_a;
    rapidjson::Writer<rapidjson::StringBuffer> aes_writer_to_a(buffer_aes_message_to_a);
    aes_message_to_a.Accept(aes_writer_to_a);

    return buffer_aes_message_to_a.GetString();
}
