#ifndef SLOT_H
#define SLOT_H
#include "../cryptoki.h"

namespace sca {

class Slot {
private:
    CK_SLOT_ID id{};
    CK_SLOT_INFO info{};

    CK_TOKEN_INFO token_info{};
    
    CK_ULONG mechanism_count{0};
    CK_MECHANISM_TYPE_PTR mechanism_list{NULL_PTR};
    CK_MECHANISM_INFO getMechanismInfo(CK_MECHANISM_TYPE type);

public:
    Slot();
    ~Slot();
    void define(CK_SLOT_ID id_number);
    bool canMechanismDoMethod(CK_MECHANISM_TYPE type, CK_FLAGS);
    bool closeAllSessions();
    CK_SLOT_ID getID();
    
};

}

#endif
