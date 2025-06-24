#ifndef SCA_RSA_H
#define SCA_RSA_H

#include "../rapidjson/document.h"

#include "../sca.h"

std::string rsa_response(rapidjson::Document& doc, sca::Session& session);

#endif
