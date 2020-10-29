#include "test_loop.h"
#include "ranking_event.h"
#include "data_buffer.h"
#include "config_utility.h"
#include <fstream>
#include <iostream>
#include <thread>
#include "serialization/fb_serializer.h"

namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;
namespace po = boost::program_options;
namespace chrono = std::chrono;

test_loop::test_loop(size_t index, const boost::program_options::variables_map& vm)
  : threads(vm["threads"].as<size_t>())
  , controller(experiment_controller_factory::create(vm))
  , experiment_name(generate_experiment_name(vm["experiment_name"].as<std::string>(), threads, vm["features"].as<size_t>(), vm["actions"].as<size_t>(), vm["slots"].as<size_t>(), index))
  , json_config(vm["json_config"].as<std::string>())
  , test_inputs(experiment_name, threads, vm["features"].as<size_t>(), vm["actions"].as<size_t>(), vm["slots"].as<size_t>(), vm.count("float_outcome") > 0, vm["reward_period"].as<size_t>())
  , sleep_interval(vm["sleep"].as<size_t>())
{
  for (size_t i = 0; i < threads; ++i) {
    loggers.push_back(std::make_shared<std::ofstream>(experiment_name + "." + std::to_string(i), std::ofstream::out));
  }
}

void _on_error(const reinforcement_learning::api_status& status, void* nothing) {
  std::cerr << status.get_error_msg() << std::endl;
}

bool test_loop::init() {
  r::api_status status;
  u::configuration config;

  if (load_config_from_json(json_config, config, &status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }

  rl = std::unique_ptr<r::live_model>(new r::live_model(config, _on_error, nullptr));
  if (rl->init(&status) != err::success) {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }

  return true;
}

int test_loop::load_config_from_json(const std::string& file_name,
  u::configuration& config,
  r::api_status* status) const {
  std::string config_str;
  RETURN_IF_FAIL(load_file(file_name, config_str));
  return cfg::create_from_json(config_str, config, nullptr, status);
}

int test_loop::load_file(const std::string& file_name, std::string& config_str) const {
  std::ifstream fs;
  fs.open(file_name);
  if (!fs.good()) return err::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return err::success;
}

std::string test_loop::generate_experiment_name(const std::string& experiment_name_base, size_t threads, size_t features, size_t actions, size_t slots, size_t index)
{
  return experiment_name_base + "-t" + std::to_string(threads) + "-f" + std::to_string(features) + "-a" + std::to_string(actions) + "-s" + std::to_string(slots) + "-i" + std::to_string(index);
}

void test_loop::run(bool is_ccb) {
  std::vector<std::thread> _threads;
  for (size_t i = 0; i < threads; ++i) {
    _threads.push_back(is_ccb ? std::thread(&test_loop::ccb_loop, this, i) : std::thread(&test_loop::cb_loop, this, i));
  }
  for (size_t i = 0; i < threads; ++i) {
    _threads[i].join();
  }
}

using namespace reinforcement_learning;
using namespace reinforcement_learning::logger;

void test_loop::cb_loop(size_t thread_id)
{
  r::ranking_response response;
  r::api_status status;
  std::cout << "Warmup..." << std::endl;
  size_t choose_rank_size = 0;
  {
    const auto event_id = test_inputs.create_event_id(0, 0);
    const std::string warmup_id = "_warmup_" + std::string(event_id.c_str());
    if (rl->choose_rank(warmup_id.c_str(), test_inputs.get_context(0, 0), response, &status) != err::success) {
      std::cout << "Warmup has failed. " << status.get_error_msg() << std::endl;
      return;
    }

    r::utility::data_buffer buffer;
    fb_collection_serializer<ranking_event> serializer(buffer, r::content_encoding_enum::IDENTITY);
    auto choose_rank_event = r::ranking_event::choose_rank(warmup_id.c_str(), test_inputs.get_context(0, 0), r::action_flags::DEFAULT, response, timestamp{});
    serializer.add(choose_rank_event);
    serializer.finalize();
    choose_rank_size = buffer.body_filled_size();
    std::cout << "Choose rank size: " << choose_rank_size << std::endl;
  }  

  std::cout << "Perf test is started..." << std::endl;
  std::cout << "Choose_rank..." << std::endl;
  const auto choose_rank_start = chrono::high_resolution_clock::now();
  for (controller->restart(); controller->is_running(); controller->iterate()) {
    if (thread_id == 0) controller->progress_bar();
    const auto event_id = test_inputs.create_event_id(thread_id, controller->get_iteration());

    if (rl->choose_rank(event_id.c_str(), test_inputs.get_context(thread_id, controller->get_iteration()), response, &status) != err::success) {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    if (test_inputs.is_rewarded(thread_id, controller->get_iteration())) {
      if (test_inputs.report_outcome(rl.get(), thread_id, controller->get_iteration(), &status) != err::success) {
        std::cout << status.get_error_msg() << std::endl;
        continue;
      }
    }
  }
  if (thread_id == 0) controller->progress_bar();
  std::cout << std::endl;

  const auto choose_rank_end = chrono::high_resolution_clock::now();

  const auto choose_rank_perf = (chrono::duration_cast<chrono::microseconds>(choose_rank_end - choose_rank_start).count()) / controller->get_iteration();
  (*loggers[thread_id]) << thread_id << ": Choose_rank: " << choose_rank_perf << " microseconds" << std::endl;
}

std::vector<const char*> to_const_char(const std::vector<std::string>& ids) {
  std::vector<const char*> result;
  for (const auto& id : ids) {
    result.push_back(id.c_str());
  }
  return result;
}

void test_loop::ccb_loop(size_t thread_id)
{
  r::decision_response response;
  r::api_status status;
  std::cout << "Warmup..." << std::endl;
  size_t choose_rank_size = 0;
  {
    const auto event_ids = test_inputs.create_event_ids(0, 0);
    const auto event_ids_c = to_const_char(event_ids);
    const std::string warmup_id = "_warmup_" + std::string(event_ids[0].c_str());
    const auto context = test_inputs.get_context(0, 0, event_ids);
    if (rl->request_decision(context.c_str(), response, &status) != err::success) {
      std::cout << "Warmup is failed. " << status.get_error_msg() << std::endl;
      return;
    }

    r::utility::data_buffer buffer;
    fb_collection_serializer<decision_ranking_event> serializer(buffer, r::content_encoding_enum::IDENTITY);
    const std::vector<std::vector<uint32_t>> blank_action_ids(response.size());
    const std::vector<std::vector<float>> blank_pdf(response.size());
    auto decision_event = r::decision_ranking_event::request_decision(event_ids_c, context.c_str(), r::action_flags::DEFAULT, blank_action_ids, blank_pdf, "model", timestamp{});
    serializer.add(decision_event);
    serializer.finalize();
    choose_rank_size = buffer.body_filled_size();
    std::cout << "Decision event size: " << choose_rank_size << std::endl;
  }

  std::cout << "Perf test is started..." << std::endl;
  std::cout << "Choose_rank..." << std::endl;
  const auto choose_rank_start = chrono::high_resolution_clock::now();
  for (controller->restart(); controller->is_running(); controller->iterate()) {
    if (thread_id == 0) controller->progress_bar();
    const auto event_ids = test_inputs.create_event_ids(thread_id, controller->get_iteration());
    const auto event_ids_c = to_const_char(event_ids);
    const auto context = test_inputs.get_context(thread_id, controller->get_iteration(), event_ids);
    if (rl->request_decision(context.c_str(), response, &status) != err::success) {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    if (test_inputs.is_rewarded(thread_id, controller->get_iteration())) {

      if (rl->report_outcome(event_ids[0].c_str(), 1, &status) != err::success) {
        std::cout << status.get_error_msg() << std::endl;
        continue;
      }
    }
  }
  if (thread_id == 0) controller->progress_bar();
  std::cout << std::endl;

  const auto choose_rank_end = chrono::high_resolution_clock::now();

  const auto choose_rank_perf = (chrono::duration_cast<chrono::microseconds>(choose_rank_end - choose_rank_start).count()) / controller->get_iteration();
  (*loggers[thread_id]) << thread_id << ": Choose_rank: " << choose_rank_perf << " microseconds" << std::endl;
}
