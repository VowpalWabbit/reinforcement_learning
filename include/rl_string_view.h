#pragma once

// Rely on the VW implementation of string_view
#include "../ext_libs/vowpal_wabbit/vowpalwabbit/vw_string_view.h"

namespace reinforcement_learning
{
using string_view = VW::string_view;
}
