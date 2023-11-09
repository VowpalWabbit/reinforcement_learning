#pragma once

#include "ca_loop.h"
#include "cb_loop.h"
#include "ccb_loop.h"
#include "config_utility.h"
#include "live_model.h"

#include <fstream>
#include <iostream>

#if defined(_MSC_VER)
//  Microsoft
#  define API __declspec(dllexport)
#elif defined(__GNUC__)
//  GCC
#  define API __attribute__((visibility("default")))
#else
//  do nothing and hope for the best?
#  define API
#  pragma warning Unknown dynamic link import / export semantics.
#endif

namespace rl_net_native
{
typedef void (*background_error_callback_t)(const reinforcement_learning::api_status&);
}

extern "C"
{
  API const char* LookupMessageForErrorCode(int code);
}
