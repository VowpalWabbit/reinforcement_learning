#pragma once

#include "future_compat.h"

#ifdef HAS_STD17
#  include <string_view>
namespace reinforcement_learning
{
using std::string_view;
}
#else
#  include "nonstd/string_view.h"
namespace reinforcement_learning
{
using nonstd::string_view;
}
#endif
