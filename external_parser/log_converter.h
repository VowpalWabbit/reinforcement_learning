#pragma once

#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "vw/io/logger.h"
#include "joiners/example_joiner.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace log_converter {
void build_json(std::ofstream &outfile,
                joined_event::joined_event &je, VW::io::logger& logger);
void build_cb_json(std::ofstream &outfile,
                   joined_event::joined_event &je, VW::io::logger& logger);
void build_ccb_json(std::ofstream &outfile,
                   joined_event::joined_event &je, VW::io::logger& logger);
void build_ca_json(std::ofstream &outfile, joined_event::joined_event &je, VW::io::logger& logger);
void build_slates_json(std::ofstream &outfile, joined_event::joined_event &je, VW::io::logger& logger);
} // namespace log_converter
