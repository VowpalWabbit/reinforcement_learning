#include "test_loop.h"

#include <iostream>

namespace po = boost::program_options;

bool is_help(const po::variables_map& vm) {
  return vm.count("help") > 0;
}

po::variables_map process_cmd_line(const int argc, char** argv) {
  po::options_description desc("Options");
  desc.add_options()
    ("help", "produce help message")
    ("json_config,j", po::value<std::string>()->
      default_value("client.json"), "JSON file with config information for hosted RL loop")
    ("message_size,s", po::value<size_t>()->default_value(100), "Message size in Kb")
    ("message_count,n", po::value<size_t>()->default_value(1000000), "Amount of messages")
    ("threads,t", po::value<size_t>()->default_value(1))
    ;

  po::variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);

  if (is_help(vm))
    std::cout << desc << std::endl;

  return vm;
}

int main(int argc, char** argv) {
  try {
    const auto vm = process_cmd_line(argc, argv);
    if (is_help(vm)) return 0;

    test_loop loop(vm);
    if (!loop.init()) {
      std::cerr << "Test loop haven't initialized properly." << std::endl;
      return -1;
    }
    for (int i = 0; i < 5; ++i) {
      loop.run();
    }
  }
  catch (const std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
    return -1;
  }
}
