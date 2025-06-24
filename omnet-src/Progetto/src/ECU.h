#ifndef __PROGETTO_ECU_H_
#define __PROGETTO_ECU_H_

#include <omnetpp.h>
#include <chrono>
#include "Packet_m.h"
#include "common/common.h"

#include "TPM.h"
#include "clock/Clock.h"

using namespace omnetpp;

class ECU : public cSimpleModule
{
protected:
    int id;                         // ECU id
    int numECUs;                    // numero totale di ECU
    Packet *HSMCommunicationInit;   // timer per inviare il prossimo messaggio

    TPM *tpm_access = nullptr;
    Clock hw_clock;

    void sendHsmRsaRequest();
    bool setHsmSessionKey(Packet *pkg);

    bool handleEcuSessionKey(Packet *pkg);
    void sendEcuSessionRequest(int dst);
    bool handleEcuTicket(Packet *pkg);

    void sendEcuAuthenticartion();


    void handleClockSync(Packet *pkg);

    void sendEncrypted(Packet *pkg, const unsigned char *key);

    std::string *timestamp_b64;

    virtual void initialize() override;
    virtual void additional_initialize() {};

    virtual void handleMessage(cMessage *msg) override;
    virtual void additional_handleMessage(cMessage *msg);
    virtual void finish() override;
};

#endif
