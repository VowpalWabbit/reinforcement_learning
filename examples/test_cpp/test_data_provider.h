#pragma once

#include "live_model.h"
#include "ranking_response.h"

#include <fstream>
#include <string>
#include <vector>

class test_data_provider
{
public:
  test_data_provider(std::string experiment_name, size_t threads, size_t features, size_t actions, size_t slots,
      bool _is_float_outcome, size_t _reward_period);

  std::string create_event_id(size_t thread_id, size_t example_id) const;
  std::vector<std::string> create_event_ids(size_t thread_id, size_t example_id) const;

  const char* get_context(size_t thread_id, size_t example_id) const;
  std::string get_context(size_t thread_id, size_t example_id, const std::vector<std::string>& event_ids) const;

  float get_outcome(size_t thread_id, size_t example_id) const;
  const char* get_outcome_json(size_t thread_id, size_t example_id) const;

  bool is_rewarded(size_t thread_id, size_t example_id) const;

  int report_outcome(reinforcement_learning::live_model* rl, size_t thread_id, size_t example_id,
      reinforcement_learning::api_status* status) const;

private:
  static std::string create_action_features(size_t actions, size_t features, size_t example_id);
  static std::string create_slot_features(size_t slots, size_t features, size_t slot_decision_id);
  static std::string create_features(size_t features, size_t thread_id, size_t example_id);
  static std::string create_context_json(const std::string& cntxt, const std::string& action, bool ccb);
  std::string create_json_outcome(size_t thread_id, size_t example_id) const;
  static std::string create_slots_json(const std::vector<std::string>& ids);
  static std::string create_ccb_context_json(const std::string& cntxt, const std::vector<std::string>& ids);

private:
  static const size_t preallocated_count = 100;

private:
  const std::string _experiment_name;
  std::vector<std::vector<std::string>> contexts;
  std::vector<std::vector<std::string>> outcomes;
  bool is_float_outcome;
  size_t reward_period;
  size_t slots;
};
