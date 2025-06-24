#include "slot.h"

#include "../return_check/return_check.h"
#include "../cryptoki.h"

namespace sca {

Slot::Slot() {}

Slot::~Slot() {
    delete mechanism_list;
}

void Slot::define(CK_SLOT_ID id_number) {
    id = id_number;
    return_check(C_GetSlotInfo(id, &info),\
        "C_GetSlotInfo (Slot::define)");
    return_check(C_GetTokenInfo(id, &token_info),\
        "C_GetTokenInfo (Slot::define)");
    return_check(C_GetMechanismList(id, NULL_PTR, &mechanism_count),\
        "C_GetMechanismList (Slot::define)");
    mechanism_list = new CK_MECHANISM_TYPE[mechanism_count];
    return_check(C_GetMechanismList(id, mechanism_list, &mechanism_count),\
        "C_GetMechanismList (Slot::define)");
}

CK_MECHANISM_INFO Slot::getMechanismInfo(CK_MECHANISM_TYPE type) {
    CK_MECHANISM_INFO info;
    return_check(C_GetMechanismInfo(id, type, &info),\
        "C_GetMechanismInfo (Slot::getMechanismInfo)");
    return info;
}

bool Slot::canMechanismDoMethod(CK_MECHANISM_TYPE type, CK_FLAGS method) {
    CK_MECHANISM_INFO info = getMechanismInfo(type);
    if(info.flags & method) return true;
    else return false;
}

bool Slot::closeAllSessions() {
    if(
        return_check(
            C_CloseAllSessions(id), 
            "C_CloseAllSessions (Slot::closeAllSessions)"
        ) != CKR_OK
    ) 
        return false;
    else 
        return true;
}

CK_SLOT_ID Slot::getID() {
    return id;
}

}
