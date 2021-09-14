#include "rl_sim_cpp.h"

// Entry point of console apps
int main(int argc, char** argv) {
  try {
    const auto vm = process_cmd_line(argc, argv);
    if ( is_help(vm) ) return 0;

    // Instantiate reinforcement learning simulator
    rl_sim sim(vm);

    // Run loop: (1) world event (2) choose action (3) report outcome
    return sim.loop();
  }
  catch ( const std::exception& e ) {
    std::cout << "Error: " << e.what() << std::endl;
    return -1;
  }
}

// Helper functions

po::variables_map process_cmd_line(const int argc, char** argv) {
  po::options_description desc("Options");
  desc.add_options()
    ( "help", "produce help message" )
    ( "json_config,j", po::value<std::string>()->
      default_value("client.json"), "JSON file with config information for hosted RL loop" )
    ("log_to_file,l", po::value<bool>()->
      default_value(false), "Log interactions and observations to local files")
    ("get_model,m", po::value<bool>()->
      default_value(true), "Download model from model source")
    ("log_timestamp,t", po::value<bool>()->
      default_value(true), "Apply timestamp to all logged message")
    ("ccb", po::value<bool>()->
      default_value(false), "Run in ccb mode")
    ("slates", po::value<bool>()->
      default_value(false), "Run in slates mode")
    ("ca", po::value<bool>()->
      default_value(false), "Run in continuous actions mode");

  po::variables_map vm;
  store(parse_command_line(argc, argv, desc), vm);

  if ( is_help(vm) )
    std::cout << desc << std::endl;

  return vm;
}

bool is_help(const po::variables_map& vm) {
  return vm.count("help") > 0;
}
