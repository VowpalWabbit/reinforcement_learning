#pragma once

#include "err_constants.h"

#include "example.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/FileFormat_generated.h"
#include "lru_dedup_cache.h"
#include "i_example_joiner.h"
#include "v_array.h"

#include <list>
#include <queue>
#include <unordered_map>
// VW headers
// vw.h has to come before json_utils.h
// clang-format off
#include "vw.h"
#include "json_utils.h"
// clang-format on

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

class example_joiner : public i_example_joiner {
public:
  example_joiner(vw *vw); // TODO rule of 5

  virtual ~example_joiner();

  virtual void set_reward_function(const v2::RewardFunctionType type);
  virtual void set_default_reward(float default_reward);
  virtual void set_learning_mode_config(const v2::LearningModeType& learning_mode);
  virtual void set_problem_type_config(const v2::ProblemType& problem_type);

  // Takes an event which will have a timestamp and event payload
  // groups all events interactions with their event observations based on their
  // id. The grouped events can be processed when process_joined() is called
  virtual int process_event(const v2::JoinedEvent &joined_event);
  // Takes all grouped events, processes them (e.g. decompression) and populates
  // the examples array with complete example(s) ready to be used by vw for
  // training
  virtual int process_joined(v_array<example *> &examples);
  // true if there are still event-groups to be processed from a deserialized
  // batch
  virtual bool processing_batch();

  virtual void on_new_batch();
  float get_reward();

private:
  int process_dedup(const v2::Event &event, const v2::Metadata &metadata);

  int process_interaction(const v2::Event &event, const v2::Metadata &metadata,
                          const TimePoint &enqueued_time_utc,
                          v_array<example *> &examples);

  int process_outcome(const v2::Event &event, const v2::Metadata &metadata,
                      const TimePoint &enqueued_time_utc);

  template <typename T>
  const T *process_compression(const uint8_t *data, size_t size,
                               const v2::Metadata &metadata);

  void try_set_label(const joined_event &je, float reward,
                     v_array<example *> &examples);

  example *get_or_create_example();

  static example &get_or_create_example_f(void *vw);

  void return_example(example *ex);

  static void return_example_f(void *vw, example *ex);

  lru_dedup_cache _dedup_cache;
  // from event id to all the information required to create a complete
  // (multi)example
  std::unordered_map<std::string, joined_event> _batch_grouped_examples;
  // from event id to all the events that have that event id
  std::unordered_map<std::string, std::vector<const v2::JoinedEvent *>>
      _batch_grouped_events;
  std::queue<std::string> _batch_event_order;

  std::vector<example *> _example_pool;

  vw *_vw;
  flatbuffers::DetachedBuffer _detached_buffer;

  float _default_reward = 0.f;
  float _reward = _default_reward;
  RewardCalcType _reward_calculation;

  v2::LearningModeType _learning_mode_config = v2::LearningModeType_Online;
  v2::ProblemType _problem_type_config = v2::ProblemType_UNKNOWN;
};
