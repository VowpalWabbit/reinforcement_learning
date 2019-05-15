#include "live_model.h"
#include "config_utility.h"

#include <iostream>

#include <iostream>
#include <fstream>
#include "config_utility.h"
#include "live_model.h"

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;

int load_file(const std::string& file_name, std::string& file_data);
int load_config_from_json(const std::string& file_name, u::configuration& cc);

char const * const  event_id = "event_id";
char const * const  context = R"({
                                  "GUser":{"id":"a","major":"eng","hobby":"hiking"},
                                  "_multi":[ { "TAction":{"a1":"f1"} },{"TAction":{"a2":"f2"}}],
                                  "_slots":[ { "TSlot":{"s1":"f1"} },{"TSlot":{"s2":"f2"}}]})";
float outcome = 1.0f;

int main() {
  u::configuration config;
  if( load_config_from_json("client.json", config) != err::success ) {
    std::cout << "Unable to Load file: client.json" << std::endl;
    return -1;
  }
  r::api_status status;
  r::live_model rl(config);
  if( rl.init(&status) != err::success ) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  for (int i = 0; i < 100000; i++)
  {
    r::decision_response response;
    if (rl.request_decision(context, response, &status) != err::success) {
      std::cout << status.get_error_msg() << std::endl;
      return -1;
    }
  }
  // Response class

  //size_t chosen_action;
  //std::cout << "Chosen action id is: " << chosen_action << std::endl;
  ////     Report received outcome (Optional: if this call is not made, default missing outcome is applied)
  ////     Missing outcome can be thought of as negative reinforcement
  //if( rl.report_outcome(event_id, outcome, &status) != err::success ) {
  //  std::cout << status.get_error_msg() << std::endl;
  //  return -1;
  //}
  return 0;
}

// Helper methods
int load_config_from_json(const std::string& file_name, u::configuration& config) {
  std::string config_str;
  // Load contents of config file into a string
  const auto scode = load_file(file_name, config_str);
  if ( scode != 0 ) return scode;
  // Use library supplied convenience method to parse json and build config object
  // namespace cfg=reinforcement_learning::utility::config;

  return cfg::create_from_json(config_str, config);
}
int load_file(const std::string& file_name, std::string& config_str) {
  std::ifstream fs;
  fs.open(file_name);
  if ( !fs.good() )
    return reinforcement_learning::error_code::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return reinforcement_learning::error_code::success;
}
