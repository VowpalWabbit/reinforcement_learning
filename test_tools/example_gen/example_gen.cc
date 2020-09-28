/**
 * @brief Simple RL Inference API sample implementation
 *
 * @file basic_usage_cpp.cc
 * @author Rajan Chari et al
 * @date 2018-07-15
 */

#include <iostream>
#include <fstream>
 #include <cstring>
 #include "config_utility.h"
#include "live_model.h"
#include "constants.h"

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace m = reinforcement_learning::model_management;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

static const char *options[] = {
  "cb",
  "ccb",
  "slates",
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
  file_name += "_v2.fbs";


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

  if(action == CCB_ACTION) {
    config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  } else if (action == SLATES_ACTION) {
    config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--slates --ccb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A");
  }
}

const auto JSON_CB_CONTEXT = R"({ "GUser":{"id":"a","major":"eng","hobby":"hiking"}, "_multi":[ { "TAction":{"a1":"f1"} },{"TAction":{"a2":"f2"}}]})";

const auto JSON_CCB_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[ { "TAction":{"a1":"f1"} },{"TAction":{"a2":"f2"}}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a1":"f1"}}]})";

const auto JSON_SLATES_CONTEXT = R"({"GUser":{"id":"a","major":"eng","hobby":"hiking"},"_multi":[{"TAction":{"a1":"f1"},"_slot_id":0},{"TAction":{"a2":"f2"},"_slot_id":0},{"TAction":{"a3":"f3"},"_slot_id":1},{"TAction":{"a4":"f4"},"_slot_id":1},{"TAction":{"a5":"f5"},"_slot_id":1}],"_slots":[{"Slot":{"a1":"f1"}},{"Slot":{"a2":"f2"}}]})";


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
    case F_REWARD: // "float"
      if( rl.report_outcome("event_id", 1.5, &status) != err::success ) 
          std::cout << status.get_error_msg() << std::endl;
      break;

    case F_I_REWARD: // "float-int",
      if( rl.report_outcome("event_id", 1.5, 10, &status) != err::success ) 
          std::cout << status.get_error_msg() << std::endl;
      break;
    case F_S_REWARD: // "float-string"
      if( rl.report_outcome("event_id", 1.5, "index_id", &status) != err::success ) 
          std::cout << status.get_error_msg() << std::endl;
      break;
    case S_REWARD: // "string-reward",
        if( rl.report_outcome("event_id", "reward-str", &status) != err::success ) 
          std::cout << status.get_error_msg() << std::endl;
      break;
    case S_I_REWARD: // "string-int",
        if( rl.report_outcome("event_id", "reward-str", 10, &status) != err::success ) 
          std::cout << status.get_error_msg() << std::endl;
      break;
    case S_S_REWARD: // "string-string",
        if( rl.report_outcome("event_id", "reward-str", "index_id", &status) != err::success ) 
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
    if(argc == 1) {
      printf("usage:\n");
      printf("\texample_gen action|--all\n");
      return -1;
    }

  if(!strcmp("--all", argv[1])) {
    for(int i = 0; options[i]; ++i) {
      if(run_config(i))
        return -1;
    }
    return 0;
  }

  int action = -1;
  for (int i = 0; options[i]; ++i) {
    if(!strcmp(options[i], argv[1])) {
      action = i;
      break;
    }
  }

  if(action == -1) {
    std::cout << "Invalid action " << std::endl;
    return -1;
  }

  return run_config(action);
}