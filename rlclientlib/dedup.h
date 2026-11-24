#pragma once
#include "configuration.h"
#include "logger/logger_facade.h"

namespace reinforcement_learning
{
bool should_use_dedup_logger_extension(const utility::configuration& config, const char* section);

std::unique_ptr<logger::i_logger_extensions> create_dedup_logger_extension(
    const utility::configuration& config, const char* section, std::unique_ptr<i_time_provider> time_provider);
}  // namespace reinforcement_learning
