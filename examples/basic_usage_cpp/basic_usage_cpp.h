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

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;

int basic_usage_cb();
int basic_usage_ccb();
int basic_usage_slates();

int load_file(const std::string& file_name, std::string& file_data);
int load_config_from_json(const std::string& file_name, u::configuration& cc);
