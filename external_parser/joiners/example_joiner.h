#pragma once

#include "error_constants.h"

#include "event_processors/joined_event.h"
#include "event_processors/loop.h"
#include "example.h"
#include "joiners/i_joiner.h"
#include "lru_dedup_cache.h"
#include "metrics/metrics.h"
#include "v_array.h"

#include <fstream>
#include <list>
#include <queue>
#include <unordered_map>

class example_joiner : public i_joiner {
public:
  example_joiner(vw *vw); // TODO rule of 5
  example_joiner(vw *vw, bool binary_to_json, std::string outfile_name);

  virtual ~example_joiner();

  void set_reward_function(const v2::RewardFunctionType type, bool sticky = false) override;
  void set_default_reward(float default_reward, bool sticky = false) override;
  void set_learning_mode_config(v2::LearningModeType learning_mode, bool sticky = false) override;
  void set_problem_type_config(v2::ProblemType problem_type, bool sticky = false) override;
  void set_use_client_time(bool use_client_time, bool sticky = false) override;
  bool joiner_ready() override;

  float default_reward() const { return _loop_info.default_reward; }
  v2::LearningModeType learning_mode_config() const { return _loop_info.learning_mode_config; }
  v2::ProblemType problem_type_config() const { return _loop_info.problem_type_config; }
  bool use_client_time() const { return _loop_info.use_client_time; }

  // Takes an event which will have a timestamp and event payload
  // groups all events interactions with their event observations based on their
  // id. The grouped events can be processed when process_joined() is called
  bool process_event(const v2::JoinedEvent &joined_event) override;

  /**
   * Takes all grouped events, processes them (e.g. decompression) and populates
   * the examples array with complete example(s) ready to be used by vw for
   * training
   *
   * returns false if the interaction can not be used for training due to
   * malformed or missing data
   * On returning false, the v_array of examples will be cleaned and prepared
   * for the potential next parser call
   *
   * returns true if examples in example v_array are ready to be used for
   * training
   *
   * --- Assumptions ---
   *
   * Interactions precede observations
   *
   * If an interaction is processed without problems it will create an entry in
   * _batched_grouped_examples with key the id (event id). If an interaction
   * was not processed correctly due to an error then the corresponding
   * outcome(s) will be ignored and we will not learn from that interaction
   *
   * If an interaction was processed correctly but any of its outcome(s) failed
   * to be processed then we do not learn from that interaction
   *
   * If metadata is malformed then don't attempt to process event
   * We can't attempt to invalidate the specific id since we don't know it (it's
   * in the metadata)
   */
  bool process_joined(v_array<example *> &examples) override;

  // true if there are still event-groups to be processed from a deserialized
  // batch
  bool processing_batch() override;

  // to be called after process_joined
  // returns true if the event that was just processed is a skip_learn event
  // otherwise returns false
  bool current_event_is_skip_learn() override;

  void on_new_batch() override;

  void on_batch_read() override;

  metrics::joiner_metrics get_metrics() override;

  void persist_metrics() override;

private:
  bool process_dedup(const v2::Event &event, const v2::Metadata &metadata);

  bool process_interaction(const v2::Event &event, const v2::Metadata &metadata,
                           const TimePoint &enqueued_time_utc,
                           v_array<example *> &examples);

  bool process_outcome(const v2::Event &event, const v2::Metadata &metadata,
                       const TimePoint &enqueued_time_utc);

  void clear_batch_info();
  void clear_event_id_batch_info(const std::string &id);
  void invalidate_joined_event(const std::string &id);
  void clear_vw_examples(v_array<example *> &examples);

  example *get_or_create_example();

  static example &get_or_create_example_f(void *vw);

  void return_example(example *ex);

  static void return_example_f(void *vw, example *ex);

  lru_dedup_cache _dedup_cache;
  // from event id to all the information required to create a complete
  // (multi)example
  std::unordered_map<std::string, joined_event::joined_event>
      _batch_grouped_examples;
  // from event id to all the events that have that event id
  std::unordered_map<std::string, std::vector<const v2::JoinedEvent *>>
      _batch_grouped_events;
  std::queue<std::string> _batch_event_order;

  std::vector<example *> _example_pool;

  vw *_vw;
  flatbuffers::DetachedBuffer _detached_buffer;

  loop::sticky_value<reward::RewardFunctionType> _reward_calculation;
  loop::loop_info _loop_info;
  metrics::joiner_metrics _joiner_metrics;

  // should be reset on each call to process_joined
  // let's us know if the current event processed by process_joined is a
  // skip_learn event or not
  bool _current_je_is_skip_learn;

  bool _binary_to_json;
  std::ofstream _outfile;
};
