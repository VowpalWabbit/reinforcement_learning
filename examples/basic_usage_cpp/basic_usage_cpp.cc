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

int main()
{
  // return basic_usage_cb();
  // return basic_usage_ca();
  // return basic_usage_ccb();
  // return basic_usage_slates();
  return basic_usage_multistep();
}

int basic_usage_cb()
{
  char const* const event_id = "event_id";
  char const* const context = R"({"shared":{"sf":1},"_multi":[ { "TAction":{"af":1} },{"TAction":{"af":2}}]})";
  float outcome = 1.0f;

  //! name, value based config object used to initialise the API
  u::configuration config;

  //! Helper method to initialize config from a json file
  if (load_config_from_json("client.json", config) != err::success)
  {
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
  if (rl.init(&status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(2) Initialize the API]

  //! [(3) Choose an action]
  // Response class
  r::ranking_response response;

  if (rl.choose_rank(event_id, context, response, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(3) Choose an action]

  //! [(4) Use the response]
  size_t chosen_action = 0;
  if (response.get_chosen_action_id(chosen_action, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  std::cout << "Chosen action id is: " << chosen_action << std::endl;
  //! [(4) Use the response]

  //! [(5) Report outcome]
  //     Report received outcome (Optional: if this call is not made, default missing outcome is applied)
  //     Missing outcome can be thought of as negative reinforcement
  if (rl.report_outcome(event_id, outcome, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(5) Report outcome]

  return 0;
}

int basic_usage_ca()
{
  char const* const event_id = "event_id";
  std::string context = R"({"f1":1,"f2":1,"f3":1,"f4":1,"f5":1})";
  float outcome = 1.0f;

  //! name, value based config object used to initialise the API
  u::configuration config;

  //! Helper method to initialize config from a json file
  if (load_config_from_json("client.json", config) != err::success)
  {
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
  if (rl.init(&status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(2) Initialize the API]

  //! [(3) Choose a continuous action]
  // Response class
  r::continuous_action_response response;

  RL_IGNORE_DEPRECATED_USAGE_START
  if (rl.request_continuous_action(event_id, context, response, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  RL_IGNORE_DEPRECATED_USAGE_END
  //! [(3) Choose a continuous action]

  //! [(4) Use the response]
  float chosen_action = response.get_chosen_action();

  std::cout << "Chosen continuous action is: " << chosen_action << std::endl;
  //! [(4) Use the response]

  //! [(5) Report outcome]
  //     Report received outcome (Optional: if this call is not made, default missing outcome is applied)
  //     Missing outcome can be thought of as negative reinforcement
  if (rl.report_outcome(event_id, outcome, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(5) Report outcome]

  return 0;
}

int basic_usage_ccb()
{
  char const* const context =
      R"({"shared":{"sf":1},"_multi":[ { "TAction":{"af":1} },{"TAction":{"af":2}},{"TAction":{"af":3}}],"_slots":[{"f":1}, {"f":2}]})";

  //! name, value based config object used to initialise the API
  u::configuration config;

  //! Helper method to initialize config from a json file
  if (load_config_from_json("client.json", config) != err::success)
  {
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
  if (rl.init(&status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(2) Initialize the API]

  //! [(3) Choose an action]
  // Response class
  r::decision_response response;

  RL_IGNORE_DEPRECATED_USAGE_START
  if (rl.request_decision(context, response, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  RL_IGNORE_DEPRECATED_USAGE_END
  //! [(3) Choose an action]

  //! [(4) Use the response / report outcome]
  for (const auto& r : response)
  {
    const auto chosen_action = r.get_action_id();
    if (rl.report_outcome(r.get_slot_id(), 1.0f, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return -1;
    }
  }
  return 0;
}

int basic_usage_slates()
{
  std::string context =
      R"({"shared":{"sf":1},"_multi":[ {"_slot_id":0,"TAction":{"af":1} },{"_slot_id":0,"TAction":{"af":2}},{"_slot_id":1,"TAction":{"af":3}},{"_slot_id":1,"TAction":{"af":4}}],"_slots":[{"f":1}, {"f":2}]})";

  //! name, value based config object used to initialise the API
  u::configuration config;

  //! Helper method to initialize config from a json file
  if (load_config_from_json("client.json", config) != err::success)
  {
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
  if (rl.init(&status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  //! [(2) Initialize the API]

  //! [(3) Choose an action]
  // Response class
  r::multi_slot_response response;

  RL_IGNORE_DEPRECATED_USAGE_START
  if (rl.request_multi_slot_decision("event_id", context, response, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  RL_IGNORE_DEPRECATED_USAGE_END
  //! [(3) Choose an action]

  //! [(4) Use the response]
  for (const auto& r : response) { const auto chosen_action = r.get_action_id(); }

  //! [(5) Report outcome]
  if (rl.report_outcome("event_id", 1.0f, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }
  return 0;
}

int basic_usage_multistep()
{
  //! name, value based config object used to initialise the API
  u::configuration config;

  //! Helper method to initialize config from a json file
  if (load_config_from_json("client.json", config) != err::success)
  {
    std::cout << "Unable to Load file: client.json" << std::endl;
    return -1;
  }

  r::api_status status;

  r::live_model rl(config);

  if (rl.init(&status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  r::episode_state episode1("my_episode_id_1");
  r::episode_state episode2("my_episode_id_2");

  {
    const std::string context1 = R"({"shared":{"F1": 1.0}, "_multi": [{"AF1": 2.0}, {"AF1": 3.0}]})";
    r::ranking_response response1;
    if (rl.request_episodic_decision("event1", nullptr, context1.c_str(), response1, episode1, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return -1;
    }
  }

  {
    const std::string context1 = R"({"shared":{"F2": 1.0}, "_multi": [{"AF2": 2.0}, {"AF2": 3.0}]})";
    r::ranking_response response1;
    if (rl.request_episodic_decision("event1", nullptr, context1.c_str(), response1, episode2, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return -1;
    }
  }

  {
    const std::string context2 = R"({"shared":{"F1": 4.0}, "_multi": [{"AF1": 2.0}, {"AF1": 3.0}]})";
    r::ranking_response response2;
    if (rl.request_episodic_decision("event2", "event1", context2.c_str(), response2, episode1, &status) !=
        err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return -1;
    }
  }

  {
    const std::string context2 = R"({"shared":{"F2": 4.0}, "_multi": [{"AF2": 2.0}, {"AF2": 3.0}]})";
    r::ranking_response response2;
    if (rl.request_episodic_decision("event2", "event1", context2.c_str(), response2, episode2, &status) !=
        err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return -1;
    }
  }

  {
    if (rl.report_outcome(episode1.get_episode_id(), "event1", 1.0f, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return -1;
    }
  }

  {
    if (rl.report_outcome(episode2.get_episode_id(), "event2", 1.0f, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return -1;
    }
  }

  return 0;
}

// Helper methods

//! Load config from json file
int load_config_from_json(const std::string& file_name, u::configuration& config)
{
  std::string config_str;
  // Load contents of config file into a string
  const auto scode = load_file(file_name, config_str);
  if (scode != 0) { return scode; }

  //! [Create a configuration from json string]
  // Use library supplied convenience method to parse json and build config object
  // namespace cfg=reinforcement_learning::utility::config;

  return cfg::create_from_json(config_str, config);
  //! [Create a configuration from json string]
}

//! Load contents of file into a string
int load_file(const std::string& file_name, std::string& config_str)
{
  std::ifstream fs;
  fs.open(file_name);
  if (!fs.good()) { return reinforcement_learning::error_code::invalid_argument; }
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return reinforcement_learning::error_code::success;
}
