#include <benchmark/benchmark.h>
#include <iostream>
#include <algorithm>

#include "api_status.h"
#include "config_utility.h"
#include "constants.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "live_model.h"
#include "model_mgmt.h"
#include "ranking_response.h"

#include "benchmarks_common.h"

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
static void bench_cb(benchmark::State &state, ExtraArgs &&... extra_args) {

  int res[sizeof...(extra_args)] = {extra_args...};
  auto shared_features = res[0];
  auto action_features = res[1];
  auto actions_per_decision = res[2];
  auto total_actions = res[3];
  auto count = res[4];
  bool compression = res[5];
  bool dedup = res[6];

  cb_decision_gen cb_gen(shared_features, action_features, actions_per_decision,
                         total_actions, 0, false);

  std::vector<std::string> examples;
  std::generate_n(std::back_inserter(examples), count,
                  [&cb_gen] { return cb_gen.gen_example(); });

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION,
             r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION,
             r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "/dev/null");
  config.set(r::name::OBSERVATION_FILE_NAME, "/dev/null");
  config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");
  // config.set(r::name::MODEL_IMPLEMENTATION, r::value::PASSTHROUGH_PDF_MODEL);
  config.set(r::name::VW_POOL_INIT_SIZE, "1");
  config.set(r::name::INTERACTION_USE_COMPRESSION,
             compression ? "true" : "false");
  config.set(r::name::INTERACTION_USE_DEDUP, dedup ? "true" : "false");
  config.set("queue.mode", "BLOCK");

  r::api_status status;
  r::live_model model(config);
  model.init(&status);
  const auto event_id = "event_id";

  r::ranking_response response;

  for (auto _ : state) {
    for (size_t i = 0; i < count; i++) {
      if (model.choose_rank(event_id, examples[i].c_str(), response,
                            &status) != err::success) {
        std::cout << "there was an error so something went wrong during "
                     "benchmarking: "
                  << status.get_error_msg() << std::endl;
      }
    }
    benchmark::ClobberMemory();
  }
}

// x shared features
// x features per action
// x actions per example
// x actions in total
// example count
// compression
// dedup
BENCHMARK_CAPTURE(bench_cb, non_dedupable_payload, 20, 10, 50, 2000, 500, false,
                  false);
BENCHMARK_CAPTURE(bench_cb, non_dedupable_payload_compression, 20, 10, 50, 2000,
                  500, true, false);
BENCHMARK_CAPTURE(bench_cb, non_dedupable_payload_dedup, 20, 10, 50, 2000, 500,
                  false, true);
BENCHMARK_CAPTURE(bench_cb, non_dedupable_payload_compression_dedup, 20, 10, 50,
                  2000, 500, true, true);

BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload, 20, 400, 10, 50, 500,
                  false, false);
BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload_compression, 20, 400,
                  10, 50, 500, true, false);
BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload_dedup, 20, 400, 10, 50,
                  500, false, true);
BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload_compression_dedup, 20,
                  400, 10, 50, 500, true, true);

BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload, 20, 100, 10, 30, 500,
                  false, false);
BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload_compression, 20, 100, 10,
                  30, 500, true, false);
BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload_dedup, 20, 100, 10, 30,
                  500, false, true);
BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload_compression_dedup, 20, 100,
                  10, 30, 500, true, true);