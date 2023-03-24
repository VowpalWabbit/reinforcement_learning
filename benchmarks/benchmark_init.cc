#include "benchmark_common.h"
#include "config_utility.h"
#include "constants.h"
#include "err_constants.h"
#include "cb_loop.h"
#include "model_mgmt.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iostream>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
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
static void bench_init(benchmark::State& state, ExtraArgs&&... extra_args)
{
  int res[sizeof...(extra_args)] = {extra_args...};
  auto load_model = res[0];
  auto model_id = res[1];

  std::ostringstream model_path;
  model_path << "benchmarks/models/";
  // Note: models generated using '--cb_explore_adf -b 18'
  switch (model_id)
  {
    case 0:
      model_path << "cb_explore_adf_small.m";
      break;
    case 1:
      model_path << "cb_explore_adf_half.m";
      break;
    case 2:
      model_path << "cb_explore_adf_large.m";
      break;
    default:
      std::cout << "invalid model id" << std::endl;
      break;
  }

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, r::DEV_NULL);
  config.set(r::name::OBSERVATION_FILE_NAME, r::DEV_NULL);
  config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");
  config.set(r::name::VW_POOL_INIT_SIZE, "1");
  config.set(r::name::INTERACTION_USE_COMPRESSION, "false");
  config.set(r::name::INTERACTION_USE_DEDUP, "false");
  config.set("queue.mode", "BLOCK");

  if (load_model)
  {
    config.set(r::name::MODEL_SRC, "FILE_MODEL_DATA");
    config.set(r::name::MODEL_FILE_NAME, model_path.str().c_str());
  }
  else { config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA); }

  for (auto _ : state)
  {
    r::api_status status;
    r::cb_loop model(config);
    model.init(&status);
    if (status.get_error_code() != err::success)
    {
      std::cout << "there was an error so something went wrong during "
                   "benchmarking: "
                << status.get_error_msg() << std::endl;
    }
    benchmark::ClobberMemory();
  }
}

// characteristics of the benchmark examples that will be generated are:
// x load model (on/off)
// x model path

BENCHMARK_CAPTURE(bench_init, no_model, false, 0)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_init, small_model, true, 0)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_init, half_model, true, 1)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_init, large_model, true, 2)->Unit(benchmark::kMillisecond);
