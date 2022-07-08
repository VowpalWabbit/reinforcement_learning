#pragma once

#ifdef HAS_STD17
namespace reinforcement_learning
{
#include <string_view>
using std::string_view;
}
#else
#include "nonstd/string_view.h"
namespace reinforcement_learning
{
using string_view = nonstd::string_view;
}
#endif
