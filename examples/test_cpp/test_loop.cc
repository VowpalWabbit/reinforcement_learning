#include "test_loop.h"

#include "config_utility.h"
#include "constants.h"
#include "data_buffer.h"
#include "options.h"
#include "ranking_event.h"
#include "serialization/fb_serializer.h"

#include <fstream>
#include <iostream>
#include <thread>

namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;
namespace po = boost::program_options;
namespace chrono = std::chrono;

test_loop::test_loop(size_t index, const boost::program_options::variables_map& vm)
    : loop_kind(get_loop_kind(vm))
    , threads(vm["threads"].as<size_t>())
    , experiment_name(
          generate_experiment_name(vm["experiment_name"].as<std::string>(), threads, vm["features"].as<size_t>(),
              vm["actions"].as<size_t>(), vm["slots"].as<size_t>(), vm["episode_length"].as<size_t>(), index))
    , json_config(vm["json_config"].as<std::string>())
    , test_inputs(experiment_name, threads, vm["features"].as<size_t>(), vm["actions"].as<size_t>(),
          vm["slots"].as<size_t>(), vm.count("float_outcome") > 0, vm["reward_period"].as<size_t>())
    , episode_length(vm["episode_length"].as<size_t>())
    , sleep_interval(vm["sleep"].as<size_t>())
{
  for (size_t i = 0; i < threads; ++i)
  {
    controllers.push_back(std::unique_ptr<experiment_controller>(experiment_controller_factory::create(vm)));
    loggers.push_back(std::make_shared<std::ofstream>(experiment_name + "." + std::to_string(i), std::ofstream::out));
  }
}

void _on_error(const r::api_status& status, void* nothing) { std::cerr << status.get_error_msg() << std::endl; }

bool test_loop::init()
{
  r::api_status status;
  u::configuration config;

  if (load_config_from_json(json_config, config, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }

  rl = std::unique_ptr<r::live_model>(new r::live_model(config, _on_error, nullptr));
  if (rl->init(&status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return false;
  }

  return true;
}

int test_loop::load_config_from_json(
    const std::string& file_name, u::configuration& config, r::api_status* status) const
{
  std::string config_str;
  RETURN_IF_FAIL(load_file(file_name, config_str));
  return cfg::create_from_json(config_str, config, nullptr, status);
}

int test_loop::load_file(const std::string& file_name, std::string& config_str) const
{
  std::ifstream fs;
  fs.open(file_name);
  if (!fs.good()) return err::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return err::success;
}

std::string test_loop::generate_experiment_name(const std::string& experiment_name_base, size_t threads,
    size_t features, size_t actions, size_t slots, size_t episode_length, size_t index) const
{
  return experiment_name_base + "-t" + std::to_string(threads) + "-x" + std::to_string(features) + "-a" +
      std::to_string(actions) + "-s" + std::to_string(slots) + "-m" + std::to_string(episode_length) + "-i" +
      std::to_string(index);
}

LoopKind test_loop::get_loop_kind(const boost::program_options::variables_map& vm) const
{
  throw_if_conflicting(vm, "episode_length", "slots");
  if (vm["episode_length"].as<size_t>() > 0) return LoopKind::MULTISTEP;
  if (vm["slots"].as<size_t>() > 0) return LoopKind::CCB;
  return LoopKind::CB;
}

void test_loop::run() const
{
  void (test_loop::*loop)(size_t) const;
  switch (loop_kind)
  {
    case LoopKind::CB:
    {
      loop = &test_loop::cb_loop;
      break;
    }
    case LoopKind::CCB:
    {
      loop = &test_loop::ccb_loop;
      break;
    }
    case LoopKind::MULTISTEP:
    {
      loop = &test_loop::multistep_loop;
      break;
    }
  }

  std::vector<std::thread> _threads;
  for (size_t i = 0; i < threads; ++i) { _threads.push_back(std::thread(loop, this, i)); }
  for (size_t i = 0; i < threads; ++i) { _threads[i].join(); }
}

// TODO: why do we need this warmup?
void test_loop::cb_loop(size_t thread_id) const
{
  r::ranking_response response;
  r::api_status status;
  std::cout << "Warmup..." << std::endl;
  size_t choose_rank_size = 0;
  {
    const auto event_id = test_inputs.create_event_id(0, 0);
    const std::string warmup_id = "_warmup_" + std::string(event_id.c_str());
    if (rl->choose_rank(warmup_id.c_str(), test_inputs.get_context(0, 0), response, &status) != err::success)
    {
      std::cout << "Warmup has failed. " << status.get_error_msg() << std::endl;
      return;
    }

    r::utility::data_buffer buffer;
    r::logger::fb_collection_serializer<r::ranking_event> serializer(buffer, r::value::CONTENT_ENCODING_IDENTITY);
    auto choose_rank_event = r::ranking_event::choose_rank(
        warmup_id.c_str(), test_inputs.get_context(0, 0), r::action_flags::DEFAULT, response, r::timestamp{});
    serializer.add(choose_rank_event);
    serializer.finalize(nullptr);
    choose_rank_size = buffer.body_filled_size();
    std::cout << "Choose rank size: " << choose_rank_size << std::endl;
  }

  auto controller = controllers[thread_id].get();
  auto& logger = *loggers[thread_id];

  std::cout << "Perf test is started..." << std::endl;
  std::cout << "Choose_rank..." << std::endl;
  const auto choose_rank_start = chrono::high_resolution_clock::now();
  for (controller->restart(); controller->is_running(); controller->iterate())
  {
    if (thread_id == 0) controller->show_progress_bar();

    const auto example_id = controller->get_iteration();
    const auto event_id = test_inputs.create_event_id(thread_id, example_id);

    if (rl->choose_rank(event_id.c_str(), test_inputs.get_context(thread_id, example_id), response, &status) !=
        err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    if (test_inputs.is_rewarded(thread_id, example_id))
    {
      if (test_inputs.report_outcome(rl.get(), thread_id, example_id, &status) != err::success)
      {
        std::cout << status.get_error_msg() << std::endl;
        continue;
      }
    }
  }
  if (thread_id == 0) controller->show_progress_bar();
  std::cout << std::endl;

  const auto choose_rank_end = chrono::high_resolution_clock::now();

  const auto choose_rank_perf =
      (chrono::duration_cast<chrono::microseconds>(choose_rank_end - choose_rank_start).count()) /
      controller->get_iteration();
  logger << thread_id << ": Choose_rank: " << choose_rank_perf << " microseconds" << std::endl;
}

std::vector<const char*> to_const_char(const std::vector<std::string>& ids)
{
  std::vector<const char*> result;
  for (const auto& id : ids) { result.push_back(id.c_str()); }
  return result;
}

void test_loop::ccb_loop(size_t thread_id) const
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
    if (rl->request_decision(context.c_str(), response, &status) != err::success)
    {
      std::cout << "Warmup is failed. " << status.get_error_msg() << std::endl;
      return;
    }

    r::utility::data_buffer buffer;
    r::logger::fb_collection_serializer<r::decision_ranking_event> serializer(
        buffer, r::value::CONTENT_ENCODING_IDENTITY);
    const std::vector<std::vector<uint32_t>> blank_action_ids(response.size());
    const std::vector<std::vector<float>> blank_pdf(response.size());
    auto decision_event = r::decision_ranking_event::request_decision(
        event_ids_c, context.c_str(), r::action_flags::DEFAULT, blank_action_ids, blank_pdf, "model", r::timestamp{});
    serializer.add(decision_event);
    serializer.finalize(nullptr);
    choose_rank_size = buffer.body_filled_size();
    std::cout << "Decision event size: " << choose_rank_size << std::endl;
  }

  auto controller = controllers[thread_id].get();
  auto& logger = *loggers[thread_id];

  std::cout << "Perf test is started..." << std::endl;
  std::cout << "Choose_rank..." << std::endl;
  const auto choose_rank_start = chrono::high_resolution_clock::now();
  for (controller->restart(); controller->is_running(); controller->iterate())
  {
    if (thread_id == 0) controller->show_progress_bar();

    const auto example_id = controller->get_iteration();
    const auto event_ids = test_inputs.create_event_ids(thread_id, example_id);
    const auto event_ids_c = to_const_char(event_ids);
    const auto context = test_inputs.get_context(thread_id, example_id, event_ids);
    if (rl->request_decision(context.c_str(), response, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    if (test_inputs.is_rewarded(thread_id, example_id))
    {
      // TODO: why a fixed reward is used here (probably it doesn't matter for perf)
      if (rl->report_outcome(event_ids[0].c_str(), 1, &status) != err::success)
      {
        std::cout << status.get_error_msg() << std::endl;
        continue;
      }
    }
  }
  if (thread_id == 0) controller->show_progress_bar();
  std::cout << std::endl;

  const auto choose_rank_end = chrono::high_resolution_clock::now();

  const auto choose_rank_perf =
      (chrono::duration_cast<chrono::microseconds>(choose_rank_end - choose_rank_start).count()) /
      controller->get_iteration();
  logger << thread_id << ": Choose_rank: " << choose_rank_perf << " microseconds" << std::endl;
}

void test_loop::multistep_loop(size_t thread_id) const
{
  r::ranking_response response;
  r::api_status status;
  std::cout << "Warmup..." << std::endl;
  size_t choose_rank_size = 0;
  {
    r::episode_state episode("warmup_episode");
    const auto event_id = test_inputs.create_event_id(0, 0);
    const std::string warmup_id = "_warmup_" + std::string(event_id.c_str());
    if (rl->request_episodic_decision(
            warmup_id.c_str(), nullptr, test_inputs.get_context(0, 0), response, episode, &status) != err::success)
    {
      std::cout << "Warmup has failed. " << status.get_error_msg() << std::endl;
      return;
    }

    // TODO: Add serializable request_episodic_decision event.
  }

  auto controller = controllers[thread_id].get();
  auto& logger = *loggers[thread_id];

  size_t episode_id = 0;
  std::unique_ptr<r::episode_state> current_episode = nullptr;
  std::string previous_event_id("");

  std::cout << "Perf test is started..." << std::endl;
  std::cout << "Choose_rank..." << std::endl;
  const auto choose_rank_start = chrono::high_resolution_clock::now();
  for (controller->restart(); controller->is_running(); controller->iterate())
  {
    if (thread_id == 0) controller->show_progress_bar();

    const auto example_id = controller->get_iteration();
    const auto event_id = test_inputs.create_event_id(thread_id, example_id);

    if (example_id % episode_length == 0)
    {
      ++episode_id;
      const auto episode_id_str = std::string("episode") + std::to_string(episode_id);
      current_episode.reset(new r::episode_state(episode_id_str.c_str()));
      previous_event_id = "";
    }

    if (rl->request_episodic_decision(event_id.c_str(), previous_event_id.empty() ? nullptr : previous_event_id.c_str(),
            test_inputs.get_context(thread_id, example_id), response, *current_episode, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    if (test_inputs.is_rewarded(thread_id, example_id))
    {
      float reward = test_inputs.get_outcome(thread_id, example_id);
      if (rl->report_outcome(current_episode->get_episode_id(), event_id.c_str(), reward, &status) != err::success)
      {
        std::cout << status.get_error_msg() << std::endl;
        continue;
      }
    }

    // TODO: This makes a chain of events. Add other types of event relationships.
    previous_event_id = event_id;
  }
  if (thread_id == 0) controller->show_progress_bar();
  std::cout << std::endl;

  const auto choose_rank_end = chrono::high_resolution_clock::now();

  const auto choose_rank_perf =
      (chrono::duration_cast<chrono::microseconds>(choose_rank_end - choose_rank_start).count()) /
      controller->get_iteration();
  logger << thread_id << ": Choose_rank: " << choose_rank_perf << " microseconds" << std::endl;
}
