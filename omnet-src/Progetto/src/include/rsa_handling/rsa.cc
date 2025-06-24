#include "rsa.h"

#include <iostream>

#include "../rapidjson/document.h"
#include "../rapidjson/stringbuffer.h"
#include "../rapidjson/writer.h"
#include "../rapidjson/error/en.h"

#include "../cryptoki.h"
#include "../sca.h"

std::string rsa_response(rapidjson::Document& doc, sca::Session& session) {
    using namespace rapidjson;

    if (!doc.HasMember("id") || !doc["id"].IsInt()) 
        std::cerr << "Errore parsing JSON (rsa_response)" << std::endl;

    CK_ULONG id = doc["id"].GetInt();

    CK_OBJECT_HANDLE ecu_public_key = {0};
    CK_ULONG key_count = 0;
    session.findKey(id, &ecu_public_key, &key_count);

    std::string label = "ECU" + std::to_string(doc["id"].GetInt());

    CK_OBJECT_HANDLE hsm_ecu_key = {0};
    CK_ULONG count = {0};
    session.findKey(label, &hsm_ecu_key, &count);
    if(count != 0) {
        session.destroyObject(hsm_ecu_key);
        std::cerr << "[INFO] Session key with this label present -> destroy..." << std::endl;
    }
    if(!session.createSessionKey(label, &hsm_ecu_key))
        std::cerr << "failed to create key" << label << std::endl;
    
    CK_BYTE_PTR wrapped_key;
    CK_ULONG wrapped_key_len;
    session.wrapSessionKey(
        ecu_public_key, 
        hsm_ecu_key, 
        wrapped_key, 
        &wrapped_key_len
    );
    
    std::string wrapped_key_b64 = sca::base64_encode(
        (const unsigned char*)wrapped_key, 
        wrapped_key_len
    );

    rapidjson::Document return_json;
    return_json.SetObject();
    auto& alloc = return_json.GetAllocator();

    return_json.AddMember("type", HSM_RSA_RESPONSE, alloc);
    return_json.AddMember("id", id, alloc);
    return_json.AddMember(
        "aes_key_enc", 
        rapidjson::Value().SetString(
            wrapped_key_b64.c_str(), 
            static_cast<rapidjson::SizeType>(wrapped_key_b64.length())
        ),
        alloc
    );

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    return_json.Accept(writer);

    return buffer.GetString();
}
