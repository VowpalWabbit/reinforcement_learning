#pragma once

#include "err_constants.h"

#include "example.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/FileFormat_generated.h"
#include "lru_dedup_cache.h"
#include "timestamp_helper.h"
#include "v_array.h"

#include <fstream>
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

struct metadata_info {
  TimePoint client_time_utc;
  std::string app_id;
  v2::PayloadType payload_type;
  float pass_probability;
  v2::EventEncoding event_encoding;
  std::string event_id;
  v2::LearningModeType learning_mode;
};

struct outcome_event {
  metadata_info metadata;
  std::string s_index;
  int index;
  std::string s_value;
  float value;
  TimePoint enqueued_time_utc;
  bool action_taken;
};

// TODO in separate PR move metadat_info, outcome_event, joined_event and reward
// calculations to their own file
struct joined_event {
  joined_event(const TimePoint &tp, metadata_info &&mi,
               DecisionServiceInteraction &&id, const std::string &ctx,
               const std::string &mid)
      : joined_event_timestamp(tp), interaction_metadata(std::move(mi)),
        interaction_data(std::move(id)), outcome_events({}), context(ctx),
        model_id(mid), ok(true) {}
  joined_event() : ok(true) {}

  TimePoint joined_event_timestamp;
  metadata_info interaction_metadata;
  DecisionServiceInteraction interaction_data;
  std::vector<outcome_event> outcome_events;
  std::string context;
  std::string model_id;
  bool ok; // ok till proved otherwise
  // Default Baseline Action for CB is 1 (rl client recommended actions are 1
  // indexed in the CB case)
  static const int baseline_action = 1;
};

using RewardCalcType = float (*)(const joined_event &);

namespace RewardFunctions {
float average(const joined_event &event);
float sum(const joined_event &event);
float min(const joined_event &event);
float max(const joined_event &event);
float median(const joined_event &event);
float apprentice(const joined_event &event);
} // namespace RewardFunctions

class example_joiner {
public:
  example_joiner(vw *vw); // TODO rule of 5
  example_joiner(vw *vw, bool binary_to_json, std::string outfile_name);

  ~example_joiner();

  void set_reward_function(const v2::RewardFunctionType type);
  void set_default_reward(float default_reward);
  void set_learning_mode_config(const v2::LearningModeType &learning_mode);
  void set_problem_type_config(const v2::ProblemType &problem_type);

  // Takes an event which will have a timestamp and event payload
  // groups all events interactions with their event observations based on their
  // id. The grouped events can be processed when process_joined() is called
  bool process_event(const v2::JoinedEvent &joined_event);
  // Takes all grouped events, processes them (e.g. decompression) and populates
  // the examples array with complete example(s) ready to be used by vw for
  // training
  bool process_joined(v_array<example *> &examples);
  // true if there are still event-groups to be processed from a deserialized
  // batch
  bool processing_batch();

  float get_reward();
  float get_original_reward();

private:
  bool process_dedup(const v2::Event &event, const v2::Metadata &metadata);

  bool process_interaction(const v2::Event &event, const v2::Metadata &metadata,
                           const TimePoint &enqueued_time_utc,
                           v_array<example *> &examples);

  bool process_outcome(const v2::Event &event, const v2::Metadata &metadata,
                       const TimePoint &enqueued_time_utc);

  template <typename T>
  bool process_compression(const uint8_t *data, size_t size,
                           const v2::Metadata &metadata, const T *&payload);

  void try_set_label(const joined_event &je, v_array<example *> &examples);

  void clear_batch_info();
  void clear_event_id_batch_info(const std::string &id);
  void invalidate_joined_event(const std::string &id);

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
  // original reward is used to record the observed reward of apprentice mode
  float _original_reward = _default_reward;
  RewardCalcType _reward_calculation;

  v2::LearningModeType _learning_mode_config = v2::LearningModeType_Online;
  v2::ProblemType _problem_type_config = v2::ProblemType_UNKNOWN;

  bool _binary_to_json;
  std::ofstream _outfile;
};
