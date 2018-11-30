#include "action_flags.h"
#include "ranking_event.h"
#include "data_buffer.h"
#include "explore_internal.h"
#include "hash.h"
#include <sstream>
#include <iomanip>
using namespace std;
namespace reinforcement_learning {
  namespace u = utility;
  event::event() {}

  event::event(const char* event_id, float pass_prob)
    : _event_id(event_id), _pass_prob(pass_prob) {}

  event::event(event&& other)
    : _event_id(std::move(other._event_id)), _pass_prob(other._pass_prob) {}

  event& event::operator=(event&& other) {
    if (&other != this) {
      _event_id = std::move(other._event_id);
      _pass_prob = other._pass_prob;
    }
    return *this;
  }

  event::~event() {}

  bool event::try_drop(float pass_prob, int drop_pass) {
    _pass_prob *= pass_prob;
    return prg(drop_pass) > pass_prob;
  }

  float event::get_pass_prob() const { return _pass_prob; }

  float event::prg(int drop_pass) const {
    const auto seed_str = _event_id + std::to_string(drop_pass);
    const auto seed = uniform_hash(seed_str.c_str(), seed_str.length(), 0);
    return exploration::uniform_random_merand48(seed);
  }

  ranking_event::ranking_event() { }

  ranking_event::ranking_event(const char* event_id, bool deferred_action, float pass_prob, const char* context,
                               const ranking_response& response)
    : event(event_id, pass_prob), _deferred_action(deferred_action), _model_id(response.get_model_id()) {
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

  ranking_event ranking_event::choose_rank(const char* event_id, const char* context, unsigned int flags,
                                           const ranking_response& resp, float pass_prob) {
    return ranking_event(event_id, flags & action_flags::DEFERRED, pass_prob, context, resp);
  }

  outcome_event::outcome_event(const char* event_id, float pass_prob, const char* outcome, bool deferred_action)
    : event(event_id, pass_prob), _outcome(outcome), _float_outcome(0.0f), _deferred_action(deferred_action) { }

  outcome_event::outcome_event(const char* event_id, float pass_prob, float outcome, bool deferred_action)
    : event(event_id, pass_prob), _outcome(""), _float_outcome(outcome), _deferred_action(deferred_action) { }

  outcome_event outcome_event::report_outcome(const char* event_id, const char* outcome, float pass_prob) {
    outcome_event evt(event_id, pass_prob, outcome, true);
    evt.outcome_type = outcome_type_string;
    return evt;
  }

  outcome_event outcome_event::report_outcome(const char* event_id, float outcome, float pass_prob) {
    outcome_event evt(event_id, pass_prob, outcome, true);
    evt.outcome_type = outcome_type_numeric;
    return evt;
  }

  outcome_event outcome_event::report_action_taken(const char* event_id, float pass_prob) {
    outcome_event evt(event_id, pass_prob, "", false);
    evt.outcome_type = outcome_type_action_taken;
    return evt;
  }

  const std::string& outcome_event::get_outcome() const { return _outcome; }
  float outcome_event::get_numeric_outcome() const { return _float_outcome; }
  bool outcome_event::get_deferred_action() const { return _deferred_action; }
}
