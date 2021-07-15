#include <iostream>
#include <fstream>
#include <cstring>
#include <boost/program_options.hpp>
#include <random>

#include "config_utility.h"
#include "live_model.h"
#include "constants.h"
#include "action_flags.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace m = reinforcement_learning::model_management;
namespace nm = reinforcement_learning::name;
namespace val = reinforcement_learning::value;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;
namespace po = boost::program_options;

//global var, yeah ugg
bool enable_dedup = false;

static const char *options[] = {
  "cb",
  "invalid-cb",
  "ccb",
  "ccb-with-slot-id",
  "ccb-baseline",
  "slates",
  "ca",
  "f-reward",
  "fi-reward",
  "fi-out-of-bound-reward",
  "fs-reward",
  "fmix-reward",
  "s-reward",
  "si-reward",
  "ss-reward",
  "action-taken",
  "cb-loop",
  nullptr
};

enum options{
  CB_ACTION,
  INVALID_CB_ACTION,
  CCB_ACTION,
  CCB_WITH_SLOT_ID_ACTION,
  CCB_BASELINE_ACTION,
  SLATES_ACTION,
  CA_ACTION,
  // Put interaction actions before F_REWARD
  F_REWARD,
  F_I_REWARD,
  F_I_OUT_OF_BOUND_REWARD,
  F_S_REWARD,
  F_MIX_REWARD,
  S_REWARD,
  S_I_REWARD,
  S_S_REWARD,
  ACTION_TAKEN,
  CB_LOOP
};

void load_config_from_json(int action, u::configuration& config, bool enable_apprentice_mode)
{
  std::string file_name(options[action]);
  file_name += "_v2.fb";

  config.set("ApplicationID", "<appid>");
  config.set("interaction.sender.implementation", "INTERACTION_FILE_SENDER");
  config.set("observation.sender.implementation", "OBSERVATION_FILE_SENDER");
  config.set("decisions.sender.implementation", "INTERACTION_FILE_SENDER");
  config.set("model.source", "NO_MODEL_DATA");

  if (enable_apprentice_mode) {
    config.set("rank.learning.mode", "APPRENTICE");
  }

  bool is_observation = action >= F_REWARD;
  if(is_observation) {
    config.set("observation.file.name", file_name.c_str());
    config.set("interaction.file.name", "/dev/null");
  } else {
    config.set("observation.file.name", "/dev/null");
    config.set("interaction.file.name", file_name.c_str());
  }
  config.set("protocol.version", "2");
  config.set("InitialExplorationEpsilon", "1.0");

  if(enable_dedup) {
    config.set(nm::INTERACTION_USE_DEDUP, "true");
    config.set(nm::INTERACTION_USE_COMPRESSION, "true");
  }

  if(action == CCB_ACTION || action == CCB_BASELINE_ACTION || action == CCB_WITH_SLOT_ID_ACTION) {
    config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  } else if (action == SLATES_ACTION) {
    config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  }
  else if (action == CA_ACTION)
  {
    config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--cats 4 --min_value 1 --max_value 100 --bandwidth 1 --json --quiet --id N/A");
  }
}

bool load_config_from_provided_json(const std::string& config_file, u::configuration& config)
{
  std::string config_str;
	  // Load contents of config file into a string
    std::ifstream fs;
    fs.open(config_file);
    if (!fs.good()) {
      std::cout << "could load config file: " << config_file << std::endl;
      return false;
    }
    std::stringstream buffer;
    buffer << fs.rdbuf();
    config_str = buffer.str();

	  if(cfg::create_from_json(config_str, config) != reinforcement_learning::error_code::success)
    {
      std::cout << "could not create configuration from config file: " << config_file << std::endl;
      return false;
    }

    // yes could be set in the config file but cli overrides that
    if (enable_dedup) {
      std::cout << "enabling dedup" << std::endl;
      config.set(nm::INTERACTION_USE_DEDUP, "true");
      config.set(nm::INTERACTION_USE_COMPRESSION, "true");
    }

    return true;
}

const auto JSON_CB_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}]})";

const auto JSON_CCB_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a1":"f1"}}]})";

const auto JSON_CCB_WITH_SLOT_ID_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}],"_slots":[{"Slot":{"a1":"f1"}, "_id": "slot_0"},{"Slot":{"a1":"f1"}, "_id":"slot_1"}]})";

const auto JSON_SLATES_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"},"_slot_id":0},{"TAction":{"a2":"f2"},"_slot_id":0},{"TAction":{"a3":"f3"},"_slot_id":1},{"TAction":{"a4":"f4"},"_slot_id":1},{"TAction":{"a5":"f5"},"_slot_id":1}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a2":"f2"}}]})";

const auto JSON_CA_CONTEXT = R"({"RobotJoint1":{"friction":78}})";

float get_random_number(std::mt19937& rng, int min = 1) {
  std::uniform_int_distribution<int> uni(min, 5); // guaranteed unbiased
  auto random_integer = uni(rng);
  return random_integer;
}

int take_action(r::live_model& rl, const char *event_id, int action, unsigned int action_flag, bool gen_random_reward, std::mt19937& rng) {
  r::api_status status;
  float reward = gen_random_reward ? get_random_number(rng) : 1.5f;

  switch(action) {
    case CB_ACTION: {// "cb",
      r::ranking_response response;
      if(rl.choose_rank(event_id, JSON_CB_CONTEXT, action_flag, response, &status))
          std::cout << status.get_error_msg() << std::endl;
      break;
    }
    case INVALID_CB_ACTION: {// "cb invalid",
      r::ranking_response response;
      // call choose rank but with slates context
      if(rl.choose_rank(event_id, JSON_SLATES_CONTEXT, action_flag, response, &status))
          std::cout << status.get_error_msg() << std::endl;
      break;
    }
    case CCB_ACTION: {// "ccb",
      r::multi_slot_response response;
      if(rl.request_multi_slot_decision(event_id, JSON_CCB_CONTEXT, action_flag, response, &status) != err::success)
          std::cout << status.get_error_msg() << std::endl;
      break;
    };
    case CCB_WITH_SLOT_ID_ACTION: {// "ccb-with-slot-id",
      r::multi_slot_response response;
      if(rl.request_multi_slot_decision(event_id, JSON_CCB_WITH_SLOT_ID_CONTEXT, action_flag, response, &status) != err::success)
          std::cout << status.get_error_msg() << std::endl;
      break;
    };
    case SLATES_ACTION: {// "slates",
      r::multi_slot_response response;
      if(rl.request_multi_slot_decision(event_id, JSON_SLATES_CONTEXT, action_flag, response, &status) != err::success)
          std::cout << status.get_error_msg() << std::endl;
      break;
    };
    case CA_ACTION: {// "ca",
      r::continuous_action_response response;
      if(rl.request_continuous_action(event_id, JSON_CA_CONTEXT, action_flag, response, &status) != err::success)
          std::cout << status.get_error_msg() << std::endl;
      break;
    };
    case F_REWARD: // "float"
      if( rl.report_outcome(event_id, reward, &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case F_I_REWARD: // "float-int",
      {
        size_t num_of_rewards = 4;
        for (size_t i = 0; i < num_of_rewards; i++)
        {
          float reward_0 = gen_random_reward ? get_random_number(rng, 0) : 1.5f;
          float reward_1 = gen_random_reward ? get_random_number(rng, 0) : 1.5f;

          if (rl.report_outcome(event_id, 0, reward_0, &status) != err::success) {
            std::cout << status.get_error_msg() << std::endl;
          }
          if( rl.report_outcome(event_id, 1, reward_1, &status) != err::success ) {
            std::cout << status.get_error_msg() << std::endl;
          }
        }
      }
      break;
    case F_I_OUT_OF_BOUND_REWARD:
      if (rl.report_outcome(event_id, 1000, 1.5, &status) != err::success) {
        std::cout << status.get_error_msg() << std::endl;
      }
      break;
    case F_S_REWARD: // "float-string"
      {
        size_t num_of_rewards = 4;
        for (size_t i = 0; i < num_of_rewards; i++)
        {
          float reward_0 = gen_random_reward ? get_random_number(rng, 0) : 1.5f;
          float reward_1 = gen_random_reward ? get_random_number(rng, 0) : 1.5f;

          if (rl.report_outcome(event_id, "slot_0", reward_0, &status) != err::success) {
            std::cout << status.get_error_msg() << std::endl;
          }
          if( rl.report_outcome(event_id, "slot_1", reward_1, &status) != err::success ) {
            std::cout << status.get_error_msg() << std::endl;
          }
        }
      }
      break;
    case F_MIX_REWARD: // "float-i and float-s mixed"
      {
        std::vector<std::string> slot_ids {"slot_0", "slot_1"};
        size_t num_of_rewards = 2;

        for (size_t i = 0; i < slot_ids.size(); i++) {
          for (size_t j = 0; j < num_of_rewards; j++) {
            float reward_0 = gen_random_reward ? get_random_number(rng, 0) : 1.5f;
            float reward_1 = gen_random_reward ? get_random_number(rng, 0) : 1.5f;

            if (rl.report_outcome(event_id, i, reward_0, &status) != err::success) {
              std::cout << status.get_error_msg() << std::endl;
            }

            if (rl.report_outcome(event_id, slot_ids[i].c_str(), reward_1, &status) != err::success) {
              std::cout << status.get_error_msg() << std::endl;
            }
          }
        }
      }
      break;
    case S_REWARD: // "string-reward",
        if( rl.report_outcome(event_id, "reward-str", &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case S_I_REWARD: // "string-int-reward",
        if( rl.report_outcome(event_id, 1, "reward-str", &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case S_S_REWARD: // "string-string-reward",
        if( rl.report_outcome(event_id, "index_id", "reward-str", &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case ACTION_TAKEN:
        if( rl.report_action_taken(event_id, &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case CCB_BASELINE_ACTION: {// "ccb",
      r::multi_slot_response response;
      std::vector<int> baselines { 1, 0 };
      if(rl.request_multi_slot_decision(event_id, JSON_CCB_CONTEXT, 0, response, &baselines[0], 2, &status) != err::success)
          std::cout << status.get_error_msg() << std::endl;
      break;
    };
    case CB_LOOP: { // "cb action and random number of float rewards"
      r::ranking_response response;
      std::cout << "choose rank for event: " << event_id << std::endl;
      if(rl.choose_rank(event_id, JSON_CB_CONTEXT, action_flag, response, &status))
          std::cout << status.get_error_msg() << std::endl;

      size_t num_of_rewards = get_random_number(rng);
      for (size_t i = 0; i < num_of_rewards; i++)
      {
        float reward = gen_random_reward ? get_random_number(rng, 0) : 1.5f;
        std::cout << "report outcome: " << reward << " for event: " << event_id << std::endl;
        if( rl.report_outcome(event_id, reward, &status) != err::success )
            std::cout << status.get_error_msg() << std::endl;
      }
      break;
    };

    default:
      std::cout << "Invalid action " << action << std::endl;
      return -1;
  }
  return 0;
}

// We use this instead of rand to ensure it's xplat
int pseudo_random(int seed) {
  constexpr uint64_t CONSTANT_A = 0xeece66d5deece66dULL;
  constexpr uint64_t CONSTANT_C = 2147483647;

  uint64_t val = CONSTANT_A * seed + CONSTANT_C;
  return (int)(val & 0xFFFFFFFF);
}

int run_config(int action, int count, int initial_seed, bool gen_random_reward, bool enable_apprentice_mode, int deferred_action_count, std::string config_file, std::mt19937& rng) {
  u::configuration config;

  if (config_file.empty())
  {
    load_config_from_json(action, config, enable_apprentice_mode);
  }
  else
  {
    if (!load_config_from_provided_json(config_file, config))
    {
      return -1;
    }
  }

  r::api_status status;
  r::live_model rl(config);

  if( rl.init(&status) != err::success ) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  for(int i = 0; i < count; ++i) {
    char event_id[128];
    if(initial_seed == -1)
      strcpy(event_id, "abcdefghijklm");
    else
      sprintf(event_id, "%x", pseudo_random(initial_seed + i * 997739));

    auto action_flag = i < deferred_action_count
      ? r::action_flags::DEFERRED : r::action_flags::DEFAULT;

    int r = take_action(rl, event_id, action, action_flag, gen_random_reward, rng);
    if(r)
      return r;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  po::options_description desc("example-gen");
  std::string action_name;
  std::string config_file;
  bool gen_all = false;
  int count = 1;
  int seed = 473747277; //much random
  bool gen_random_reward = false;
  bool enable_apprentice_mode = false;
  int deferred_action_count = 0;

  desc.add_options()
    ("help", "Produce help message")
    ("all", "use all args")
    ("dedup", "Enable dedup/zstd")
    ("count", po::value<int>(), "Number of events to produce")
    ("seed", po::value<int>(), "Initial seed used to produce event ids")
    ("kind", po::value<std::string>(), "which kind of example to generate (cb,invalid-cb,ccb,ccb-with-slot-id,ccb-baseline,slates,ca,cb-loop,(f|s)(s|i|mix|i-out-of-bound)?-reward,action-taken)")
    ("random_reward", "Generate random float reward for observation event")
    ("config_file", po::value<std::string>(), "json config file for rlclinetlib")
    ("apprentice", "Enable apprentice mode for cb event")
    ("deferred_action_count",  po::value<int>(), "Number of deferred action for interaction events. Set the deferred_action flag to true for first deferred_action_count number of actions");

  po::positional_options_description pd;
  pd.add("kind", 1);

  po::variables_map vm;
  try {
    store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
    po::notify(vm);
    gen_all = vm.count("all");
    gen_random_reward = vm.count("random_reward");
    enable_apprentice_mode = vm.count("apprentice");
    enable_dedup = vm.count("dedup");

    std::vector<std::string> deferrable_interactions {
      "cb", "invalid-cb", "ccb", "ccb-baseline", "slates", "ca", "cb-loop",
      "ccb-with-slot-id"
    };

    if(vm.count("kind") > 0)
      action_name = vm["kind"].as<std::string>();
    if(vm.count("count") > 0)
      count = vm["count"].as<int>();
    if(vm.count("seed") > 0)
      seed = vm["seed"].as<int>();
    if(vm.count("config_file") > 0)
      config_file = vm["config_file"].as<std::string>();
    if(vm.count("deferred_action_count") > 0)
      deferred_action_count = vm["deferred_action_count"].as<int>();

    if(vm.count("deferred_action_count") > 0 && !std::any_of(
      deferrable_interactions.begin(),
      deferrable_interactions.end(),
      [action_name](std::string evt_type) {return action_name == evt_type;}
    )) {
      throw std::runtime_error("'--deferred_action' should be used with interaction event");
    }
  } catch(std::exception& e) {
    std::cout << e.what() << std::endl;
    std::cout << desc << std::endl;
    return 0;
  }

  std::random_device rd;     // only used once to initialise (seed) engine
  std::mt19937 rng(seed);    // random-number engine used (Mersenne-Twister in this case)

  if(vm.count("help") > 0 || (action_name.empty() && !gen_all)) {
    std::cout << desc << std::endl;
    return 0;
  }

  if(gen_all) {
    for(int i = 0; options[i]; ++i) {
      if(run_config(i, count, seed, gen_random_reward, enable_apprentice_mode, deferred_action_count, config_file, rng))
        return -1;
    }
    return 0;
  }

  int action = -1;
  for (int i = 0; options[i]; ++i) {
    if(action_name == options[i]) {
      action = i;
      break;
    }
  }

  if(action == -1) {
    std::cout << "Invalid action: " << action_name << std::endl;
    std::cout << desc << std::endl;
    return -1;
  }

  return run_config(action, count, seed, gen_random_reward, enable_apprentice_mode, deferred_action_count, config_file, rng);
}