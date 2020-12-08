#include <iostream>
#include <fstream>
#include <cstring>
#include <boost/program_options.hpp>

#include "config_utility.h"
#include "live_model.h"
#include "constants.h"

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
  "ccb",
  "slates",
  "ca",
  "f-reward",
  "fi-reward",
  "fs-reward",
  "s-reward",
  "si-reward",
  "ss-reward",
  "action-taken",
  nullptr
};

enum options{
  CB_ACTION,
  CCB_ACTION,
  SLATES_ACTION,
  CA_ACTION,
  F_REWARD,
  F_I_REWARD,
  F_S_REWARD,
  S_REWARD,
  S_I_REWARD,
  S_S_REWARD,
  ACTION_TAKEN,
};

void load_config_from_json(int action, u::configuration& config)
{
  std::string file_name(options[action]);
  file_name += "_v2.fb";

  config.set("ApplicationID", "<appid>");
  config.set("interaction.sender.implementation", "INTERACTION_FILE_SENDER");
  config.set("observation.sender.implementation", "OBSERVATION_FILE_SENDER");
  config.set("decisions.sender.implementation", "INTERACTION_FILE_SENDER");
  config.set("model.source", "NO_MODEL_DATA");

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

  if(enable_dedup)
    config.set(nm::INTERACTION_CONTENT_ENCODING, val::CONTENT_ENCODING_ZSTD_AND_DEDUP);

  if(action == CCB_ACTION) {
    config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  } else if (action == SLATES_ACTION) {
    config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  }
  else if (action == CA_ACTION)
  {
    config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--cats 4 --min_value 1 --max_value 100 --bandwidth 1 --json --quiet --id N/A");
  }
}

const auto JSON_CB_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}]})";

const auto JSON_CCB_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"}},{"TAction":{"a2":"f2"}}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a1":"f1"}}]})";

const auto JSON_SLATES_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"},"_slot_id":0},{"TAction":{"a2":"f2"},"_slot_id":0},{"TAction":{"a3":"f3"},"_slot_id":1},{"TAction":{"a4":"f4"},"_slot_id":1},{"TAction":{"a5":"f5"},"_slot_id":1}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a2":"f2"}}]})";

const auto JSON_CA_CONTEXT = R"({"RobotJoint1":{"friction":78}})";

int run_config(int action) {
  u::configuration config;

  load_config_from_json(action, config);

  r::api_status status;
  r::live_model rl(config);
  if( rl.init(&status) != err::success ) {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  switch(action) {
    case CB_ACTION: {// "cb",
      r::ranking_response response;
      rl.choose_rank("event_id", JSON_CB_CONTEXT, response, &status);
      break;
    }
    case CCB_ACTION: {// "ccb",
      r::multi_slot_response response;
      if(rl.request_multi_slot_decision("event_id", JSON_CCB_CONTEXT, 0, response, &status) != err::success)
          std::cout << status.get_error_msg() << std::endl;
      break;
    };
    case SLATES_ACTION: {// "slates",
      r::multi_slot_response response;
      if(rl.request_multi_slot_decision("event_id", JSON_SLATES_CONTEXT, response, &status) != err::success)
          std::cout << status.get_error_msg() << std::endl;
      break;
    };
    case CA_ACTION: {// "ca",
      r::continuous_action_response response;
      if(rl.request_continuous_action("event_id", JSON_CA_CONTEXT, 0, response, &status) != err::success)
          std::cout << status.get_error_msg() << std::endl;
      break;
    };
    case F_REWARD: // "float"
      if( rl.report_outcome("event_id", 1.5, &status) != err::success ) 
          std::cout << status.get_error_msg() << std::endl;
      break;

    case F_I_REWARD: // "float-int",
      if( rl.report_outcome("event_id", 10, 1.5, &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case F_S_REWARD: // "float-string"
      if( rl.report_outcome("event_id", "index_id", 1.5, &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case S_REWARD: // "string-reward",
        if( rl.report_outcome("event_id", "reward-str", &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case S_I_REWARD: // "string-int-reward",
        if( rl.report_outcome("event_id", 10, "reward-str", &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case S_S_REWARD: // "string-string-reward",
        if( rl.report_outcome("event_id", "index_id", "reward-str", &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    case ACTION_TAKEN:
        if( rl.report_action_taken("event_id", &status) != err::success )
          std::cout << status.get_error_msg() << std::endl;
      break;
    default:
      std::cout << "Invalid action " << action << std::endl;
      return -1;
  }
  return 0;
}

int main(int argc, char *argv[]) {
  po::options_description desc("example-gen");
  std::string action_name;
  bool gen_all = false;

  desc.add_options()
    ("help", "Produce help message")
    ("all", "use all args")
    ("dedup", "Enable dedup/zstd")
    ("kind", po::value<std::string>(), "which kind of example to generate (cb,ccb,slates,ca,(f|s)(s|i)?-reward,action-taken)");

  po::positional_options_description pd;
  pd.add("kind", 1);

  po::variables_map vm;
  try {
    store(po::command_line_parser(argc, argv).options(desc).positional(pd).run(), vm);
    po::notify(vm);
    gen_all = vm.count("all");
    enable_dedup = vm.count("dedup");
    if(vm.count("kind") > 0)
      action_name = vm["kind"].as<std::string>();
  } catch(std::exception& e) {
    std::cout << e.what() << std::endl;
    std::cout << desc << std::endl;
    return 0;
  }

  if(vm.count("help") > 0 || (action_name.empty() && !gen_all)) {
    std::cout << desc << std::endl;
    return 0;
  }

  if(gen_all) {
    for(int i = 0; options[i]; ++i) {
      if(run_config(i))
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

  return run_config(action);
}