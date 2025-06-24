#include "../return_check/return_check.h"
#include "hsm.h"

#include "../return_check/return_check.h"

namespace sca {

HSM *HSM::_instance = nullptr;

void HSM::getSlotList() {
    return_check(C_GetSlotList(CK_TRUE, NULL_PTR, &slot_count),\
        "C_GetSlotList (HSM::getSlotList())");
    CK_SLOT_ID_PTR slot_list = new CK_SLOT_ID[(unsigned)slot_count];
    return_check(C_GetSlotList(CK_TRUE, slot_list, &slot_count),\
        "C_GetSlotList (HSM::getSlotList())");
    slots = new Slot[(unsigned int)slot_count];
    for(unsigned i = 0; i < (unsigned)slot_count; ++i) {
        slots[i].define(slot_list[i]);
    }
}

void HSM::initialize() {
    // structure containing the optional arguments for the C_Initialize
    // 
    // by having the flag set to CKF_OS_LOCKING_OK and by not supplying
    //      
    CK_C_INITIALIZE_ARGS args = {
        NULL_PTR,         // pointer to a function for creating mutex objects
        NULL_PTR,         // pointer to a function for destroying mutex objects
        NULL_PTR,         // pointer to a function for locking mutex objects
        NULL_PTR,         // pointer to a function for unlocking mutex objects
        CKF_OS_LOCKING_OK,// flags for how to handle multi-threaded access
        NULL_PTR          // reserved for future use
    };
    return_check(C_Initialize(&args), "C_Initialize (Session::Session)");
}

HSM::HSM() {
    initialize();
    // fill a list with available slots id
    getSlotList();
}

HSM::~HSM() {
    delete slots;
    return_check(C_Finalize(NULL_PTR), "C_Finalize (Session::~Session)");
}

HSM *HSM::get() {
    if(_instance == nullptr) 
        _instance = new HSM();
    return _instance;
}

 
CK_INFO HSM::getHSMInfo() {
    CK_INFO info;
    return_check(C_GetInfo(&info), "C_GetInfo (Session::getInfo)");
    return info;
}

Slot* HSM::getSlot(unsigned short cardinal) {
    if (cardinal < slot_count)
        return &slots[cardinal];
    else
        return nullptr;
}

}
