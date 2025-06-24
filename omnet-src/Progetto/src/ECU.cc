#include "ECU.h"
#include "Communication_m.h"
#include "Packet_m.h"

#include <string>
#include <cstdio>
#include <cstring>
#include <openssl/rand.h>
#include <ctime>

#include "def.h"
#include "rsa/rsa_exchange.h"
#include "ns/ns_exchange.h"
#include "aes/aes.h"
#include "crypto_utils/crypto_utils.h"
#include "TPM.h"

Define_Module(ECU);
void ECU::initialize()
{
    id = par("id");
    numECUs = par("numECUs");

    tpm_access = new TPM(id);
    timestamp_b64 = new std::string[numECUs];

    HSMCommunicationInit = new Packet("HSM");
    HSMCommunicationInit->setType(ECU_INIT_RSA_SIGNAL);
    scheduleAt(0, HSMCommunicationInit);

    additional_initialize();
}

void ECU::handleMessage(cMessage *msg)
{
    Packet *pkg = (Packet *) msg;

    int type = pkg->getType();
    switch(type) {
        case ECU_INIT_RSA_SIGNAL: {
            sendHsmRsaRequest();
            }break;
        case RSA_RESPONSE: {
            setHsmSessionKey(pkg);
            //if(id == 1) {
                //sendEcuSessionRequest(4);
                for(int i = 1; i <= numECUs; ++i) {
                    if(i == id) continue;
                    sendEcuSessionRequest(i);
                }
            //}
            }break;
        case NS_RESPONSE_SENDER: {
            handleEcuSessionKey(pkg);
            }break;
        case NS_RESPONSE_RECEIVER: {
            handleEcuTicket(pkg);
            }break;
        case CLOCK_SYNC_RESPONSE: {
            handleClockSync(pkg);
            delete pkg;
            }break;
        default: {
            additional_handleMessage(msg);
            }break;
    }
}

void ECU::additional_handleMessage(cMessage *msg)
{
    Packet *pkg = (Packet *) msg;

    int type = pkg->getType();
    switch(type) {
        default:
            break;
    }
}

void ECU::sendHsmRsaRequest() {
    Packet *req = new Packet("hsm_rsa_req");

    req->setType(RSA_REQUEST);
    req->setDstId(HSM_TOPOLOGICAL_ID);
    req->setSrcId(id);
    std::string json_formatted_request = serialize_rsa_request(id);
    const char *data = json_formatted_request.c_str();
    req->setData(data);

    send(req, "out");
}

bool ECU::setHsmSessionKey(Packet *res) {
    std::string json_response(res->getData());

    unsigned char aes_key_enc[AES_KEY_ENC_MAXLEN];
    //std::cout << "json_response: " << json_response << std::endl;
    parse_rsa_response(json_response, aes_key_enc);

    size_t aes_key_len = AES_KEY_LEN;
    int ret = rsa_decrypt_evp(tpm_access->getPrivateKey(), aes_key_enc, AES_KEY_ENC_MAXLEN, tpm_access->getSessionKeyHandle(0), &aes_key_len);
    //std::cout << "rsa_decrypt_evp: " << ret << std::endl;
    //std::cout << "aes_key_len: " << aes_key_len << std::endl;

    if (aes_key_len != AES_KEY_LEN) {
        EV << "[Error] Chiave ricevuta con lunghezza errata" << std::endl;
        EV << "with key len: " << aes_key_len << std::endl;
    }

    return true;
}

void ECU::sendEcuSessionRequest(int dst) {
    std::string timestamp = std::to_string(hw_clock.time_since_epoch());
    timestamp_b64[dst-1] = base64_encode((const unsigned char*)timestamp.c_str(), timestamp.length());

    std::string ns_request = serialize_ns_session_request(
            id,
            dst,
            timestamp_b64[dst-1]
    );

    Packet *req = new Packet("hsm_ecu_key_req");

    req->setType(NS_REQUEST);
    req->setSrcId(id);
    req->setDstId(HSM_TOPOLOGICAL_ID);
    //const char *data = json_request.c_str();
    const char *data = ns_request.c_str();
    req->setData(data);

    // std::cout << "json_request: " << json_request << std::endl;

    send(req, "out");
}

bool ECU::handleEcuSessionKey(Packet *pkg) {
    const std::string& receivedData = pkg->getData();

    std::string nonce_b64;
    std::string ns_session_key_b64;
    std::string ticket_b64;
    int receiver_id;

    parse_ns_response_aes(
            receivedData,
            nonce_b64,
            ns_session_key_b64,
            ticket_b64,
            receiver_id,
            tpm_access->getSessionKeyHandle(0)
    );

    if(timestamp_b64[receiver_id-1] != nonce_b64)
        return false;
    base64_decode(ns_session_key_b64, tpm_access->getSessionKeyHandle(receiver_id), AES_KEY_LEN);

    unsigned char* ticket_msg_arr[4096];
    size_t ticket_len = base64_decode(ticket_b64, (unsigned char*)ticket_msg_arr, sizeof(ticket_msg_arr));
    std::string ticket((const char*)ticket_msg_arr, ticket_len);

    Packet *msg = new Packet("ecu_ticket");
    msg->setType(NS_RESPONSE_RECEIVER);
    msg->setSrcId(id);
    msg->setDstId(receiver_id);
    msg->setData(ticket.c_str());

    send(msg, "out");

    return true;
}

bool ECU::handleEcuTicket(Packet *pkg)
{
    const std::string receivedData = pkg->getData();
    int sender_id;
    std::string ns_session_key_b64;

    ns_receive_ticket(receivedData, sender_id, ns_session_key_b64, tpm_access->getSessionKeyHandle(0));

    base64_decode(ns_session_key_b64, (unsigned char*)tpm_access->getSessionKeyHandle(sender_id), AES_KEY_LEN);

    return true;
}

void ECU::handleClockSync(Packet *pkg)
{
    std::string json_message = pkg->getData();

    rapidjson::Document message_doc;
    if (message_doc.Parse(json_message.c_str()).HasParseError())
        std::cerr << "Error while parsing json (ECU::handleClockSync)" << std::endl;

    if (!message_doc.HasMember("timestamp") || !message_doc["timestamp"].IsInt())
        std::cerr << "Sync Message lacks time member (ECU::handleClockSync)" << std::endl;

    std::time_t trusted_timestamp = message_doc["timestamp"].GetInt();
    hw_clock.update_drift_correction(trusted_timestamp);
}

void ECU::finish()
{
    delete tpm_access;
    cancelAndDelete(HSMCommunicationInit);
}
