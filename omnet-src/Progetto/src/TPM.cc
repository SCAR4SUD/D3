#include "TPM.h"

#include "crypto_utils/crypto_utils.h"
#include <string>
#include <cstring>


TPM::TPM(int id) {
    std::string path = "../storage/ecu" + std::to_string(id) + "/ecu" + std::to_string(id) +"_private.pem";
    init_keys(path.c_str(), ecu_priv_key);

    if(ecu_priv_key == nullptr)
        std::cout << "[Error] private key not retrieved" << std::endl;
}

EVP_PKEY* TPM::getPrivateKey()
{
    return ecu_priv_key;
}

unsigned char* TPM::getSessionKeyHandle(unsigned int key_id)
{
    if(key_id >= MAX_ECU_NUM)
        return nullptr;
    if(key_id == 0)
        return aes_hsm_key;
    return aes_ecu_session_keys[key_id];
}

TPM::~TPM() {
}

