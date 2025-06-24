#ifndef __PROGETTO_GATEWAY_H_
#define __PROGETTO_GATEWAY_H_

#include <omnetpp.h>
#include <fstream>
#include <map>
#include "common/common.h"
#include "Communication_m.h"   // contiene Request, KeyRequest, KeyResponse
#include "Request_m.h"
#include "Packet_m.h"
using namespace omnetpp;

class Gateway : public cSimpleModule
{
private:
    int numECUs;
    std::ofstream logFile;

    Packet *sync_clock_event;
    rapidjson::Document doc;
    rapidjson::StringBuffer buffer;


protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
};

#endif
