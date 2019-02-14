#pragma once
#include <string>
#include "ranking_response.h"

namespace reinforcement_learning {
  namespace utility { class data_buffer; }

  class event {
  public:
    event();
    event(const char* event_id, float pass_prob = 1);
    event(event&& other);
    const std::string& get_event_id() const {
      return _event_id;
    }

    event& operator=(event&& other);
    virtual ~event();

    virtual bool try_drop(float pass_prob, int drop_pass);
    float get_pass_prob() const;
  
  protected:
    float prg(int drop_pass) const;

  protected:
    std::string _event_id;
    float _pass_prob;
  };

  class ranking_response;

  //serializable ranking event
  class ranking_event : public event {
  public:
    ranking_event();
    ranking_event(ranking_event&& other) = default;
    ranking_event& operator=(ranking_event&& other) = default;

    const std::vector<unsigned char>& get_context() const;
    const std::vector<uint64_t>& get_action_ids() const;
    const std::vector<float>& get_probabilities() const;
    const std::string& get_model_id() const;
    bool get_defered_action() const;
  
  public:
    static ranking_event choose_rank(const char* event_id, const char* context,
      unsigned int flags, const ranking_response& resp, float pass_prob = 1);

  private:
    ranking_event(const char* event_id, bool deferred_action, float pass_prob, const char* context, const ranking_response& response);

    std::vector<unsigned char> _context;
    std::vector<uint64_t> _action_ids_vector;
    std::vector<float> _probilities_vector;
    std::string _model_id;
    bool _deferred_action;
  };

  //serializable outcome event
  class outcome_event : public event {
  public:
    outcome_event() = default;
    outcome_event(outcome_event&& other) = default;
    outcome_event& operator=(outcome_event&& other) = default;

    const std::string& get_outcome() const;
    float get_numeric_outcome() const;
    bool get_action_taken() const;

    static const unsigned int outcome_type_unset = 0;
    static const unsigned int outcome_type_string = 1;
    static const unsigned int outcome_type_numeric = 2;
    static const unsigned int outcome_type_action_taken = 3;
    unsigned int get_outcome_type() const { return _outcome_type; }

  public:
    static outcome_event report_action_taken(const char* event_id, float pass_prob = 1);
    static outcome_event report_outcome(const char* event_id, const char* outcome, float pass_prob = 1);
    static outcome_event report_outcome(const char* event_id, float outcome, float pass_prob = 1);

  private:
    outcome_event(const char* event_id, float pass_prob, const char* outcome, bool _action_taken);
    outcome_event(const char* event_id, float pass_prob, float outcome, bool _action_taken);
  
  private:
    std::string _outcome;
    float _float_outcome;
    bool _action_taken;
    unsigned int _outcome_type = 0;
  };
}
