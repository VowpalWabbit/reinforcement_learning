#pragma once

#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "io/logger.h"
#include "joiners/example_joiner.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace log_converter {
void build_cb_json(std::ofstream &outfile, const joined_event::joined_event &je,
                   float reward, float original_reward,
                   bool skip_learn = false);
} // namespace log_converter
