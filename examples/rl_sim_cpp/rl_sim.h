/**
 * @brief RL Simulator example defintion.  Simulates user/world interacting with RL
 *
 * @file rl_sim.h
 * @author Rajan Chari et al
 * @date 2018-07-18
 */
#pragma once
#include "azure_credentials.h"
#include "live_model.h"
#include "person.h"
#include "robot_joint.h"

#include <boost/program_options.hpp>

/**
 * @brief Reinforcement Learning Simulator
 * This class simulates a user interacting with a reinforcement learning loop where
 * a person is chosen in random then an action is chosen and the outcome generated.
 * The generated outcome is reported back to the reinforcement learning loop.
 */
class rl_sim
{
public:
  /**
   * @brief Construct a new rl_sim object
   *
   * @param vm User defined options
   */
  explicit rl_sim(boost::program_options::variables_map vm);

  /**
   * @brief Simulation loop
   *
   * @return int Error status
   */
  int loop();

  /**
   * @brief error handler for background errors
   * _on_error free funciton is registered as the background error handler with the api
   * on_error is called by _on_error()
   */
  void on_error(const reinforcement_learning::api_status& status);

private:
  /**
   * @brief Create a context json string
   * Context json is constructed from user context json and action json
   * @param cntxt User context json
   * @param action Action json
   * @return std::string Constructed context json
   */
  static std::string create_context_json(const std::string& cntxt);
  static std::string create_context_json(const std::string& cntxt, const std::string& action);
  static std::string create_context_json(const std::string& cntxt, const std::string& action, const std::string& slots);

  /**
   * @brief Create a event_id used to match choose_rank and report_outcome()
   *
   * @return std::string
   */
  std::string create_event_id();

  /**
   * @brief Pick a person from the list of people.  Use uniform random.
   *
   * @return person&
   */
  person& pick_a_random_person();

  /**
   * @brief Pick a robotic joint from the list of joints.  Use uniform random.
   *
   * @return joint&
   */
  joint& pick_a_random_joint();

  /**
   * @brief Load Inference API configuration from json string.
   *
   * @param str json string
   * @param config Configuration object used by Inference API
   * @param status api_status object for error feedback
   * @return int Error status
   */
  static int load_config_from_json(const std::string& file_name, reinforcement_learning::utility::configuration& config,
      reinforcement_learning::api_status* status);

  /**
   * @brief Load the contents of a file in to a std::string
   *
   * @param file_name File name
   * @param config_str String to hold the data
   * @param status api_status object for error feedback
   * @return int Error status
   */
  static int load_file(
      const std::string& file_name, std::string& config_str, reinforcement_learning::api_status* status);

  /**
   * @brief Initialize Inference API
   *
   * @return int Error status
   */
  int init_rl();

  /**
   * @brief Initialize collection of people
   *
   * @return true If there is no error during init
   * @return false On init error
   */
  bool init_sim_world();

  /**
   * @brief Initialize robot joints for continuous action space
   *
   * @return true If there is no error during init
   * @return false On init error
   */
  bool init_continuous_sim_world();

  /**
   * @brief Initialize the simulator
   *
   * @return true on success
   * @return false on failure
   */
  bool init();

  int cb_loop();
  int ca_loop();
  int ccb_loop();
  int slates_loop();
  int multistep_loop();

  /**
   * @brief Get the action features as a json string
   *
   * @return std::string
   */
  std::string get_action_features();

  /**
   * @brief Get slates action features as a json string
   *
   * @return std::string
   */
  std::string get_slates_action_features();

  std::string get_slot_features();

private:
  enum LoopKind
  {
    CB,
    CCB,
    Slates,
    CA,
    Multistep
  };

  boost::program_options::variables_map _options;
  std::unique_ptr<reinforcement_learning::live_model> _rl;
  std::vector<person> _people;
  std::vector<std::string> _topics;
  std::vector<std::string> _slot_sizes;
  std::vector<joint> _robot_joints;
  std::vector<float> _friction;
  const uint8_t NUM_SLOTS = 3;
  const uint8_t NUM_SLATES_SLOTS = 2;
  bool _run_loop = true;
  LoopKind _loop_kind;
  int _num_events = 0;
  int _current_events = 0;
  uint64_t _random_seed = 0;
  int64_t _delay = 2000;
  bool _quiet = false;
  bool _random_ids = true;
#ifdef LINK_AZURE_LIBS
  AzureCredentials _creds;
#endif
};
