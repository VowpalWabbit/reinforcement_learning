#pragma once
#include "live_model.h"
#include "ranking_response.h"

#include <fstream>
#include <string>
#include <vector>

class test_data_provider {
public:
  test_data_provider(const std::string& experiment_name, size_t threads, size_t features, size_t actions, bool _is_float_outcome, size_t reward_ratio);

  std::string create_event_id(size_t thread_id, size_t example_id) const;

  const char* get_context(size_t thread_id, size_t example_id) const;

  float get_outcome(size_t thread_id, size_t example_id) const;
  const char* get_outcome_json(size_t thread_id, size_t example_id) const;

  bool is_rewarded(size_t thread_id, size_t example_id) const;

  int report_outcome(reinforcement_learning::live_model* rl, size_t thread_id, size_t example_id, reinforcement_learning::api_status* status) const;

  void log(size_t thread_id, size_t example_id, const reinforcement_learning::ranking_response& response, std::ostream& logger) const;

private:
  std::string create_action_features(size_t actions, size_t features, size_t example_id) const;
  std::string create_features(size_t features, size_t thread_id, size_t example_id) const;
  std::string create_context_json(const std::string& cntxt, const std::string& action) const;
  std::string create_json_outcome(size_t thread_id, size_t example_id) const;

private:
  static const size_t preallocated_count = 100;

private:
  const std::string _experiment_name;
  std::vector<std::vector<std::string>> contexts;
  std::vector<std::vector<std::string>> outcomes;
  bool is_float_outcome;
  size_t reward_period;
};
