/**
 * @brief Simple RL Inference API sample implementation
 *
 * @file basic_usage_cpp.cc
 * @author Rajan Chari et al
 * @date 2018-07-15
 */
#include "basic_usage_cpp.h"

/**
 * @brief Basic API usage example
 *
 * @return int Error code
 */
int cb_basic_usage() {
  //! name, value based config object used to initialise the API
  u::configuration config;

  //! Helper method to initialize config from a json file
  if (load_config_from_json("client.json", config) != err::success) {
    std::cout << "Unable to Load file: client.json" << std::endl;
    return -1;
  }

  /** api_status is an optional argument used to get detailed
   *  error description from all API calls
   */
  r::api_status status;

  //! [(1) Instantiate Inference API using config]
  r::live_model rl(config);
  //! [(1) Instantiate Inference API using config]

  //! [(2) Initialize the API]
  if (rl.init(&status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(2) Initialize the API]

  //! [(3) Choose an action]
  // Response class
  r::ranking_response response;

  if (rl.choose_rank(event_id, context, response, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(3) Choose an action]

  //! [(4) Use the response]
  size_t chosen_action;
  if (response.get_chosen_action_id(chosen_action, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  std::cout << "Chosen action id is: " << chosen_action << std::endl;
  //! [(4) Use the response]

  //! [(5) Report outcome]
  //     Report received outcome (Optional: if this call is not made, default missing outcome is applied)
  //     Missing outcome can be thought of as negative reinforcement
  if (rl.report_outcome(event_id, outcome, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(5) Report outcome]

  return 0;
}

int multistep_basic_usage() {
  //! name, value based config object used to initialise the API
  u::configuration config;

  //! Helper method to initialize config from a json file
  if (load_config_from_json("client.json", config) != err::success) {
    std::cout << "Unable to Load file: client.json" << std::endl;
    return -1;
  }

  r::api_status status;

  r::live_model rl(config);

  if (rl.init(&status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  r::episode_state episode("my_episode_id");

  r::ranking_response response1;
  const std::string context1 = R"({"F": 1.0, "_multi": [{"AF": 2.0}, {"AF": 3.0}]})";

  if (rl.request_episodic_decision("event1", nullptr, context1.c_str(), response1, episode, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  const std::string context2 = R"({"F": 4.0, "_multi": [{"AF": 2.0}, {"AF": 3.0}]})";
  r::ranking_response response2;
  if (rl.request_episodic_decision("event2", "event1", context2.c_str(), response2, episode, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  if (rl.report_outcome(episode.get_episode_id(), "event1", 1.0f, &status)) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  if (rl.report_outcome(episode.get_episode_id(), "event2", 1.0f, &status)) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  return 0;
}

int main() {
  return multistep_basic_usage();
  //return cb_basic_usage();
}

// Helper methods

//! Load config from json file
int load_config_from_json(const std::string& file_name, u::configuration& config) {
  std::string config_str;
  // Load contents of config file into a string
  const auto scode = load_file(file_name, config_str);
  if ( scode != 0 ) return scode;

  //! [Create a configuration from json string]
  // Use library supplied convenience method to parse json and build config object
  // namespace cfg=reinforcement_learning::utility::config;

  return cfg::create_from_json(config_str, config);
  //! [Create a configuration from json string]
}

//! Load contents of file into a string
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
