#ifndef HSM_H
#define HSM_H
#include "../cryptoki.h"
#include "../slot/slot.h"

namespace sca {

class HSM {
private:
    static HSM *_instance;

    CK_ULONG slot_count{0};
    Slot *slots{NULL_PTR};

    void initialize();
    void getSlotList();
    
    HSM();
    ~HSM();

public:
    static HSM *get();
    CK_INFO getHSMInfo();
    Slot *getSlot(unsigned short cardinal);

};

}

#endif
