#include "test_data_provider.h"

#include "data_buffer.h"
#include "ranking_event.h"

#include <sstream>
#include "serialization/json_serializer.h"

test_data_provider::test_data_provider(const std::string& experiment_name, size_t threads, size_t features, size_t actions, bool _is_float_outcome)
  : _experiment_name(experiment_name)
  , contexts(threads, std::vector<std::string>(preallocated_count))
  , outcomes(threads, std::vector<std::string>(preallocated_count))
  , is_float_outcome(_is_float_outcome)
{
  for (size_t t = 0; t < threads; ++t) {
    for (size_t i = 0; i < preallocated_count; ++i) {
      contexts[t][i] = create_context_json(create_features(features, t, i), create_action_features(actions, features, i));
      outcomes[t][i] = create_json_outcome(t, i);
    }
  }
}

std::string test_data_provider::create_event_id(size_t thread_id, size_t example_id) const {
  std::ostringstream oss;
  oss << _experiment_name << "-" << thread_id << "-" << example_id;
  return oss.str();
}

std::string test_data_provider::create_action_features(size_t actions, size_t features, size_t example_id) const {
  std::ostringstream oss;
  oss << R"("_multi": [ )";
  for (size_t a = 0; a < actions; ++a) {
	  oss << R"({ "TAction":{)";
	  for (size_t f = 0; f < features; ++f) {
      oss << R"("a_f_)" << f << R"(":"value_)" << (a + f + example_id) << R"(")";
      if (f + 1 < features) oss << ",";
	  }
    oss << "}}";
	  if (a + 1 < actions) oss << ",";
  }
  oss << R"(])";
  return oss.str();
}

std::string test_data_provider::create_slot_features(size_t slots, size_t features, size_t slot_decision_id) const {
  std::ostringstream oss;
  oss << R"("_slots": [ )";
  for (size_t a = 0; a < slots; ++a) {
    oss << R"({ "TSlot":{)";
    oss << R"("_id":")" << slot_decision_id << R"(")";
    if (features > 0) oss << ",";
    for (size_t f = 0; f < features; ++f) {
      oss << R"("a_f_)" << f << R"(":"value_)" << (a + f + slot_decision_id) << R"(")";
      if (f + 1 < features) oss << ",";
    }
    oss << "}}";
    if (a + 1 < slots) oss << ",";
  }
  oss << R"(])";
  return oss.str();
}

std::string test_data_provider::create_features(size_t features, size_t thread_id, size_t example_id) const {
  std::ostringstream oss;
  oss << R"("GUser":{)";
  oss << R"("f_int":)" << example_id << R"(,)";
  oss << R"("f_float":)" << float(example_id) + 0.5 << R"(,)";
  for (size_t f = 0; f < features; ++f) {
    oss << R"("f_str_)" << f << R"(":"value_)" << (f + thread_id + example_id) << R"(")";
    if (f + 1 < features) oss << ",";
  }
  oss << R"(})";
  return oss.str();
}

std::string test_data_provider::create_json_outcome(size_t thread_id, size_t example_id) const {
  std::ostringstream oss;
  oss << R"({"Reward":)" << get_outcome(thread_id, example_id) << R"(,"CustomRewardField":)" << get_outcome(thread_id, example_id) + 1 << "}";
  return oss.str();
}

std::string test_data_provider::create_context_json(const std::string& cntxt, const std::string& action) const {
  std::ostringstream oss;
  oss << "{ " << cntxt << ", " << action << " }";
  return oss.str();
}

float test_data_provider::get_outcome(size_t thread_id, size_t example_id) const {
  return is_rewarded(thread_id, example_id) ? static_cast<float>(thread_id  + example_id) : 0.f;
}

const char* test_data_provider::get_outcome_json(size_t thread_id, size_t example_id) const {
  return outcomes[thread_id][example_id % preallocated_count].c_str();
}

const char* test_data_provider::get_context(size_t thread_id, size_t example_id) const {
  return contexts[thread_id][example_id % preallocated_count].c_str();
}

bool test_data_provider::is_rewarded(size_t thread_id, size_t example_id) const {
  return example_id % 10 == 0;
}

void test_data_provider::log(size_t thread_id, size_t example_id, const reinforcement_learning::ranking_response& response, std::ostream& logger) const {
  size_t action_id;
  const auto event_id = create_event_id(thread_id, example_id);
  response.get_chosen_action_id(action_id);
  float prob = 0;
  for (auto it = response.begin(); it != response.end(); ++it) {
    if ((*it).action_id == action_id) {
      prob = (*it).probability;
    }
  }

  reinforcement_learning::utility::data_buffer buffer;
  logger << R"({"_label_cost":)" << -get_outcome(thread_id, example_id) << R"(,"_label_probability":)" << prob << R"(,"_label_Action":)" << (action_id + 1) << R"(,"_labelIndex":)" << action_id << ",";

  reinforcement_learning::timestamp ts;

  if (is_rewarded(thread_id, example_id)) {
    reinforcement_learning::outcome_event outcome_evt;
    if (is_float_outcome)
      outcome_evt = reinforcement_learning::outcome_event::report_outcome(event_id.c_str(), get_outcome(thread_id, example_id), ts);
    else
      outcome_evt = reinforcement_learning::outcome_event::report_outcome(event_id.c_str(), get_outcome_json(thread_id, example_id), ts);
    buffer.reset();
    reinforcement_learning::logger::json_collection_serializer<reinforcement_learning::outcome_event> jserial(buffer);
    jserial.add(outcome_evt);
    jserial.finalize();
    logger << R"("o":[)" << buffer.body_begin() << "],";
    buffer.reset();
  }

  auto ranking_evt = reinforcement_learning::ranking_event::choose_rank(event_id.c_str(), get_context(thread_id, example_id), reinforcement_learning::action_flags::DEFAULT, response, ts);
  buffer.reset();
  reinforcement_learning::logger::json_collection_serializer<reinforcement_learning::ranking_event> jserial(buffer);
  jserial.add(ranking_evt);
  jserial.finalize();
  logger << buffer.body_begin() << std::endl;
}

int test_data_provider::report_outcome(reinforcement_learning::live_model* rl, size_t thread_id, size_t example_id, reinforcement_learning::api_status* status) const {
  const auto event_id = create_event_id(thread_id, example_id);
  if (is_float_outcome)
    return rl->report_outcome(event_id.c_str(), get_outcome(thread_id, example_id), status);
  return rl->report_outcome(event_id.c_str(), get_outcome_json(thread_id, example_id), status);
}
