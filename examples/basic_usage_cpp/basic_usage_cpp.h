/**
 * @brief Simple RL Inference API sample * 
 * 
 * @file basic_usage_cpp.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once

#include <iostream>
#include <fstream>
#include "config_utility.h"
#include "live_model.h"
#include "rl_logger.h"

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;

int load_file(const std::string& file_name, std::string& file_data);
int load_config_from_json(const std::string& file_name, u::configuration& cc);

char const * const  event_id    = "event_id";
char const * const  first_slot_id = "slot1";

char const * const  contextCB = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[ { "TAction":{"a1":"f1"} },{"TAction":{"a2":"f2"}}]})";
char const * const  contextCCB = R"({"GUser":{"uf":1},"_multi":[{"TAction":{"af":1}},{"TAction":{"af":2}},{"TAction":{"af":3}}],"_slots":[{"_id":"slot1"},{"_id":"slot2"}]})";

float outcome  = 1.0f;
