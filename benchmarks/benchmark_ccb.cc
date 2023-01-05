#include "api_status.h"
#include "benchmarks_common.h"
#include "config_utility.h"
#include "constants.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "live_model.h"
#include "model_mgmt.h"
#include "ranking_response.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iostream>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace m = reinforcement_learning::model_management;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

const auto JSON_CFG = R"(
{
  "ApplicationID": "rnc-123456-a",
  "IsExplorationEnabled": true,
  "InitialExplorationEpsilon": 1.0
}
)";

template <class... ExtraArgs>
static void bench_ccb(benchmark::State& state, ExtraArgs&&... extra_args)
{
  int res[sizeof...(extra_args)] = {extra_args...};
  auto count = res[0];
  auto shared_features_size = res[1];
  auto shared_features_count = res[2];
  auto action_features_size = res[3];
  auto action_features_count = res[4];
  auto actions_per_example = res[5];
  auto slots_per_example = res[6];
  auto total_actions = res[7];
  bool compression = res[8];
  bool dedup = res[9];

  ccb_decision_gen ccb_gen(shared_features_size, shared_features_count, action_features_size, action_features_count,
      actions_per_example, slots_per_example, total_actions, 0);

  std::vector<std::string> examples;
  std::generate_n(std::back_inserter(examples), count, [&ccb_gen] { return ccb_gen.gen_example(); });

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --quiet -q ::");
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "/dev/null");
  config.set(r::name::OBSERVATION_FILE_NAME, "/dev/null");
  config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");
  config.set(r::name::VW_POOL_INIT_SIZE, "1");
  config.set(r::name::INTERACTION_USE_COMPRESSION, compression ? "true" : "false");
  config.set(r::name::INTERACTION_USE_DEDUP, dedup ? "true" : "false");
  config.set("queue.mode", "BLOCK");

  r::api_status status;
  r::live_model model(config);
  model.init(&status);
  const auto event_id = "event_id";

  r::ranking_response response;

  for (auto _ : state)
  {
    for (size_t i = 0; i < count; i++)
    {
      if (model.choose_rank(event_id, examples[i].c_str(), response, &status) != err::success)
      {
        std::cout << "there was an error so something went wrong during "
                     "benchmarking: "
                  << status.get_error_msg() << std::endl;
      }
    }
    benchmark::ClobberMemory();
  }
}

BENCHMARK_CAPTURE(bench_ccb, ccb_adf_diff_char_interactions_predict,
    10,     // number of examples
    30,     // shared_feats_size
    20,     // shared_feats_count (actual number of shared features in example)
    30,     // action_feats_size
    20,     // action_feats_count
    30,     // actions_per_example
    20,     // slots_per_example
    2000,   // total actions to choose from
    false,  // compression
    false   // dedup
);
