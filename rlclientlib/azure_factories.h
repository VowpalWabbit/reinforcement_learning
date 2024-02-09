#pragma once

#include "oauth_callback_fn.h"

namespace reinforcement_learning
{
void register_azure_factories();

void register_azure_oauth_factories(oauth_callback_t& callback);
}
