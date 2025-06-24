#ifndef __PROGETTO_STORAGE_H_
#define __PROGETTO_STORAGE_H_

#include <omnetpp.h>
#include <fstream>
#include <map>
#include <string>
#include "Communication_m.h"

using namespace omnetpp;

class Storage : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

  private:
    std::map<int, std::ofstream> fileStreams;
    int numEcu;

};

#endif
