#include "session.h"

#include <string>
#include <cstring>
#include <iostream>

#include "../return_check/return_check.h"
#include "../cryptoki.h"

// #define DEBUG

#define AES_GCM_IV_SIZE 12
#define AES_GCM_TAG_SIZE 16

#define AAD_NIST_MAX_SIZE 8

namespace sca {

Session::Session() {
    _session_handle = new CK_SESSION_HANDLE;
}

Session::~Session() {
    //if(mechanism != NULL_PTR) delete mechanism;
    if(is_logged) logout();
    if(session_active) end();
    delete _session_handle;
}

bool Session::beginReadOnly(CK_SLOT_ID id) {
    CK_RV rv = C_OpenSession(
        id,
        CKF_SERIAL_SESSION, // read only
        NULL_PTR,           // application-defined pointer to notify call
        NULL_PTR,           // address of the notification callback function
        _session_handle
    );
    return_check(rv, "C_OpenSession (Session::Session)");
    if(rv != CKR_OK) {
        return false;
    } else {
        _id = id;
        session_active = true;
        return true;
    }
}

bool Session::beginReadWrite(CK_SLOT_ID id) {
    CK_RV rv = C_OpenSession(
        id,
        CKF_SERIAL_SESSION | CKF_RW_SESSION,    // read write
        NULL_PTR,
        NULL_PTR,
        _session_handle
    );
    return_check(rv, "C_OpenSession (Session::Session)");
    if(rv == CKR_SESSION_READ_WRITE_SO_EXISTS) 
        return false;
    else {
        session_active = true;
        _id = id;
        return true;
    }
}

bool Session::end() {
    CK_RV rv = C_CloseSession(*_session_handle);
    return_check(rv, "C_CloseSession (Session::end)");
    if(rv != CKR_OK) return false;
    else {
        session_active = false;
        return true;
    }
}

bool Session::loginUser(std::string pin) {
    CK_UTF8CHAR_PTR user_pin = (unsigned char*)pin.c_str();
    CK_RV rv = C_Login(
        *_session_handle, 
        CKU_USER, 
        user_pin, 
        (pin.size())
    );
    return_check(rv, "C_Login (Session::loginUser)");
    if(rv != CKR_OK) return false;
    else {
        is_logged = true;
        return true;
    }
}

bool Session::loginAdmin(std::string pin) {
    CK_UTF8CHAR_PTR admin_pin = (unsigned char*)pin.c_str();
    CK_RV rv = C_Login(
        *_session_handle, 
        CKU_SO, 
        admin_pin, 
        (pin.size())
    );
    return_check(rv, "C_Login (Session::loginUser)");
    if(rv != CKR_OK) 
        return false;
    else {
        is_logged = true;
        return true;
    }
}

bool Session::logout() {
    if(
        return_check(
            C_Logout(*_session_handle), 
            "C_Logout (Session::logout)"
        ) != CKR_OK
    ) 
        return false;
    else {
        is_logged = false;
        return true;
    }
}

bool Session::createObject(
    CK_OBJECT_HANDLE_PTR object_handle, 
    CK_ATTRIBUTE_PTR attr, 
    CK_ULONG count
) {
    CK_RV rv = C_CreateObject(
                    *_session_handle,
                    attr,
                    count,
                    object_handle
                );
    if(return_check(rv, "C_CreateObject (Session::createObject)") != CKR_OK) {
        return false;
    }     
    return true;
}

bool Session::destroyObject(CK_OBJECT_HANDLE_PTR object_handle) {
    CK_RV rv = C_DestroyObject(*_session_handle, *object_handle);
    if(return_check(rv, "C_DestroyObject (Session::destroyObject)") != CKR_OK) 
        return false;
    else 
        return true;
}

CK_BYTE Session::hexdigit_to_int(char ch)
{
    switch (ch)
    {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            return -1;
    }
}

bool Session::createAesKey(std::string key_label, std::string key) {
    CK_ULONG count{0};
    CK_OBJECT_HANDLE handle[512] = {0};
    findKey(key_label, handle, &count);
    if(count != 0) {
        std::cerr   << "There is already a key labeled: "\
                    << key_label << std::endl;
        return false;
    }

    CK_BYTE key_value[32];
    for(unsigned short i = 0; i < 32; i++) {
        key_value[i] =  hexdigit_to_int(key[i*2])*16 +\
                        hexdigit_to_int(key[(i*2)+1]);
    }

    CK_OBJECT_HANDLE key_handle;

    CK_OBJECT_CLASS object_class = CKO_SECRET_KEY;
    CK_BBOOL yes = CK_TRUE;
    CK_BBOOL no = CK_FALSE;
    CK_KEY_TYPE key_type = CKK_AES;
    CK_BYTE_PTR label = (CK_BYTE_PTR)key_label.c_str(); 
    CK_ULONG key_size = 32;

    CK_ATTRIBUTE attributes[] = {
        {CKA_CLASS,         &object_class,  sizeof(object_class)},
        {CKA_KEY_TYPE,      &key_type,      sizeof(key_type)},
        {CKA_LABEL,         label,          strlen((const char *)label)},
        {CKA_TOKEN,         &yes,           sizeof(CK_BBOOL)},
        {CKA_ENCRYPT,       &yes,           sizeof(CK_BBOOL)},
        {CKA_DECRYPT,       &yes,           sizeof(CK_BBOOL)},
        {CKA_SENSITIVE,     &no,            sizeof(CK_BBOOL)},
        {CKA_VALUE,         &key_value,     key_size},
    };

    CK_ULONG attributes_len = sizeof(attributes) / sizeof(CK_ATTRIBUTE);

    CK_RV rv = C_CreateObject(
        *_session_handle, 
        attributes, 
        attributes_len, 
        &key_handle
    );

    if(return_check(rv, "C_CreateObject (Session::createAesKey)") != CKR_OK) 
        return false;
    else 
        return true;
}

bool Session::createSessionKey(std::string key_label, CK_OBJECT_HANDLE_PTR key_handle) {
    CK_ULONG count{0};
    CK_OBJECT_HANDLE handle[512] = {0};
    findKey(key_label, handle, &count);
    if(count != 0) {
        std::cerr   << "There is already a key with label: "\
                    << key_label << std::endl;
        return false;
    }

    //CK_OBJECT_HANDLE key_handle;

    //CK_OBJECT_CLASS object_class = CKO_SECRET_KEY;
    CK_BBOOL yes = CK_TRUE;
    CK_BBOOL no = CK_FALSE;
    //CK_KEY_TYPE key_type = CKK_AES;
    CK_ULONG key_size = 32;

    CK_BYTE_PTR label = (CK_BYTE_PTR)key_label.c_str(); 

    CK_ATTRIBUTE attributes[] = {
        {CKA_SENSITIVE,     &no,                sizeof(CK_BBOOL)},
        {CKA_EXTRACTABLE,   &yes,               sizeof(CK_BBOOL)},
        {CKA_TOKEN,         &yes,                sizeof(CK_BBOOL)},
        {CKA_LABEL,         label,              strlen((const char *)label)},
        {CKA_VALUE_LEN,     &key_size,          sizeof(CK_ULONG)}
    };

    CK_MECHANISM mech = {CKM_AES_KEY_GEN, NULL, 0};
    CK_RV rv = C_GenerateKey(*_session_handle, &mech, attributes, 
        sizeof(attributes)/sizeof(CK_ATTRIBUTE), key_handle);

    if(return_check(rv, "C_CreateObject (Session::createSessionKey)") != CKR_OK) 
        return false;
    else 
        return true;
}

bool Session::findKey(
    std::string key_label, 
    CK_OBJECT_HANDLE_PTR object_handle, 
    CK_ULONG_PTR object_count 
) {
    CK_BYTE_PTR label = (CK_BYTE_PTR)key_label.c_str();
    CK_ATTRIBUTE attributes[] = {
        {CKA_LABEL, label, strlen((const char *)label)}
    };
    CK_ULONG attributes_len = sizeof(attributes) / sizeof(CK_ATTRIBUTE);

    CK_RV rv = C_FindObjectsInit(*_session_handle, attributes, attributes_len);
    if(return_check(rv, "C_FindObjectsInit (Session::findKey)") != CKR_OK) 
        return false;
 
    rv = C_FindObjects(*_session_handle, object_handle, 1, object_count);
    if(return_check(rv, "C_FindObjects (Session::findKey)") != CKR_OK) 
        return false;
    
    rv = C_FindObjectsFinal(*_session_handle);
    if(return_check(rv, "C_FindObjectsFinal (Session::findKey)") != CKR_OK) 
        return false;

    return true;
}

bool Session::findKey(
    CK_ULONG id, 
    CK_OBJECT_HANDLE_PTR object_handle, 
    CK_ULONG_PTR object_count 
) {
    CK_BYTE id_to_search[1];
    *id_to_search = (CK_BYTE) id;

    CK_ATTRIBUTE attributes[] = {
        {CKA_ID, id_to_search, 1}
    };
    CK_ULONG attributes_len = sizeof(attributes) / sizeof(CK_ATTRIBUTE);

    CK_RV rv = C_FindObjectsInit(*_session_handle, attributes, attributes_len);
    if(return_check(rv, "C_FindObjectsInit (Session::findKey)") != CKR_OK) 
        return false;
 
    rv = C_FindObjects(*_session_handle, object_handle, 1, object_count);
    if(return_check(rv, "C_FindObjects (Session::findKey)") != CKR_OK) 
        return false;
    
    rv = C_FindObjectsFinal(*_session_handle);
    if(return_check(rv, "C_FindObjectsFinal (Session::findKey)") != CKR_OK) 
        return false;

    return true;
}

bool Session::getKeyValue(
    CK_OBJECT_HANDLE key_handle,
    CK_BYTE_PTR& key_raw,
    CK_ULONG_PTR key_len
) {
    CK_ATTRIBUTE attributes[] = {
        {CKA_VALUE,     NULL_PTR,   0},
    };

    CK_RV rv = C_GetAttributeValue(
        *_session_handle,
        key_handle,
        attributes,
        1
    );
    if(return_check(rv, "C_GetAttributeValue (Session::getKeyValue)") != CKR_OK) 
        return false;

    if (attributes[0].ulValueLen == CK_UNAVAILABLE_INFORMATION) {
        std::cerr << "Attribute not available" << std::endl;
        return false;
    }

    *key_len = attributes[0].ulValueLen;
    attributes[0].pValue = new CK_BYTE[*key_len];
    rv = C_GetAttributeValue(
        *_session_handle,
        key_handle,
        attributes,
        1
    );
    if(return_check(rv, "C_GetAttributeValue (Session::getKeyValue)") != CKR_OK) 
        return false;
    
    key_raw = new CK_BYTE[*key_len];
    memcpy(key_raw, attributes[0].pValue, *key_len);

    return true;
}

void Session::print_bytes_as_hex(unsigned char *bytes, size_t len) {
    for(size_t ctr=0; ctr<len; ctr++)
        std::cout << std::hex << (unsigned short)bytes[ctr];
}

bool Session::seedRandom(CK_BYTE_PTR seed, CK_ULONG seed_len) {
    CK_RV rv = C_SeedRandom(*_session_handle, seed, seed_len);
    if(return_check(rv, "C_SeedRandom (Session::seedRandom)") != CKR_OK)
        return false;
    return true;
}

bool Session::generateKeyPair(
    CK_OBJECT_HANDLE& public_key,
    std::string public_lable,
    CK_OBJECT_HANDLE& private_key,
    std::string private_lable,
    CK_ULONG key_length_bits
) {
    CK_RV rv;
    CK_MECHANISM mech = {CKM_RSA_PKCS_KEY_PAIR_GEN, NULL, 0};
    CK_BYTE public_exponent[] = {0x01, 0x00, 0x00, 0x01};

    CK_BBOOL yes = CK_TRUE;
    //CK_BBOOL no = CK_FALSE;

    CK_BYTE_PTR pub_lable = (CK_BYTE_PTR)public_lable.c_str(); 
    CK_BYTE_PTR prv_lable = (CK_BYTE_PTR)private_lable.c_str(); 

    CK_ATTRIBUTE public_key_attributes[] = {
        {CKA_LABEL,           pub_lable,        strlen((const char *)pub_lable)},
        {CKA_VERIFY,          &yes,             sizeof(CK_BBOOL)},
        {CKA_MODULUS_BITS,    &key_length_bits, sizeof(CK_ULONG)},
        {CKA_ENCRYPT,         &yes,             sizeof(CK_BBOOL)},
        {CKA_TOKEN,           &yes,             sizeof(CK_BBOOL)},
        {CKA_PUBLIC_EXPONENT, &public_exponent, sizeof(public_exponent)}
    };
    
    CK_ATTRIBUTE private_key_attributes[] = {
        {CKA_LABEL,         prv_lable,      strlen((const char *)prv_lable)},
        {CKA_SIGN,          &yes,           sizeof(CK_BBOOL)},
        {CKA_TOKEN,         &yes,           sizeof(CK_BBOOL)},
        {CKA_PRIVATE,       &yes,           sizeof(CK_BBOOL)},
        {CKA_SENSITIVE,     &yes,           sizeof(CK_BBOOL)}
    };

    rv = C_GenerateKeyPair(
        *_session_handle, 
        &mech,
        public_key_attributes, 
        sizeof(public_key_attributes)/sizeof(CK_ATTRIBUTE),
        private_key_attributes, 
        sizeof(private_key_attributes)/sizeof(CK_ATTRIBUTE),
        &public_key,
        &private_key
    );
    return_check(rv, "C_GenerateKeyPair (Session::generateKeyPair)");

    return true;
}

bool Session::destroyObject(CK_OBJECT_HANDLE object_handle) {
    CK_RV rv = C_DestroyObject(*_session_handle, object_handle);
    if(return_check(rv, "C_DestroyObject (Session::destroyObject)") != CKR_OK)
        return false;
    return true;
}

bool Session::createX509Certificate(
    CK_UTF8CHAR_PTR label, 
    CK_ULONG label_size,
    CK_BYTE_PTR owner,
    CK_ULONG owner_size,
    CK_BYTE_PTR certificate,
    CK_ULONG certificate_size
) {
    CK_RV rv;
    
    CK_OBJECT_CLASS class_type = CKO_CERTIFICATE;
    CK_CERTIFICATE_TYPE certType = CKC_X_509_ATTR_CERT;
    CK_BBOOL yes = CK_TRUE;
    CK_ATTRIBUTE attributes[] = {
        {CKA_CLASS,             &class_type,    sizeof(class_type)},
        {CKA_CERTIFICATE_TYPE,  &certType,      sizeof(certType)},
        {CKA_TOKEN,             &yes,           sizeof(yes)},
        {CKA_LABEL,             label,          label_size},
        {CKA_OWNER,             owner,          owner_size},
        {CKA_TRUSTED,           &yes,           sizeof(yes)},
        {CKA_VALUE,             certificate,    certificate_size}
    };
    CK_ULONG attributes_len = sizeof(attributes) / sizeof(CK_ATTRIBUTE);
    CK_OBJECT_HANDLE obj;

    rv = C_CreateObject(
        *_session_handle,
        attributes,
        attributes_len,
        &obj
    );
    if(
        return_check(rv, "C_CreateObject (Session::createX509Certificate)") 
        != CKR_OK
    )
        return false;

    return true;
}


bool Session::generateRandom(CK_BYTE_PTR random, CK_ULONG random_len) {
    CK_RV rv = C_GenerateRandom(*_session_handle, random, random_len);
    if(
        return_check(rv, "C_GenerateRandom (Session::Session::generateRandom)") 
        != CKR_OK
    )
        return false;
    return true;
}

struct Info {
    unsigned int    id;
    unsigned int    age;
};

bool Session::encryptAesGCM( 
    CK_OBJECT_HANDLE key,
    CK_BYTE_PTR data, 
    CK_ULONG data_len, 
    std::string aad_str,
    CK_BYTE_PTR& encrypted_data, 
    CK_ULONG_PTR encrypted_data_len
) {
    CK_RV rv;

    CK_MECHANISM mechanism;
    CK_GCM_PARAMS parameters;

    CK_BYTE_PTR iv = new CK_BYTE[AES_GCM_IV_SIZE];
    // NIST SP 800-38D recommends using a random iv
    // for the seed use the first byte of the message to be encrypted
    // not sure if it makes sense here (removed for now)
    // seedRandom((CK_BYTE_PTR)&data_len, sizeof(CK_ULONG));
    for(size_t i = 0; i < AES_GCM_IV_SIZE; ++i)
        generateRandom(iv + i, sizeof(CK_BYTE));
    
    CK_BYTE aad[AAD_NIST_MAX_SIZE];
    memset(aad, 0, AAD_NIST_MAX_SIZE);
    strncpy((char *)aad, aad_str.c_str(), aad_str.length());

    parameters.pIv = iv;
    parameters.ulIvLen = AES_GCM_IV_SIZE;
    //parameters.ulIvBits = 0;  // pkcs#11 is unclear about this flag use
    parameters.pAAD = aad;
    parameters.ulAADLen = AAD_NIST_MAX_SIZE;
    parameters.ulTagBits = AES_GCM_TAG_SIZE * 8;

    mechanism.mechanism = CKM_AES_GCM;
    mechanism.ulParameterLen = sizeof(parameters);
    mechanism.pParameter = &parameters;

    rv = C_EncryptInit(*_session_handle, &mechanism, key);
    return_check(rv, "C_EncryptInit (Session::encryptAesGCM)");

    rv = C_Encrypt(
        *_session_handle, 
        data, 
        data_len, 
        NULL_PTR, 
        encrypted_data_len
    );
    *encrypted_data_len += AES_GCM_IV_SIZE;
    return_check(rv, "C_Encrypt (Session::encryptAesGCM)");
    if(rv != CKR_OK) return false;
    
    encrypted_data = (CK_BYTE_PTR)malloc(*encrypted_data_len);
    memset(encrypted_data, 0, *encrypted_data_len);

    rv = C_Encrypt(
        *_session_handle, 
        data, 
        data_len, 
        encrypted_data + AES_GCM_IV_SIZE, 
        encrypted_data_len
    );

    memcpy(encrypted_data, iv, AES_GCM_IV_SIZE);
    *encrypted_data_len += AES_GCM_IV_SIZE;
    return_check(rv, "C_Encrypt (Session::encryptAesGCM)");
    if(rv != CKR_OK) return false;

    delete[] iv;
    return true;
}

bool Session::encryptAesGCMParam( 
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
) {
    CK_RV rv;

    CK_MECHANISM mechanism;
    CK_GCM_PARAMS parameters;

    *iv_size = AES_GCM_IV_SIZE;
    iv = new CK_BYTE[*iv_size];
    for(size_t i = 0; i < AES_GCM_IV_SIZE; ++i)
        generateRandom(iv + i, sizeof(CK_BYTE));
    
    aad = new CK_BYTE[AAD_NIST_MAX_SIZE];
    memset(aad, 0, AAD_NIST_MAX_SIZE);
    *aad_size = AAD_NIST_MAX_SIZE;

    parameters.pIv = iv;
    parameters.ulIvLen = AES_GCM_IV_SIZE;
    parameters.ulIvBits = 96;  // pkcs#11 is unclear about this flag use
    parameters.pAAD = aad;
    parameters.ulAADLen = AAD_NIST_MAX_SIZE;
    parameters.ulTagBits = 128;//AES_GCM_TAG_SIZE * 8;

    mechanism.mechanism = CKM_AES_GCM;
    mechanism.ulParameterLen = sizeof(parameters);
    mechanism.pParameter = &parameters;

    rv = C_EncryptInit(*_session_handle, &mechanism, key);
    return_check(rv, "C_EncryptInit (Session::encryptAesGCM)");

    rv = C_Encrypt(
        *_session_handle, 
        data, 
        data_len, 
        NULL_PTR, 
        encrypted_data_len
    );
    return_check(rv, "C_Encrypt (Session::encryptAesGCM)");
    if(rv != CKR_OK) return false;
    
    encrypted_data = (CK_BYTE_PTR)malloc(*encrypted_data_len);
    memset(encrypted_data, 0, *encrypted_data_len);

    rv = C_Encrypt(
        *_session_handle, 
        data, 
        data_len, 
        encrypted_data, 
        encrypted_data_len
    );
    return_check(rv, "C_Encrypt (Session::encryptAesGCM)");
    if(rv != CKR_OK) return false;

    *tag_size = AES_GCM_TAG_SIZE;
    tag = new CK_BYTE[*tag_size];
    memcpy(tag, encrypted_data + (*encrypted_data_len - AES_GCM_TAG_SIZE), AES_GCM_TAG_SIZE);
    *encrypted_data_len -= AES_GCM_TAG_SIZE;

    return true;
}

bool Session::decryptAesGCM(
    CK_OBJECT_HANDLE key,
    CK_BYTE_PTR data, 
    CK_ULONG data_len,
    std::string aad_str,
    CK_BYTE_PTR& decrypted_data, 
    CK_ULONG_PTR decrypted_data_len
) {
    CK_RV rv;

    CK_MECHANISM mechanism;
    CK_GCM_PARAMS parameters;

    CK_BYTE_PTR iv = new CK_BYTE[AES_GCM_IV_SIZE];
    // TODO: NIST SP 800-38D recommends using a random iv
    for(size_t i = 0; i < AES_GCM_IV_SIZE; ++i)
        iv[i] = data[i];

    CK_BYTE aad[AAD_NIST_MAX_SIZE];
    memset(aad, 0, AAD_NIST_MAX_SIZE);
    memcpy(aad, (void *)aad_str.c_str(), aad_str.length());

    parameters.pIv = iv;  
    parameters.ulIvLen = AES_GCM_IV_SIZE;
    parameters.pAAD = aad;
    //parameters.ulAADLen = AAD_NIST_MAX_SIZE;
    parameters.ulAADLen = 8;
    parameters.ulTagBits = AES_GCM_TAG_SIZE * 8;

    mechanism.mechanism = CKM_AES_GCM;
    mechanism.ulParameterLen = sizeof(parameters);
    mechanism.pParameter = &parameters;

    rv = C_DecryptInit(*_session_handle, &mechanism, key);
    if(return_check(rv, "C_DecryptInit (Session::decryptAesGCM)") != CKR_OK)
        return false;

    rv = C_Decrypt(
        *_session_handle, 
        data + AES_GCM_IV_SIZE, 
        data_len - AES_GCM_IV_SIZE,
        NULL_PTR, 
        decrypted_data_len
    );
    if(return_check(rv, "C_Decrypt (Session::decryptAesGCM)") != CKR_OK)
        return false;

    // +1 to null terminate the raw bytes
    decrypted_data = (CK_BYTE_PTR) malloc(*decrypted_data_len);  

    rv = C_Decrypt(
        *_session_handle, 
        data + AES_GCM_IV_SIZE, 
        data_len - AES_GCM_IV_SIZE,
        decrypted_data, 
        decrypted_data_len
    );
    if(return_check(rv, "C_Decrypt (Session::decryptAesGCM)") != CKR_OK)
        return false;

    decrypted_data[*decrypted_data_len] = 0;

    delete[] iv;
    return true;
}

bool Session::encryptRsa(
    CK_OBJECT_HANDLE key,
    CK_BYTE_PTR data, 
    CK_ULONG data_len, 
    CK_BYTE_PTR& encrypted_data, 
    CK_ULONG_PTR encrypted_data_len
) {
    CK_RV rv;
    // CK_MECHANISM mechanism = {CKM_RSA_X_509};
    CK_MECHANISM mechanism = {CKM_RSA_PKCS};

    rv = C_EncryptInit(*_session_handle, &mechanism, key);
    if(return_check(rv, "C_EncryptInit (Session::encryptRsa)") != CKR_OK)
        return false;

    CK_BYTE_PTR padded_data = (CK_BYTE_PTR)malloc(256 * sizeof(CK_BYTE));
    CK_ULONG padded_data_len = 256;

    memset(padded_data, 0, padded_data_len); 
    memcpy(padded_data, data, data_len);

    rv = C_Encrypt(
        *_session_handle,
        padded_data,
        padded_data_len,
        encrypted_data,
        encrypted_data_len
    );

    if(return_check(rv, "C_Encrypt (Session::encryptRsa)") != CKR_OK)
        return false;

    encrypted_data = (CK_BYTE_PTR)malloc(*encrypted_data_len);
    memset(encrypted_data, 0, *encrypted_data_len);

    rv = C_Encrypt(
        *_session_handle,
        data,
        data_len,
        encrypted_data,
        encrypted_data_len
    );
    if(return_check(rv, "C_Encrypt (Session::encryptRsa)") != CKR_OK)
        return false;
    encrypted_data[*encrypted_data_len] = 0;

    return true;
}

bool Session::decryptRsa(
    CK_OBJECT_HANDLE key,
    CK_BYTE_PTR data, 
    CK_ULONG data_len, 
    CK_BYTE_PTR& decrypted_data, 
    CK_ULONG_PTR decrypted_data_len
) {
    CK_RV rv;
    //CK_MECHANISM mechanism = {CKM_RSA_X_509};
    CK_MECHANISM mechanism = {CKM_RSA_PKCS};

    rv = C_DecryptInit(*_session_handle, &mechanism, key);
    if(return_check(rv, "C_DecryptInit (Session::decryptRsa)") != CKR_OK)
        return false;

    rv = C_Decrypt(
        *_session_handle, 
        data, 
        data_len,
        NULL_PTR, 
        decrypted_data_len
    );
    if(return_check(rv, "C_Decrypt (Session::decryptAesGCM)") != CKR_OK)
        return false;

    // std::cout << "decrypted_data_len: " << *decrypted_data_len << std::endl;

    decrypted_data = (CK_BYTE_PTR)malloc(*decrypted_data_len);
    memset(decrypted_data, 0, *decrypted_data_len);
    
    rv = C_Decrypt(
        *_session_handle,
        data,
        data_len,
        decrypted_data,
        decrypted_data_len
    );
    if(return_check(rv, "C_Decrypt (Session::decryptAesGCM)") != CKR_OK)
        return false;

    decrypted_data[*decrypted_data_len] = 0;

    return true;
}

bool Session::sign(
    CK_OBJECT_HANDLE key,
    CK_BYTE_PTR data, 
    CK_ULONG data_len,
    CK_BYTE_PTR& signature, 
    CK_ULONG_PTR signature_len
) {
    CK_RV rv;
    CK_MECHANISM mechanism = {CKM_SHA512_RSA_PKCS};

    rv = C_SignInit(*_session_handle, &mechanism, key);
    if(return_check(rv, "C_SignInit (Session::sign)") != CKR_OK)
        return false;
    
    rv = C_Sign(
        *_session_handle,
        data,
        data_len,
        NULL_PTR,
        signature_len
    );
    if(return_check(rv, "C_Sign (Session::sign)") != CKR_OK)
        return false;
    signature = (CK_BYTE_PTR)malloc(*signature_len);
    memset(signature, 0, *signature_len);
    //signature = (CK_BYTE_PTR)malloc(512*sizeof(CK_BYTE));
    //memset(signature, 0, 512);

    rv = C_Sign(
        *_session_handle,
        data,
        data_len,
        signature,
        signature_len
    );
    if(return_check(rv, "C_Sign (Session::sign)") != CKR_OK)
        return false;
    //signature[*signature_len] = 0;

    return true;
}

bool Session::verifyRsa(
    CK_OBJECT_HANDLE key,
    CK_BYTE_PTR data, 
    CK_ULONG data_len,
    CK_BYTE_PTR signature, 
    CK_ULONG signature_len
) {
    CK_RV rv;
    CK_MECHANISM mechanism = {CKM_SHA512_RSA_PKCS};

    rv = C_VerifyInit(*_session_handle, &mechanism, key);
    if(return_check(rv, "C_VerifyInit (Session::verifyRsa)") != CKR_OK) 
        return false;

    rv = C_Verify(*_session_handle, data, data_len, signature, signature_len);
    if(return_check(rv, "C_Verify (Session::verifyRsa)") != CKR_OK)
        return false;

    return true;
}

bool Session::wrapSessionKey(
    CK_OBJECT_HANDLE wrapping_key,
    CK_OBJECT_HANDLE key_to_wrap,
    CK_BYTE_PTR& wrapped_key,
    CK_ULONG_PTR wrapped_key_len
) {
    CK_RV rv;

    CK_MECHANISM mechanism = {CKM_RSA_PKCS, NULL_PTR, 0};

    rv = C_WrapKey(
        *_session_handle,
        &mechanism,
        wrapping_key,
        key_to_wrap,
        NULL,
        wrapped_key_len
    );
    if(return_check(rv, "C_WrapKey (Session::wrapSessionKey)") != CKR_OK) 
        return false;

    wrapped_key = new CK_BYTE[*wrapped_key_len];
    
    rv = C_WrapKey(
        *_session_handle,
        &mechanism,
        wrapping_key,
        key_to_wrap,
        wrapped_key,
        wrapped_key_len
    );
    if(return_check(rv, "C_WrapKey (Session::wrapSessionKey)") != CKR_OK) 
        return false;
    

    return true;
}

}

