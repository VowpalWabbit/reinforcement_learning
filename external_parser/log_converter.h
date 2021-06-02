#pragma once

#include <iostream>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "joiners/example_joiner.h"
#include "io/logger.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace log_converter {
void build_cb_json(std::ofstream &outfile, const joined_event &je, float reward,
                   float original_reward);
} // namespace log_converter
