#pragma once
#include "decision_response.h"
#include "learning_mode.h"
#include "multi_slot_response.h"
#include "ranking_response.h"
#include "rl_string_view.h"
#include "time_helper.h"

#include <flatbuffers/flatbuffers.h>

#include <string>

namespace reinforcement_learning
{
struct timestamp;
namespace utility
{
class data_buffer;
}

class event
{
public:
  event(){};
  event(const char* seed_id, const timestamp& ts, float pass_prob = 1.f);
  event(const event&) = default;
  event(event&&) = default;
  event& operator=(const event&) = default;
  event& operator=(event&&) = default;
  virtual ~event() = default;
  float get_pass_prob() const;
  uint64_t get_event_index() const;
  void set_event_index(uint64_t event_index);
  timestamp get_client_time_gmt() const;
  ;
  virtual bool try_drop(float pass_prob, int drop_pass);
  const std::string& get_seed_id() const { return _seed_id; }

protected:
  float prg(int drop_pass) const;

protected:
  std::string _seed_id;
  float _pass_prob = 1.0;
  timestamp _client_time_gmt;
  uint64_t _event_index;
};

class ranking_response;

// serializable ranking event
class ranking_event : public event
{
public:
  ranking_event() {}
  ranking_event(const ranking_event& other) = default;
  ranking_event(ranking_event&& other) = default;
  ranking_event& operator=(const ranking_event& other) = default;
  ranking_event& operator=(ranking_event&& other) = default;
  ~ranking_event() = default;

  const std::vector<unsigned char>& get_context() const;
  const std::vector<uint64_t>& get_action_ids() const;
  const std::vector<float>& get_probabilities() const;
  const std::string& get_model_id() const;
  bool get_defered_action() const;
  const std::string& get_event_id() const { return get_seed_id(); }
  learning_mode get_learning_mode() const;

public:
  static ranking_event choose_rank(const char* event_id, string_view context, unsigned int flags,
      const ranking_response& resp, const timestamp& ts, float pass_prob = 1, learning_mode learning_mode = ONLINE);

private:
  ranking_event(const char* event_id, bool deferred_action, float pass_prob, string_view context,
      const ranking_response& response, const timestamp& ts, learning_mode learning_mode);

  std::vector<unsigned char> _context;
  std::vector<uint64_t> _action_ids_vector;
  std::vector<float> _probilities_vector;
  std::string _model_id;
  bool _deferred_action = false;
  learning_mode _learning_mode;
};

// serializable decision ranking event
class decision_ranking_event : public event
{
public:
  decision_ranking_event();
  decision_ranking_event(const decision_ranking_event& other) = delete;
  decision_ranking_event(decision_ranking_event&& other) = default;
  decision_ranking_event& operator=(const decision_ranking_event& other) = delete;
  decision_ranking_event& operator=(decision_ranking_event&& other) = default;

  const std::vector<unsigned char>& get_context() const;
  const std::vector<std::vector<uint32_t>>& get_actions_ids() const;
  const std::vector<std::vector<float>>& get_probabilities() const;
  const std::string& get_model_id() const;
  bool get_defered_action() const;
  const std::vector<std::string>& get_event_ids() const;

public:
  static decision_ranking_event request_decision(const std::vector<const char*>& event_ids, string_view context,
      unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, const timestamp& ts,
      float pass_prob = 1.f);

private:
  decision_ranking_event(const std::vector<const char*>& event_ids, bool deferred_action, float pass_prob,
      string_view context, std::vector<std::vector<uint32_t>> action_ids, std::vector<std::vector<float>> pdfs,
      std::string model_version, const timestamp& ts);

  std::vector<unsigned char> _context;
  std::vector<std::vector<uint32_t>> _action_ids_vector;
  std::vector<std::vector<float>> _probilities_vector;
  std::vector<std::string> _event_ids;

  std::string _model_id;
  bool _deferred_action{};
};

// serializable decision ranking event
class multi_slot_decision_event : public event
{
public:
  multi_slot_decision_event() = default;
  multi_slot_decision_event(const multi_slot_decision_event& other) = delete;
  multi_slot_decision_event(multi_slot_decision_event&& other) = default;
  multi_slot_decision_event& operator=(const multi_slot_decision_event& other) = delete;
  multi_slot_decision_event& operator=(multi_slot_decision_event&& other) = default;

  const std::vector<unsigned char>& get_context() const;
  const std::vector<std::vector<uint32_t>>& get_actions_ids() const;
  const std::vector<std::vector<float>>& get_probabilities() const;
  const std::string& get_model_id() const;
  bool get_defered_action() const;
  const std::string& get_event_id() const;

public:
  static multi_slot_decision_event request_decision(const std::string& event_id, string_view context,
      unsigned int flags, const std::vector<std::vector<uint32_t>>& action_ids,
      const std::vector<std::vector<float>>& pdfs, const std::string& model_version, const timestamp& ts,
      float pass_prob = 1.f);

private:
  multi_slot_decision_event(const std::string& event_id, bool deferred_action, float pass_prob, string_view context,
      std::vector<std::vector<uint32_t>> action_ids, std::vector<std::vector<float>> pdfs, std::string model_version,
      const timestamp& ts);

  std::vector<unsigned char> _context;
  std::vector<std::vector<uint32_t>> _action_ids_vector;
  std::vector<std::vector<float>> _probilities_vector;
  std::string _event_id;

  std::string _model_id;
  bool _deferred_action;
};

// serializable outcome event
class outcome_event : public event
{
public:
  outcome_event() {}
  outcome_event(const outcome_event& other) = delete;
  outcome_event(outcome_event&& other) = default;
  outcome_event& operator=(const outcome_event& other) = delete;
  outcome_event& operator=(outcome_event&& other) = default;
  ~outcome_event() = default;

  const std::string& get_outcome() const;
  float get_numeric_outcome() const;
  bool get_action_taken() const;
  const std::string& get_event_id() const { return get_seed_id(); }

  static const unsigned int outcome_type_unset = 0;
  static const unsigned int outcome_type_string = 1;
  static const unsigned int outcome_type_numeric = 2;
  static const unsigned int outcome_type_action_taken = 3;
  unsigned int get_outcome_type() const { return _outcome_type; }

public:
  static outcome_event report_action_taken(const char* event_id, const timestamp& ts, float pass_prob = 1);
  static outcome_event report_outcome(
      const char* event_id, const char* outcome, const timestamp& ts, float pass_prob = 1);
  static outcome_event report_outcome(const char* event_id, float outcome, const timestamp& ts, float pass_prob = 1);

private:
  outcome_event(const char* event_id, float pass_prob, const char* outcome, bool action_taken, const timestamp& ts);
  outcome_event(const char* event_id, float pass_prob, float outcome, bool action_taken, const timestamp& ts);

private:
  std::string _outcome;
  float _float_outcome = 0.0;
  bool _action_taken = false;
  unsigned int _outcome_type = 0;
};
}  // namespace reinforcement_learning
