#ifndef __PROGETTO_INFOTAINMENT_H_
#define __PROGETTO_INFOTAINMENT_H_

#include "ECU.h"

#include <omnetpp.h>
#include "Packet_m.h"
#include "common/common.h"
#include <unordered_map>


using namespace omnetpp;

class Infotainment : public ECU
{
private:
    std::unordered_map<std::string, std::string> rules;

protected:
    bool loadFilterRules();
    bool webAccess(std::string uri);

    void additional_initialize() override;
    void additional_handleMessage(cMessage *msg) override;
};

#endif
