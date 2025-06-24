#include "Gateway.h"
#include "Packet_m.h"
#include "def.h"

#include "common/common.h"

#define CLOCK_INTERVAL 5

Define_Module(Gateway);

void Gateway::initialize()
{
    numECUs = par("numECUs");

    doc.SetObject();
    auto& alloc = doc.GetAllocator();

    doc.AddMember("type", CLOCK_SYNC_RESPONSE, alloc);
    doc.AddMember("timestamp", std::time(0), alloc);

    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    buffer.GetString();

    sync_clock_event = new Packet("SYNC_CLOCK_REQUEST");
    sync_clock_event->setType(CLOCK_SYNC_REQUEST);
    sync_clock_event->setData(buffer.GetString());
    scheduleAt(simTime(), sync_clock_event);
}

void Gateway::handleMessage(cMessage *msg)
{
    Packet *pkg = (Packet *) msg;
    int type = pkg->getType();
    switch(type) {
        case RSA_REQUEST: {
            send(msg, "toHsm");
            }break;
        case RSA_RESPONSE: {
            send(msg, "ecuOut", pkg->getDstId()-1);
            }break;
        case NS_REQUEST: {
            send(msg, "toHsm");
            }break;
        case NS_RESPONSE_SENDER: {
            send(msg, "ecuOut", pkg->getDstId()-1);
            }break;
        case NS_RESPONSE_RECEIVER: {
            send(msg, "ecuOut", pkg->getDstId()-1);
            }break;
        case CLOCK_SYNC_REQUEST: {
            doc["timestamp"].SetInt(std::time(0));
            sync_clock_event->setData(buffer.GetString());
            sync_clock_event->setName("SYNC_CLOCK_RESPONSE");
            for(size_t i = 0; i < numECUs; ++i)
                send(sync_clock_event->dup(), "ecuOut", i);
            sync_clock_event->setName("SYNC_CLOCK_REQUEST");
            scheduleAt(simTime() + CLOCK_INTERVAL, sync_clock_event);
            }break;
        default: {
            if(pkg->getDstId() != 0) {
                send(msg, "ecuOut", pkg->getDstId()-1);
            }else {
                send(msg, "toHsm");
            }
            }break;
    }
}

void Gateway::finish()
{

    // Chiudo il file di log
    if (logFile.is_open()) {
        logFile.close();
        EV << "[Gateway] Chiuso events.log\n";
    }
    cancelAndDelete(sync_clock_event);
}
