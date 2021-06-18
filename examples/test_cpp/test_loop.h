#pragma once

#include <memory>
#include <boost/program_options.hpp>

#include "experiment_controller.h"
#include "test_data_provider.h"
#include "live_model.h"

class test_loop {
public:
  test_loop(size_t index, const boost::program_options::variables_map& vm);
  bool init();
  void run(bool is_ccb) const;

private:
  int load_file(const std::string& file_name, std::string& config_str) const;
  int load_config_from_json(const std::string& file_name,
    reinforcement_learning::utility::configuration& config,
    reinforcement_learning::api_status* status) const;
  std::string generate_experiment_name(const std::string& experiment_name_base,
	  size_t threads, size_t features, size_t actions, size_t slots, size_t index) const;

  void cb_loop(size_t thread_id) const;
  void ccb_loop(size_t thread_id) const;

private:
  const size_t threads;
  const size_t sleep_interval;
  const std::string experiment_name;
  const std::string json_config;
  test_data_provider test_inputs;
  std::vector<std::unique_ptr<experiment_controller>> controllers;
  std::vector<std::shared_ptr<std::ofstream>> loggers;
  std::unique_ptr<reinforcement_learning::live_model> rl;
};
