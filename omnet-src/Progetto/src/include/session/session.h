#ifndef SESSION_H
#define SESSION_H
#include <string>
#include "../cryptoki.h"

// not implemented: C_GetOperationState, C_SetOperationState, C_GetObjectSize
//                  C_CopyObject, 

namespace sca {

class Session {
private:
    CK_SLOT_ID _id{0};
    CK_SESSION_HANDLE_PTR _session_handle{NULL_PTR};

    bool is_logged{false};
    bool session_active{false};

public:
    Session();
    ~Session();

    // CK_SESSION_HANDLE_PTR getSessionHandle() { return _session_handle; };

    bool beginReadOnly(CK_SLOT_ID id);
    bool beginReadWrite(CK_SLOT_ID id);
    bool end();

    bool loginUser(std::string pin);
    bool loginAdmin(std::string pin);
    bool logout();

    bool createObject(
        CK_OBJECT_HANDLE_PTR object_handle, 
        CK_ATTRIBUTE_PTR attr, 
        CK_ULONG count
    );
    bool destroyObject(CK_OBJECT_HANDLE_PTR object_handle);

    bool createAesKey(std::string key_label, std::string key);
    bool createSessionKey(std::string key_label, CK_OBJECT_HANDLE_PTR key_handle);
    bool generateKeyPair(
        CK_OBJECT_HANDLE& public_key, 
        std::string public_label, 
        CK_OBJECT_HANDLE& private_key,
        std::string private_lable,
        CK_ULONG key_length_bits
    );
    bool destroyObject(CK_OBJECT_HANDLE object_handle);

    bool findKey(
        std::string key_label, 
        CK_OBJECT_HANDLE_PTR object_handle, 
        CK_ULONG_PTR object_count
    );
    bool findKey(
        CK_ULONG id,
        CK_OBJECT_HANDLE_PTR object_handle, 
        CK_ULONG_PTR object_count 
    );
    bool getKeyValue(
        CK_OBJECT_HANDLE key_handle,
        CK_BYTE_PTR& key_raw,
        CK_ULONG_PTR key_len
    );

    bool createX509Certificate(
        CK_UTF8CHAR_PTR label, 
        CK_ULONG label_size,
        CK_BYTE_PTR owner,
        CK_ULONG owner_size,
        CK_BYTE_PTR certificate,
        CK_ULONG certificate_size
    );
    
    bool encryptAesGCM(
        CK_OBJECT_HANDLE key,
        CK_BYTE_PTR data, 
        CK_ULONG data_len, 
        std::string aad_str,
        CK_BYTE_PTR& encrypted_data, 
        CK_ULONG_PTR encrypted_data_len
    );
    bool encryptAesGCMParam( 
        CK_OBJECT_HANDLE key,
        CK_BYTE_PTR data, 
        CK_ULONG data_len, 
        CK_BYTE_PTR& encrypted_data, 
        CK_ULONG_PTR encrypted_data_len,
        CK_BYTE_PTR& aad,
        CK_ULONG_PTR aad_size,
        CK_BYTE_PTR& tag,
        CK_ULONG_PTR tag_size,
        CK_BYTE_PTR& iv,
        CK_ULONG_PTR iv_size
    );
    bool decryptAesGCM(
        CK_OBJECT_HANDLE key,
        CK_BYTE_PTR data, 
        CK_ULONG data_len, 
        std::string aad_str,
        CK_BYTE_PTR& decrypted_data, 
        CK_ULONG_PTR decrypted_data_len
    );

    bool encryptRsa(
        CK_OBJECT_HANDLE key,
        CK_BYTE_PTR data, 
        CK_ULONG data_len, 
        CK_BYTE_PTR& encrypted_data, 
        CK_ULONG_PTR encrypted_data_len
    );
    bool decryptRsa(
        CK_OBJECT_HANDLE key,
        CK_BYTE_PTR data, 
        CK_ULONG data_len, 
        CK_BYTE_PTR& decrypted_data, 
        CK_ULONG_PTR decrypted_data_len
    );

    bool sign(
        CK_OBJECT_HANDLE key,
        CK_BYTE_PTR data, 
        CK_ULONG data_len,
        CK_BYTE_PTR& signature, 
        CK_ULONG_PTR signature_len
    );
    bool verifyRsa(
        CK_OBJECT_HANDLE key,
        CK_BYTE_PTR data, 
        CK_ULONG data_len,
        CK_BYTE_PTR signature, 
        CK_ULONG signature_len
    );
    bool wrapSessionKey(
        CK_OBJECT_HANDLE wrapping_key,
        CK_OBJECT_HANDLE key_to_wrap,
        CK_BYTE_PTR& wrapped_key,
        CK_ULONG_PTR wrapped_key_len
    );

    CK_BYTE hexdigit_to_int(char ch);
    void print_bytes_as_hex(unsigned char *bytes, size_t len);

    bool seedRandom(CK_BYTE_PTR seed, CK_ULONG seed_len);
    bool generateRandom(CK_BYTE_PTR random, CK_ULONG random_len);
};

}
#endif
