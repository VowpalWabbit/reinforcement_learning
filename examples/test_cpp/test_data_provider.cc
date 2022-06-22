#include "test_data_provider.h"

#include "data_buffer.h"
#include "ranking_event.h"
#include "serialization/json_serializer.h"

#include <sstream>
#include <utility>

test_data_provider::test_data_provider(std::string experiment_name, size_t threads, size_t features, size_t actions,
    size_t _slots, bool _is_float_outcome, size_t _reward_period)
    : _experiment_name(std::move(experiment_name))
    , contexts(threads, std::vector<std::string>(preallocated_count))
    , outcomes(threads, std::vector<std::string>(preallocated_count))
    , is_float_outcome(_is_float_outcome)
    , reward_period(_reward_period)
    , slots(_slots)
{
  for (size_t t = 0; t < threads; ++t)
  {
    for (size_t i = 0; i < preallocated_count; ++i)
    {
      contexts[t][i] = create_context_json(
          create_features(features, t, i), create_action_features(actions, features, i), _slots > 0);
      outcomes[t][i] = create_json_outcome(t, i);
    }
  }
}

// TODO: stringstream is the slowest way to concat strings.
std::string test_data_provider::create_event_id(size_t thread_id, size_t example_id) const
{
  std::ostringstream oss;
  oss << _experiment_name << "-" << thread_id << "-" << example_id;
  return oss.str();
}

std::vector<std::string> test_data_provider::create_event_ids(size_t thread_id, size_t example_id) const
{
  std::ostringstream oss;
  oss << _experiment_name << "-" << thread_id << "-" << example_id;
  std::string base = oss.str();
  std::vector<std::string> result;
  for (size_t i = 0; i < slots; ++i)
  {
    std::ostringstream id;
    id << base << "-" << i;
    result.push_back(id.str());
  }
  return result;
}

std::string test_data_provider::create_action_features(size_t actions, size_t features, size_t example_id)
{
  std::ostringstream oss;
  oss << R"("_multi": [ )";
  for (size_t a = 0; a < actions; ++a)
  {
    oss << R"({ "TAction":{)";
    for (size_t f = 0; f < features; ++f)
    {
      oss << R"("a_f_)" << f << R"(":"value_)" << (a + f + example_id) << R"(")";
      if (f + 1 < features) { oss << ","; }
    }
    oss << "}}";
    if (a + 1 < actions) { oss << ","; }
  }
  oss << R"(])";
  return oss.str();
}

std::string test_data_provider::create_slot_features(size_t slots, size_t features, size_t slot_decision_id)
{
  std::ostringstream oss;
  oss << R"("_slots": [ )";
  for (size_t a = 0; a < slots; ++a)
  {
    oss << R"({ "TSlot":{)";
    oss << R"("_id":")" << slot_decision_id << R"(")";
    if (features > 0) { oss << ","; }
    for (size_t f = 0; f < features; ++f)
    {
      oss << R"("a_f_)" << f << R"(":"value_)" << (a + f + slot_decision_id) << R"(")";
      if (f + 1 < features) { oss << ","; }
    }
    oss << "}}";
    if (a + 1 < slots) { oss << ","; }
  }
  oss << R"(])";
  return oss.str();
}

std::string test_data_provider::create_features(size_t features, size_t thread_id, size_t example_id)
{
  std::ostringstream oss;
  oss << R"("GUser":{)";
  oss << R"("f_int":)" << example_id << R"(,)";
  oss << R"("f_float":)" << float(example_id) + 0.5 << R"(,)";
  for (size_t f = 0; f < features; ++f)
  {
    oss << R"("f_str_)" << f << R"(":"value_)" << (f + thread_id + example_id) << R"(")";
    if (f + 1 < features) { oss << ","; }
  }
  oss << R"(})";
  return oss.str();
}

std::string test_data_provider::create_json_outcome(size_t thread_id, size_t example_id) const
{
  std::ostringstream oss;
  oss << R"({"Reward":)" << get_outcome(thread_id, example_id) << R"(,"CustomRewardField":)"
      << get_outcome(thread_id, example_id) + 1 << "}";
  return oss.str();
}

std::string test_data_provider::create_context_json(const std::string& cntxt, const std::string& action, bool ccb)
{
  std::ostringstream oss;
  if (ccb) { oss << cntxt << ", " << action; }
  else
  {
    oss << "{ " << cntxt << ", " << action << " }";
  }

  return oss.str();
}

std::string test_data_provider::create_ccb_context_json(const std::string& cntxt, const std::vector<std::string>& ids)
{
  std::ostringstream oss;
  oss << "{ " << cntxt << ", " << create_slots_json(ids) << " }";
  return oss.str();
}

std::string test_data_provider::create_slots_json(const std::vector<std::string>& ids)
{
  std::ostringstream oss;
  oss << R"("_slots": [ )";
  for (size_t a = 0; a < ids.size(); ++a)
  {
    oss << R"({ )";  // "TSlot":{)";
    oss << R"("_id":")" << ids[a] << R"(")";
    oss << "}";  //}";
    if (a + 1 < ids.size()) { oss << ","; }
  }
  oss << R"(])";
  return oss.str();
}

std::string test_data_provider::get_context(
    size_t thread_id, size_t example_id, const std::vector<std::string>& event_ids) const
{
  return create_ccb_context_json(contexts[thread_id][example_id % preallocated_count], event_ids);
}

float test_data_provider::get_outcome(size_t thread_id, size_t example_id) const
{
  return is_rewarded(thread_id, example_id) ? static_cast<float>(thread_id + example_id) : 0.f;
}

const char* test_data_provider::get_outcome_json(size_t thread_id, size_t example_id) const
{
  return outcomes[thread_id][example_id % preallocated_count].c_str();
}

const char* test_data_provider::get_context(size_t thread_id, size_t example_id) const
{
  return contexts[thread_id][example_id % preallocated_count].c_str();
}

bool test_data_provider::is_rewarded(size_t thread_id, size_t example_id) const
{
  return reward_period > 0 && example_id % reward_period == 0;
}

int test_data_provider::report_outcome(reinforcement_learning::live_model* rl, size_t thread_id, size_t example_id,
    reinforcement_learning::api_status* status) const
{
  const auto event_id = create_event_id(thread_id, example_id);
  if (is_float_outcome) { return rl->report_outcome(event_id.c_str(), get_outcome(thread_id, example_id), status); }
  return rl->report_outcome(event_id.c_str(), get_outcome_json(thread_id, example_id), status);
}
