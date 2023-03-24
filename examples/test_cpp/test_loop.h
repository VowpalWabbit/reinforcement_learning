#pragma once

#include "experiment_controller.h"
#include "cb_loop.h"
#include "ccb_loop.h"
#include "episodic_loop.h"
#include "test_data_provider.h"

#include <boost/program_options.hpp>
#include <memory>

enum class LoopKind
{
  CB,
  CCB,
  MULTISTEP,
};

class test_loop
{
public:
  test_loop(size_t index, const boost::program_options::variables_map& vm);
  bool init();
  void run() const;

private:
  static int load_file(const std::string& file_name, std::string& config_str);
  static int load_config_from_json(const std::string& file_name, reinforcement_learning::utility::configuration& config,
      reinforcement_learning::api_status* status);
  static std::string generate_experiment_name(const std::string& experiment_name_base, size_t threads, size_t features,
      size_t actions, size_t slots, size_t episode_length, size_t index);
  static LoopKind get_loop_kind(const boost::program_options::variables_map& vm);

  void cb_loop(size_t thread_id) const;
  void ccb_loop(size_t thread_id) const;
  void multistep_loop(size_t thread_id) const;

private:
  const LoopKind loop_kind;
  const size_t threads;
  const size_t sleep_interval;
  const size_t episode_length;
  const std::string experiment_name;
  const std::string json_config;
  test_data_provider test_inputs;
  std::vector<std::unique_ptr<experiment_controller>> controllers;
  std::vector<std::shared_ptr<std::ofstream>> loggers;
  std::unique_ptr<reinforcement_learning::cb_loop> rl_cb;
  std::unique_ptr<reinforcement_learning::ccb_loop> rl_ccb;
  std::unique_ptr<reinforcement_learning::episodic_loop> rl_episodic;
};
