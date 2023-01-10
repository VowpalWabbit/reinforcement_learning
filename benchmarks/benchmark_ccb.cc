#include "api_status.h"
#include "benchmarks_common.h"
#include "config_utility.h"
#include "constants.h"
#include "err_constants.h"
#include "factory_resolver.h"
#include "live_model.h"
#include "model_mgmt.h"
#include "multi_slot_response.h"
#include "vw/config/options_cli.h"
#include "vw/core/parse_example_json.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"

#include <benchmark/benchmark.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>

namespace r = reinforcement_learning;
namespace u = reinforcement_learning::utility;
namespace m = reinforcement_learning::model_management;
namespace err = reinforcement_learning::error_code;
namespace cfg = reinforcement_learning::utility::config;

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

  // generate examples
  ccb_decision_gen ccb_gen(shared_features_size, shared_features_count, action_features_size, action_features_count,
      actions_per_example, slots_per_example, total_actions, 0);
  std::vector<std::string> examples;
  std::generate_n(std::back_inserter(examples), count, [&ccb_gen] { return ccb_gen.gen_example(); });

  // set up rlclientlib config
  u::configuration config;
  config.set(r::name::APP_ID, "bench_ccb");
  config.set(r::name::INITIAL_EPSILON, "0");
  config.set(r::name::MODEL_VW_INITIAL_COMMAND_LINE, "--ccb_explore_adf --quiet -q ::");
  config.set(r::name::PROTOCOL_VERSION, "2");
  config.set(r::name::EH_TEST, "true");
  config.set(r::name::MODEL_SRC, r::value::FILE_MODEL_DATA);
  config.set(r::name::MODEL_FILE_MUST_EXIST, "true");
  config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
  config.set(r::name::INTERACTION_FILE_NAME, r::DEV_NULL);
  config.set(r::name::OBSERVATION_FILE_NAME, r::DEV_NULL);
  config.set(r::name::MODEL_BACKGROUND_REFRESH, "false");
  config.set(r::name::VW_POOL_INIT_SIZE, "1");
  config.set(r::name::INTERACTION_USE_COMPRESSION, compression ? "true" : "false");
  config.set(r::name::INTERACTION_USE_DEDUP, dedup ? "true" : "false");
  config.set("queue.mode", "BLOCK");

  std::string model_output_filename = std::tmpnam(nullptr);
  config.set(r::name::MODEL_FILE_NAME, model_output_filename.c_str());

  // train a VW model and save to file
  {
    std::string command_line = config.get(r::name::MODEL_VW_INITIAL_COMMAND_LINE, nullptr);
    auto opts =
        std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(VW::split_command_line(command_line)));
    auto vw = VW::initialize_experimental(std::move(opts));

    std::vector<VW::multi_ex> examples_vec;
    for (auto&& str : examples)
    {
      VW::multi_ex ex;
      ex.push_back(VW::new_unused_example(*vw));
      line_to_examples_json<false>(vw.get(), str.c_str(), str.size(), ex);
      examples_vec.push_back(ex);
    }

    for (auto&& ex : examples_vec)
    {
      VW::setup_examples(*vw, ex);
      vw->learn(ex);
      vw->finish_example(ex);
    }

    io_buf io_buffer;
    auto backing_buffer = std::make_shared<std::vector<char>>();
    io_buffer.add_file(VW::io::create_vector_writer(backing_buffer));
    VW::save_predictor(*vw, io_buffer);
    std::fstream out_file(model_output_filename, std::ios::out | std::ios::binary);
    out_file.write(backing_buffer->data(), backing_buffer->size());
  }

  // initialize live_model
  r::api_status status;
  r::live_model model(config);
  model.init(&status);

  // run benchmark
  r::multi_slot_response response;
  for (auto _ : state)
  {
    for (size_t i = 0; i < count; i++)
    {
      if (model.request_multi_slot_decision(examples[i].c_str(), response, &status) != err::success)
      {
        std::cout << "there was an error so something went wrong during "
                     "benchmarking: "
                  << status.get_error_msg() << std::endl;
      }
    }
    benchmark::ClobberMemory();
  }

  // delete model file
  std::remove(model_output_filename.c_str());
}

BENCHMARK_CAPTURE(bench_ccb, ccb_adf_diff_char_interactions_predict,
    1,      // number of examples
    30,     // shared_feats_size
    20,     // shared_feats_count (actual number of shared features in example)
    30,     // action_feats_size
    20,     // action_feats_count
    30,     // actions_per_example
    20,     // slots_per_example
    2000,   // total actions to choose from
    false,  // compression
    false   // dedup
    )
    ->Unit(benchmark::kMillisecond);
