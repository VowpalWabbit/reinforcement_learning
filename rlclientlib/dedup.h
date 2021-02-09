#pragma once
#include "logger/logger_facade.h"
#include "configuration.h"

namespace reinforcement_learning
{
  logger::i_logger_extensions* create_dedup_logger_extension(const utility::configuration& config, const char* section, i_time_provider* time_provider);
}
