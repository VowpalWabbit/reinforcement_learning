#include "api_status.h"
#include "benchmark_common.h"
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
static void bench_init(benchmark::State& state, ExtraArgs&&... extra_args)
{
  int res[sizeof...(extra_args)] = {extra_args...};
  auto load_model = res[0];
  auto model_id = res[1];

  std::string model_path = "benchmarks/models/";
  switch (model_id)
  {
    case 0:
      model_path += "cb_small.m";
      break;
    case 1:
      model_path += "cb_half.m";
      break;
    case 2:
      model_path += "cb_large.m";
      break;
    default:
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
    config.set(r::name::MODEL_FILE_NAME, model_path.c_str());
  }
  else
  {
    config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  }

  r::api_status status;
  r::live_model model(config);
  model.init(&status);
  const auto event_id = "event_id";

  for (auto _ : state)
  {
    r::api_status status;
    r::live_model model(config);
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
