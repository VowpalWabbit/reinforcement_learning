#include "action_flags.h"
#include "ranking_event.h"
#include "data_buffer.h"
#include "explore_internal.h"
#include "hash.h"
#include "time_helper.h"
using namespace std;
namespace reinforcement_learning {
  event::event(const char* seed_id, const timestamp& ts, float pass_prob)
    : _seed_id(seed_id), _pass_prob(pass_prob), _client_time_gmt(ts) {}

  bool event::try_drop(float pass_prob, int drop_pass) {
    _pass_prob *= pass_prob;
    return prg(drop_pass) > pass_prob;
  }

  float event::get_pass_prob() const { return _pass_prob; }
  timestamp event::get_client_time_gmt() const { return _client_time_gmt; }

  float event::prg(int drop_pass) const {
    const auto seed_str = _seed_id + std::to_string(drop_pass);
    const auto seed = uniform_hash(seed_str.c_str(), seed_str.length(), 0);
    return exploration::uniform_random_merand48(seed);
  }

  ranking_event::ranking_event(const char* event_id, bool deferred_action, float pass_prob, const char* context,
                               const ranking_response& response, const timestamp& ts, learning_mode learning_mode)
    : event(event_id, ts, pass_prob), _model_id(response.get_model_id()),
      _deferred_action(deferred_action), _learning_mode(learning_mode){
    for (auto const& r : response) {
      _action_ids_vector.push_back(r.action_id + 1);
      _probilities_vector.push_back(r.probability);
    }
    string context_str(context);
    copy(context_str.begin(), context_str.end(), std::back_inserter(_context));
  }

  const std::vector<unsigned char>& ranking_event::get_context() const { return _context; }
  const std::vector<uint64_t>& ranking_event::get_action_ids() const { return _action_ids_vector; }
  const std::vector<float>& ranking_event::get_probabilities() const { return _probilities_vector; }
  const std::string& ranking_event::get_model_id() const { return _model_id; }
  bool ranking_event::get_defered_action() const { return _deferred_action; }
  learning_mode ranking_event::get_learning_mode() const { return _learning_mode; }

  ranking_event ranking_event::choose_rank(const char* event_id, const char* context, unsigned int flags,
                                           const ranking_response& resp, const timestamp& ts, float pass_prob, learning_mode learning_mode) {
    return ranking_event(event_id, flags & action_flags::DEFERRED, pass_prob, context, resp, ts, learning_mode);
  }

  decision_ranking_event::decision_ranking_event() { }

  decision_ranking_event::decision_ranking_event(const std::vector<const char*>& event_ids, bool deferred_action, float pass_prob, const char* context,
    const std::vector<std::vector<uint32_t>>& action_ids, const std::vector<std::vector<float>>& pdfs, const std::string& model_version, const timestamp& ts)
    : event(event_ids[0], ts, pass_prob)
    , _deferred_action(deferred_action)
    , _action_ids_vector(action_ids)
    , _probilities_vector(pdfs)
    , _model_id(model_version) {
    string context_str(context);
    for(auto evt : event_ids)
    {
      _event_ids.emplace_back(evt);
    }
    copy(context_str.begin(), context_str.end(), std::back_inserter(_context));
  }

  const std::vector<unsigned char>& decision_ranking_event::get_context() const { return _context; }
  const std::vector<std::vector<uint32_t>>& decision_ranking_event::get_actions_ids() const { return _action_ids_vector; }
  const std::vector<std::vector<float>>& decision_ranking_event::get_probabilities() const { return _probilities_vector; }
  const std::string& decision_ranking_event::get_model_id() const { return _model_id; }
  bool decision_ranking_event::get_defered_action() const { return _deferred_action; }
  const std::vector<std::string>& decision_ranking_event::get_event_ids() const { return _event_ids; }

  decision_ranking_event decision_ranking_event::request_decision(const std::vector<const char*>& event_ids, const char* context, unsigned int flags,
    const std::vector<std::vector<uint32_t>>& action_ids, const std::vector<std::vector<float>>& pdfs, const std::string& model_version, const timestamp& ts, float pass_prob) {
    return decision_ranking_event(event_ids, flags & action_flags::DEFERRED, pass_prob, context, action_ids, pdfs, model_version, ts);
  }

  outcome_event::outcome_event(const char* event_id, float pass_prob, const char* outcome, bool action_taken, const timestamp& ts)
    : event(event_id, ts, pass_prob), _outcome(outcome), _float_outcome(0.0f), _action_taken(action_taken) { }

  outcome_event::outcome_event(const char* event_id, float pass_prob, float outcome, bool action_taken, const timestamp& ts)
    : event(event_id, ts, pass_prob), _outcome(""), _float_outcome(outcome), _action_taken(action_taken) { }

  outcome_event outcome_event::report_outcome(const char* event_id, const char* outcome, const timestamp& ts, float pass_prob) {
    outcome_event evt(event_id, pass_prob, outcome, false, ts);
    evt._outcome_type = outcome_type_string;
    return evt;
  }

  outcome_event outcome_event::report_outcome(const char* event_id, float outcome, const timestamp& ts, float pass_prob) {
    outcome_event evt(event_id, pass_prob, outcome, false, ts);
    evt._outcome_type = outcome_type_numeric;
    return evt;
  }

  outcome_event outcome_event::report_action_taken(const char* event_id, const timestamp& ts, float pass_prob) {
    outcome_event evt(event_id, pass_prob, "", true, ts);
    evt._outcome_type = outcome_type_action_taken;
    return evt;
  }

  const std::string& outcome_event::get_outcome() const { return _outcome; }
  float outcome_event::get_numeric_outcome() const { return _float_outcome; }
  bool outcome_event::get_action_taken() const { return _action_taken; }
}
