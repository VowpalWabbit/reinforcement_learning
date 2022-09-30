#include "api_status.h"
#include "benchmarks_common.h"
#include "config_utility.h"
#include "constants.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "live_model.h"
#include "model_mgmt.h"
#include "ranking_response.h"
#include "vw/core/parse_example_json.h"
#include "vw/core/parser.h"
#include "vw/core/rand48.h"
#include "vw/core/vw.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <iostream>
#include <thread>

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
static void bench_cb(benchmark::State& state, const std::string& cmd, ExtraArgs&&... extra_args)
{
  int res[sizeof...(extra_args)] = {extra_args...};
  auto shared_features = res[0];
  auto action_features = res[1];
  auto actions_per_decision = res[2];
  auto total_actions = res[3];
  auto count = res[4];
  bool compression = res[5];
  bool dedup = res[6];

  cb_decision_gen cb_gen(shared_features, action_features, actions_per_decision, total_actions, 0, false);

  std::vector<std::string> examples;
  std::generate_n(std::back_inserter(examples), count, [&cb_gen] { return cb_gen.gen_example(); });

  u::configuration config;
  cfg::create_from_json(JSON_CFG, config);
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, "/dev/null");
  config.set(r::name::OBSERVATION_FILE_NAME, "/dev/null");
  config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");
  // config.set(r::name::MODEL_IMPLEMENTATION, r::value::PASSTHROUGH_PDF_MODEL);
  config.set(r::name::VW_POOL_INIT_SIZE, "1");
  config.set(r::name::INTERACTION_USE_COMPRESSION, compression ? "true" : "false");
  config.set(r::name::INTERACTION_USE_DEDUP, dedup ? "true" : "false");
  config.set("queue.mode", "BLOCK");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, cmd.c_str());

  r::api_status status;
  r::live_model model(config);
  model.init(&status);
  const auto event_id = "event_id";
  size_t action_id = 0;

  r::ranking_response response;

  for (auto _ : state)
  {
    if (model.choose_rank(event_id, examples[0].c_str(), response, &status) != err::success)
    {
      std::cout << "there was an error so something went wrong during "
                   "benchmarking: "
                << status.get_error_msg() << std::endl;
    }
    benchmark::DoNotOptimize(response.get_chosen_action_id(action_id));
    benchmark::ClobberMemory();
  }
}

// characteristics of the benchmark examples that will be generated are:

// x shared features
// x features per action (affects dedup-ness)
// x actions per example
// x actions in total (affects dedup-ness)
// x number of total examples so x number of total rank calls to benchmark
// compression (on/off)
// dedup (on/off)

// actions: 300

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_300_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 300, 300, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_300_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    300, 300, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_300_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 300, 300, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_300_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 300, 300, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_300_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 300, 300, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// actions: 400

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_400_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 400, 400, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_400_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    400, 400, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_400_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 400, 400, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_400_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 400, 400, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_400_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 400, 400, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);


BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_600_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 600, 600, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_600_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    600, 600, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_600_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 600, 600, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_600_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 600, 600, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_600_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 600, 600, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);


BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_700_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 700, 700, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_700_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    700, 700, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_700_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 700, 700, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_700_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 700, 700, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_700_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 700, 700, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);


BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_800_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 800, 800, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_800_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    800, 800, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_800_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 800, 800, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_800_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 800, 800, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_800_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 800, 800, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);


BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_900_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 900, 900, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_900_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    900, 900, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_900_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 900, 900, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_900_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 900, 900, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_900_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 900, 900, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);


// actions: 500

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_500_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    10, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_500_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 10,
    500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_500_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 10, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_500_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 10, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_500_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 10, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// =======

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_500_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    20, 500, 500, 1, false, false)
    // ->UseRealTime()
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_500_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 20,
    500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_500_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 20, 500, 500, 1, false, false, 20)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_500_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 20, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_small_500_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 20, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// =======

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_500_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    30, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_500_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 30,
    500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_500_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 30, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_500_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 30, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_small_500_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 30, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// ===

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_500_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    40, 500, 500, 1, false, false)
    // ->UseRealTime()
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_500_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 40,
    500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_500_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 40, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_500_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 40, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_small_500_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 40, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// ===

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_500_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 500, 500, 1, false, false)
    // ->UseRealTime()
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_500_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_500_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_500_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_500_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 500, 500, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// actions: 1000

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_1000_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    10, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_1000_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 10,
    1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_1000_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 10, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_1000_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 10, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_10af_1000_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 10, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// =======

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_1000_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    20, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_1000_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 20,
    1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_1000_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 20, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_1000_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 20, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_20af_small_1000_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 20, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// =======

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_1000_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    30, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_1000_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 30,
    1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_1000_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 30, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_1000_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 30, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_30af_small_1000_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 30, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// ===

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_1000_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    40, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_1000_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 40,
    1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_1000_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 40, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_1000_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 40, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_40af_small_1000_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 40, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// ===

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_1000_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_1000_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_1000_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_1000_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_1000_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 1000, 1000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// == 2000 actions

BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_2000_actions_plaincb, "--cb_explore_adf --json --quiet -q :: --id N/A", 50,
    50, 2000, 2000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_2000_actions_20_max_actions_nothreading,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size 0", 50, 50,
    2000, 2000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_2000_actions_20_max_actions_max_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency()),
    50, 50, 2000, 2000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_2000_actions_20_max_actions_half_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 2),
    50, 50, 2000, 2000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(bench_cb, cb_las_50s_50af_small_2000_actions_20_max_actions_quarter_threads,
    "--cb_explore_adf --json --quiet -q :: --id N/A --large_action_space --max_actions 20 --thread_pool_size " +
        std::to_string(std::thread::hardware_concurrency() / 4),
    50, 50, 2000, 2000, 1, false, false)
    ->MinTime(15.0)
    ->Unit(benchmark::kMillisecond);

// BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload, 20, 400, 10, 50, 1, false, false)
//     ->MinTime(15.0)
//     ->UseRealTime()
//     ->Unit(benchmark::kMillisecond);

// BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload, 20, 100, 10, 30, 1, false, false)
//     ->MinTime(15.0)
//     ->UseRealTime()
//     ->Unit(benchmark::kMillisecond);

// BENCHMARK_CAPTURE(bench_cb, non_dedupable_payload, 20, 10, 50, 2000, 500, false, false);
// BENCHMARK_CAPTURE(bench_cb, non_dedupable_payload_compression, 20, 10, 50, 2000, 500, true, false);
// BENCHMARK_CAPTURE(bench_cb, non_dedupable_payload_dedup, 20, 10, 50, 2000, 500, false, true);
// BENCHMARK_CAPTURE(bench_cb, non_dedupable_payload_compression_dedup, 20, 10, 50, 2000, 500, true, true);

// BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload, 20, 400, 10, 50, 500, false, false);
// BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload_compression, 20, 400, 10, 50, 500, true, false);
// BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload_dedup, 20, 400, 10, 50, 500, false, true);
// BENCHMARK_CAPTURE(bench_cb, extremeley_dedupable_payload_compression_dedup, 20, 400, 10, 50, 500, true, true);

// BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload, 20, 100, 10, 30, 500, false, false);
// BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload_compression, 20, 100, 10, 30, 500, true, false);
// BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload_dedup, 20, 100, 10, 30, 500, false, true);
// BENCHMARK_CAPTURE(bench_cb, normal_dedupable_payload_compression_dedup, 20, 100, 10, 30, 500, true, true);