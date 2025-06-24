#include "HSM.h"
#include "def.h"
#include "Packet_m.h"

#include <chrono>

#include "include/rapidjson/document.h"
#include "include/rapidjson/stringbuffer.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/error/en.h"

#include "include/rsa_handling/rsa.h"
#include "include/ns_handling/ns.h"
#include "include/sca.h"

#define PORT 9000
#define LOCALHOST "127.0.0.1"

Define_Module(HSM);

void HSM::initialize()
{

}

void HSM::handleMessage(cMessage *msg)
{
    Packet *pkg = (Packet *) msg;
    switch(pkg->getType()) {
    case RSA_REQUEST: {
        sendRequestToHSM(pkg, HSM_RSA_RESPONSE, "hsm_rsa_res");
        }break;
    case NS_REQUEST: {
        sendRequestToHSM(pkg, HSM_NS_RESPONSE_SENDER, "hsm_ecu_key_res");
        }break;
    case CLOCK_SYNC_REQUEST: {
        sendRequestToHSM(pkg, CLOCK_SYNC_RESPONSE, "SYNC_CLOCK_RESPONSE");
        }break;
    default: {
        }break;
    }
}

void HSM::sendRequestToHSM(Packet *pkg, int response_type, const char* ret_pkg_name) {
    session.beginReadWrite(slot->getID());
    session.loginUser("12345");

    std::string json_request = pkg->getData();

    rapidjson::Document doc;
    if (doc.Parse(json_request.c_str()).HasParseError())
        std::cerr << "Errore parsing JSON" << std::endl;
    if (!doc.HasMember("type") || !doc["type"].IsInt())
        std::cerr << "Errore parsing JSON" << std::endl;

    std::string json_response;
    switch(doc["type"].GetInt()) {
        case HSM_RSA_REQUEST: {
            json_response = rsa_response(doc, session);
            } break;
        case HSM_NS_REQUEST: {
            json_response = ns_response(doc, session);
            } break;
        case CLOCK_SYNC_REQUEST: {
            std::time(0);
            }break;
        default: {
            } break;
    }

    Packet *res = new Packet(ret_pkg_name);

    res->setType(response_type);
    res->setDstId(pkg->getSrcId());
    res->setSrcId(HSM_TOPOLOGICAL_ID);
    res->setData(json_response.c_str());

    send(res, "toGateway");
    session.logout();
    session.end();
}

void HSM::finish() {

}
