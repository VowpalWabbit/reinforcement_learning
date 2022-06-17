#include "options.h"

#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

bool is_help(const po::variables_map& vm) { return vm.count("help") > 0; }

po::variables_map process_cmd_line(const int argc, char** argv)
{
  po::options_description desc("Options");
  desc.add_options()("help,h", "produce help message")("json_config,j",
      po::value<std::string>()->default_value("client.json"), "JSON file with config information for hosted RL loop")(
      "threads,t", po::value<size_t>()->default_value(1), "Number of threads per instance")(
      "examples,n", po::value<size_t>()->default_value(10), "Number of examples per thread")("features,x",
      po::value<size_t>()->default_value(10), "Features count")("actions,a", po::value<size_t>()->default_value(2),
      "Number of actions")("experiment_name,e", po::value<std::string>()->required(), "(REQUIRED) experiment name")(
      "float_outcome,f", "if outcome is float (otherwise - json)")("sleep,s", po::value<size_t>()->default_value(0),
      "Milliseconds to sleep between loop iterations")("duration,d", po::value<size_t>(),
      "Duration of experiment (in ms). Alternative to n")("instances,i", po::value<size_t>()->default_value(1),
      "Number of test loop instances")("reward_period,r", po::value<size_t>()->default_value(0),
      "Ratio period (0 - no reward, otherwise - every $reward_period interaction is receiving reward)")(
      "slots,q", po::value<size_t>()->default_value(0), "Number of slots (ccb simulation is running if > 0)")(
      "episode_length,m", po::value<size_t>()->default_value(0), "Length of an episode (running multistep if > 0)");

  po::variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);

  if (is_help(vm)) std::cout << desc << std::endl;

  return vm;
}

void throw_if_conflicting(const po::variables_map& vm, const std::string& first, const std::string& second)
{
  if (vm.count(first) && !vm[first].defaulted() && vm.count(second) && !vm[second].defaulted())
  {
    throw std::logic_error(std::string("Conflicting options '") + first + "' and '" + second + "'.");
  }
}
