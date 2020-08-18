#include "rl.net.native.h"

using namespace reinforcement_learning::error_code;

API const char* LookupMessageForErrorCode(int error_code)
{

#define ERROR_CODE_DEFINITION(code, name, msg) \
    case code : \
        return name ## _s;

    switch (error_code)
    {
        case 0:
            return "Success.";
        default:
            return unknown_s;

        #include "errors_data.h"
    }

#undef ERROR_CODE_DEFINITION
}
