#pragma once

#include "config_utility.h"
#include "live_model.h"
#include "rl_sim.h"

#include <boost/program_options.hpp>
#include <exception>
#include <fstream>
#include <iostream>

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;
namespace po = boost::program_options;

// Forward declare functions
po::variables_map process_cmd_line(const int argc, char** argv);
bool is_help(const po::variables_map& vm);
