#ifndef RETURN_CHECK_H
#define RETURN_CHECK_H
#include "../cryptoki.h"

namespace sca {
    CK_RV return_check(CK_RV rv, const char* function_name);
}

#endif

