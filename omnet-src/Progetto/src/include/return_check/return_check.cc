#include "../../include/return_check/return_check.h"

#include <iostream>

#include "../cryptoki.h"

namespace sca {
    CK_RV return_check(CK_RV rv, const char* function_name) {
        if(rv == CKR_OK) 
            return rv;
        switch(rv) {
        // Universal Cryptoki function return values
            case CKR_GENERAL_ERROR: 
                { std::cerr << function_name << ": CKR_GENERAL_ERROR\n"; }; 
                break;
            case CKR_HOST_MEMORY: 
                { std::cerr << function_name << ": CKR_HOST_MEMORY\n"; }; 
                break;
            case CKR_FUNCTION_FAILED: 
                { std::cerr << function_name << ": CKR_FUNCTION_FAILED\n"; }; 
                break;
        // Cryptoki function return values for functions that use a session handle
            case CKR_SESSION_HANDLE_INVALID: 
                { std::cerr << function_name << ": CKR_SESSION_HANDLE_INVALID\n"; }; 
                break;
            case CKR_DEVICE_REMOVED: 
                { std::cerr << function_name << ": CKR_DEVICE_REMOVED\n"; }; 
                break;
            case CKR_SESSION_CLOSED: 
                { std::cerr << function_name << ": CKR_SESSION_CLOSED\n"; }; 
                break;
        // Cryptoki function return values for functions that use a token
            case CKR_DEVICE_MEMORY: 
                { std::cerr << function_name << ": CKR_DEVICE_MEMORY\n"; }; 
                break;
            case CKR_DEVICE_ERROR: 
                { std::cerr << function_name << ": CKR_DEVICE_ERROR\n"; }; 
                break;
            case CKR_TOKEN_NOT_PRESENT: 
                { std::cerr << function_name << ": CKR_TOKEN_NOT_PRESENT\n"; }; 
                break;
            // CKR_DEVICE_REMOVED can also be returned by a function that uses a token
        // Special return value for application-supplied callbacks
            case CKR_CANCEL: 
                { std::cerr << function_name << ": CKR_CANCEL\n"; }; 
                break;
        // Special return values for mutex-handling functions
            case CKR_MUTEX_BAD: 
                { std::cerr << function_name << ": CKR_MUTEX_BAD\n"; }; 
                break;
            case CKR_MUTEX_NOT_LOCKED: 
                { std::cerr << function_name << ": CKR_MUTEX_NOT_LOCKED\n"; }; 
                break;
        // All other Cryptoki function return values
            case CKR_ACTION_PROHIBITED: 
                { std::cerr << function_name << ": CKR_ACTION_PROHIBITED\n"; }; 
                break;
            case CKR_ARGUMENTS_BAD: 
                { std::cerr << function_name << ": CKR_ARGUMENTS_BAD\n"; }; 
                break;
            case CKR_ATTRIBUTE_READ_ONLY: 
                { std::cerr << function_name << ": CKR_ATTRIBUTE_READ_ONLY\n"; }; 
                break;
            case CKR_ATTRIBUTE_SENSITIVE: 
                { std::cerr << function_name << ": CKR_ATTRIBUTE_SENSITIVE\n"; }; 
                break;
            case CKR_ATTRIBUTE_TYPE_INVALID: 
                { std::cerr << function_name << ": CKR_ATTRIBUTE_TYPE_INVALID\n"; }; 
                break;
            case CKR_ATTRIBUTE_VALUE_INVALID: 
                { std::cerr << function_name << ": CKR_ATTRIBUTE_VALUE_INVALID\n"; }; 
                break;
            case CKR_BUFFER_TOO_SMALL: 
                { std::cerr << function_name << ": CKR_BUFFER_TOO_SMALL\n"; }; 
                break;
            case CKR_CANT_LOCK: 
                { std::cerr << function_name << ": CKR_CANT_LOCK\n"; }; 
                break;
            case CKR_CRYPTOKI_ALREADY_INITIALIZED: 
                { std::cerr << function_name << ": CKR_CRYPTOKI_ALREADY_INITIALIZED\n"; }; 
                break;
            case CKR_CRYPTOKI_NOT_INITIALIZED: 
                { std::cerr << function_name << ": CKR_CRYPTOKI_NOT_INITIALIZED\n"; }; 
                break;
            /*case CKR_CURVE_NOT_SUPPORTED: 
                { std::cerr << function_name << ": CKR_CURVE_NOT_SUPPORTED\n"; }; 
                break;*/
            case CKR_DATA_INVALID: 
                { std::cerr << function_name << ": CKR_DATA_INVALID\n"; }; 
                break;
            case CKR_DATA_LEN_RANGE: 
                { std::cerr << function_name << ": CKR_DATA_LEN_RANGE\n"; }; 
                break;
            case CKR_DOMAIN_PARAMS_INVALID: 
                { std::cerr << function_name << ": CKR_DOMAIN_PARAMS_INVALID\n"; }; 
                break;
            case CKR_ENCRYPTED_DATA_INVALID: 
                { std::cerr << function_name << ": CKR_ENCRYPTED_DATA_INVALID\n"; }; 
                break;
            case CKR_ENCRYPTED_DATA_LEN_RANGE: 
                { std::cerr << function_name << ": CKR_ENCRYPTED_DATA_LEN_RANGE\n"; }; 
                break;
            case CKR_EXCEEDED_MAX_ITERATIONS: 
                { std::cerr << function_name << ": CKR_EXCEEDED_MAX_ITERATIONS\n"; }; 
                break;
            case CKR_FIPS_SELF_TEST_FAILED: 
                { std::cerr << function_name << ": CKR_FIPS_SELF_TEST_FAILED\n"; }; 
                break;
            case CKR_FUNCTION_CANCELED: 
                { std::cerr << function_name << ": CKR_FUNCTION_CANCELED\n"; }; 
                break;
            case CKR_FUNCTION_NOT_PARALLEL: 
                { std::cerr << function_name << ": CKR_FUNCTION_NOT_PARALLEL\n"; }; 
                break;
            case CKR_FUNCTION_NOT_SUPPORTED: 
                { std::cerr << function_name << ": CKR_FUNCTION_NOT_SUPPORTED\n"; }; 
                break;
            case CKR_FUNCTION_REJECTED: 
                { std::cerr << function_name << ": CKR_FUNCTION_REJECTED\n"; }; 
                break;
            case CKR_INFORMATION_SENSITIVE: 
                { std::cerr << function_name << ": CKR_INFORMATION_SENSITIVE\n"; }; 
                break;
            case CKR_KEY_CHANGED: 
                { std::cerr << function_name << ": CKR_KEY_CHANGED\n"; }; 
                break;
            case CKR_KEY_FUNCTION_NOT_PERMITTED: 
                { std::cerr << function_name << ": CKR_KEY_FUNCTION_NOT_PERMITTED\n"; }; 
                break;
            case CKR_KEY_HANDLE_INVALID: 
                { std::cerr << function_name << ": CKR_KEY_HANDLE_INVALID\n"; }; 
                break;
            case CKR_KEY_INDIGESTIBLE: 
                { std::cerr << function_name << ": CKR_KEY_INDIGESTIBLE\n"; }; 
                break;
            case CKR_KEY_NEEDED: 
                { std::cerr << function_name << ": CKR_KEY_NEEDED\n"; }; 
                break;
            case CKR_KEY_NOT_WRAPPABLE: 
                { std::cerr << function_name << ": CKR_KEY_NOT_WRAPPABLE\n"; }; 
                break;
            case CKR_KEY_SIZE_RANGE: 
                { std::cerr << function_name << ": CKR_KEY_SIZE_RANGE\n"; }; 
                break;
            case CKR_KEY_TYPE_INCONSISTENT: 
                { std::cerr << function_name << ": CKR_KEY_TYPE_INCONSISTENT\n"; }; 
                break;
            case CKR_KEY_UNEXTRACTABLE: 
                { std::cerr << function_name << ": CKR_KEY_UNEXTRACTABLE\n"; }; 
                break;
            case CKR_LIBRARY_LOAD_FAILED: 
                { std::cerr << function_name << ": CKR_LIBRARY_LOAD_FAILED\n"; }; 
                break;
            case CKR_MECHANISM_INVALID: 
                { std::cerr << function_name << ": CKR_MECHANISM_INVALID\n"; }; 
                break;
            case CKR_MECHANISM_PARAM_INVALID: 
                { std::cerr << function_name << ": CKR_MECHANISM_PARAM_INVALID\n"; }; 
                break;
            case CKR_NEED_TO_CREATE_THREADS: 
                { std::cerr << function_name << ": CKR_NEED_TO_CREATE_THREADS\n"; }; 
                break;
            case CKR_NO_EVENT: 
                { std::cerr << function_name << ": CKR_NO_EVENT\n"; }; 
                break;
            case CKR_OBJECT_HANDLE_INVALID: 
                { std::cerr << function_name << ": CKR_OBJECT_HANDLE_INVALID\n"; }; 
                break;
            case CKR_OPERATION_ACTIVE: 
                { std::cerr << function_name << ": CKR_OPERATION_ACTIVE\n"; }; 
                break;
            case CKR_OPERATION_NOT_INITIALIZED: 
                { std::cerr << function_name << ": CKR_OPERATION_NOT_INITIALIZED\n"; }; 
                break;
            case CKR_PIN_EXPIRED: 
                { std::cerr << function_name << ": CKR_PIN_EXPIRED\n"; }; 
                break;
            case CKR_PIN_INCORRECT: 
                { std::cerr << function_name << ": CKR_PIN_INCORRECT\n"; }; 
                break;
            case CKR_PIN_INVALID: 
                { std::cerr << function_name << ": CKR_PIN_INVALID\n"; }; 
                break;
            case CKR_PIN_LEN_RANGE: 
                { std::cerr << function_name << ": CKR_PIN_LEN_RANGE\n"; }; 
                break;
            case CKR_PIN_LOCKED: 
                { std::cerr << function_name << ": CKR_PIN_LOCKED\n"; }; 
                break;
            case CKR_PIN_TOO_WEAK: 
                { std::cerr << function_name << ": CKR_PIN_TOO_WEAK\n"; }; 
                break;
            case CKR_PUBLIC_KEY_INVALID: 
                { std::cerr << function_name << ": CKR_PUBLIC_KEY_INVALID\n"; }; 
                break;
            case CKR_RANDOM_NO_RNG: 
                { std::cerr << function_name << ": CKR_RANDOM_NO_RNG\n"; }; 
                break;
            case CKR_RANDOM_SEED_NOT_SUPPORTED: 
                { std::cerr << function_name << ": CKR_RANDOM_SEED_NOT_SUPPORTED\n"; }; 
                break;
            case CKR_SAVED_STATE_INVALID: 
                { std::cerr << function_name << ": CKR_SAVED_STATE_INVALID\n"; }; 
                break;
            case CKR_SESSION_COUNT: 
                { std::cerr << function_name << ": CKR_SESSION_COUNT\n"; }; 
                break;
            case CKR_SESSION_EXISTS: 
                { std::cerr << function_name << ": CKR_SESSION_EXISTS\n"; }; 
                break;
            case CKR_SESSION_PARALLEL_NOT_SUPPORTED: 
                { std::cerr << function_name << ": CKR_SESSION_PARALLEL_NOT_SUPPORTED\n"; }; 
                break;
            case CKR_SESSION_READ_ONLY: 
                { std::cerr << function_name << ": CKR_SESSION_READ_ONLY\n"; }; 
                break;
            case CKR_SESSION_READ_ONLY_EXISTS: 
                { std::cerr << function_name << ": CKR_SESSION_READ_ONLY_EXISTS\n"; }; 
                break;
            case CKR_SESSION_READ_WRITE_SO_EXISTS: 
                { std::cerr << function_name << ": CKR_SESSION_READ_WRITE_SO_EXISTS\n"; }; 
                break;
            case CKR_SIGNATURE_LEN_RANGE: 
                { std::cerr << function_name << ": CKR_SIGNATURE_LEN_RANGE\n"; }; 
                break;
            case CKR_SIGNATURE_INVALID: 
                { std::cerr << function_name << ": CKR_SIGNATURE_INVALID\n"; }; 
                break;
            case CKR_SLOT_ID_INVALID: 
                { std::cerr << function_name << ": CKR_SLOT_ID_INVALID\n"; }; 
                break;
            case CKR_STATE_UNSAVEABLE: 
                { std::cerr << function_name << ": CKR_STATE_UNSAVEABLE\n"; }; 
                break;
            case CKR_TEMPLATE_INCOMPLETE: 
                { std::cerr << function_name << ": CKR_TEMPLATE_INCOMPLETE\n"; }; 
                break;
            case CKR_TEMPLATE_INCONSISTENT: 
                { std::cerr << function_name << ": CKR_TEMPLATE_INCONSISTENT\n"; }; 
                break;
            case CKR_TOKEN_NOT_RECOGNIZED: 
                { std::cerr << function_name << ": CKR_TOKEN_NOT_RECOGNIZED\n"; }; 
                break;
            case CKR_TOKEN_WRITE_PROTECTED: 
                { std::cerr << function_name << ": CKR_TOKEN_WRITE_PROTECTED\n"; }; 
                break;
            case CKR_UNWRAPPING_KEY_HANDLE_INVALID: 
                { std::cerr << function_name << ": CKR_UNWRAPPING_KEY_HANDLE_INVALID\n"; }; 
                break;
            case CKR_UNWRAPPING_KEY_SIZE_RANGE: 
                { std::cerr << function_name << ": CKR_UNWRAPPING_KEY_SIZE_RANGE\n"; }; 
                break;
            case CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT: 
                { std::cerr << function_name << ": CKR_UNWRAPPING_KEY_TYPE_INCONSISTENT\n"; }; 
                break;
            case CKR_USER_ALREADY_LOGGED_IN: 
                { std::cerr << function_name << ": CKR_USER_ALREADY_LOGGED_IN\n"; }; 
                break;
            case CKR_USER_ANOTHER_ALREADY_LOGGED_IN: 
                { std::cerr << function_name << ": CKR_USER_ANOTHER_ALREADY_LOGGED_IN\n"; }; 
                break;
            case CKR_USER_NOT_LOGGED_IN: 
                { std::cerr << function_name << ": CKR_USER_NOT_LOGGED_IN\n"; }; 
                break;
            case CKR_USER_PIN_NOT_INITIALIZED: 
                { std::cerr << function_name << ": CKR_USER_PIN_NOT_INITIALIZED\n"; }; 
                break;
            case CKR_USER_TOO_MANY_TYPES: 
                { std::cerr << function_name << ": CKR_USER_TOO_MANY_TYPES\n"; }; 
                break;
            case CKR_USER_TYPE_INVALID: 
                { std::cerr << function_name << ": CKR_USER_TYPE_INVALID\n"; }; 
                break;
            case CKR_WRAPPED_KEY_INVALID: 
                { std::cerr << function_name << ": CKR_WRAPPED_KEY_INVALID\n"; }; 
                break;
            case CKR_WRAPPED_KEY_LEN_RANGE: 
                { std::cerr << function_name << ": CKR_WRAPPED_KEY_LEN_RANGE\n"; }; 
                break;
            case CKR_WRAPPING_KEY_HANDLE_INVALID: 
                { std::cerr << function_name << ": CKR_WRAPPING_KEY_HANDLE_INVALID\n"; }; 
                break;
            case CKR_WRAPPING_KEY_SIZE_RANGE: 
                { std::cerr << function_name << ": \n"; }; 
                break;
            case CKR_WRAPPING_KEY_TYPE_INCONSISTENT: 
                { std::cerr << function_name << ": \n"; }; 
                break;
            /*case CKR_OPERATION_CANCEL_FAILED: 
                { std::cerr << function_name << ": \n"; }; 
                break;*/
            default:
                break;
        }
        return rv;
    }
}


