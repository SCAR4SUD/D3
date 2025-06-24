#include "Infotainment.h"
#include "Communication_m.h"
#include "Packet_m.h"

#include <string>
#include <cstdio>
#include <cstring>
#include <openssl/rand.h>
#include <ctime>
#include <curl/curl.h>
#include <fstream>

#include "def.h"
#include "rsa/rsa_exchange.h"
#include "ns/ns_exchange.h"
#include "aes/aes.h"
#include "crypto_utils/crypto_utils.h"

// #include <sys/socket.h>
// #include <arpa/inet.h>
// #include <unistd.h>

#define PORT 9001
#define LOCALHOST "127.0.0.1"

Define_Module(Infotainment);

void Infotainment::additional_initialize()
{
    loadFilterRules();
}

bool Infotainment::loadFilterRules()
{
    std::string path = "../storage/ecu" + std::to_string(id) + "/whitelist.rules";
    std::ifstream whitelist_file(path);
    if (!whitelist_file.is_open()) {
        std::cerr << "[ERROR] Infotainment: failed to retrieve whitelist file" << std::endl;
        return false;
    }

    rapidjson::Document doc;

    int line_count = 0;
    std::string line;
    while(getline(whitelist_file, line)) {
        if (
            doc.Parse(line.c_str()).HasParseError() ||
            !doc.HasMember("uri")                       || !doc["uri"].IsString()       ||
            !doc.HasMember("action")                    || !doc["action"].IsString()

        ) {
            std::cerr << "[ERROR] Infotainment: rule at line " << ++line_count << " has not been parsed corretly" << std::endl;
            continue;
        }

        rules[doc["uri"].GetString()] = doc["action"].GetString();
    }


    whitelist_file.close();
    return true;
}

void Infotainment::additional_handleMessage(cMessage *msg)
{
    Packet *pkg = (Packet *) msg;

    int type = pkg->getType();
    switch(type) {
        default:
            break;
    }
}


bool Infotainment::webAccess(std::string uri)
{
    if(rules[uri] == "ACCEPT")
        EV << "Infotainment: requested resouces at " << uri << " can be requested" << std::endl;
    else if(rules[uri] == "ALERT")
        EV << "Infotainment: requested resouces at " << uri << " can be requested but alert is being sent" << std::endl;
    else
        EV << "Infotainment: requested resouces at " << uri << " cannot requested. request has been dropped" << std::endl;

    return true;
}
