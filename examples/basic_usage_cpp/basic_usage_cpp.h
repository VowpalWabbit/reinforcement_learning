/**
 * @brief Simple RL Inference API sample *
 *
 * @file basic_usage_cpp.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once

#include "config_utility.h"
#include "live_model.h"
#include "live_model_ca.h"
#include "live_model_cb.h"
#include "live_model_ccb.h"
#include "live_model_slates.h"

#include <fstream>
#include <iostream>

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;

int basic_usage_cb();
int basic_usage_ca();
int basic_usage_ccb();
int basic_usage_slates();
int basic_usage_multistep();

int load_file(const std::string& file_name, std::string& config_str);
int load_config_from_json(const std::string& file_name, u::configuration& config);
