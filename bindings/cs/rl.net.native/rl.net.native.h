#pragma once

#include <iostream>
#include <fstream>
#include "config_utility.h"
#include "live_model.h"

#if defined(_MSC_VER)
    //  Microsoft
    #define API __declspec(dllexport)
#elif defined(__GNUC__)
    //  GCC
    #define API __attribute__((visibility("default")))
#else
    //  do nothing and hope for the best?
    #define API
    #pragma warning Unknown dynamic link import/export semantics.
#endif
