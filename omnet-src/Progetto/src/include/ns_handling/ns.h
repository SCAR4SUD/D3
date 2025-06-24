#ifndef NS_H
#define NS_H

#include <string>
#include "../rapidjson/document.h"

#include "../sca.h"

std::string ns_response(rapidjson::Document& doc, sca::Session& session);

#endif
