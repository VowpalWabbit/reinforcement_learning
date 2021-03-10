#pragma once

#include "err_constants.h"

#include "example.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "v_array.h"

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
  std::string client_time_utc;
  std::string app_id;
  v2::PayloadType payload_type;
  float pass_probability;
  v2::EventEncoding event_encoding;
  v2::LearningModeType learning_mode;
};

struct outcome_event {
  metadata_info metadata;
  std::string s_index;
  int index;
  std::string s_value;
  float value;
};

struct joined_event {
  std::string joined_event_timestamp;
  metadata_info interaction_metadata;
  DecisionServiceInteraction interaction_data;
  std::vector<outcome_event> outcome_events;
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

  ~example_joiner();

  void set_reward_function(const v2::RewardFunctionType type);
  void set_default_reward(float default_reward);

  // Takes an event which will have a timestamp and event payload
  // groups all events interactions with their event observations based on their
  // id. The grouped events can be processed when process_joined() is called
  int process_event(const v2::JoinedEvent &joined_event);
  // Takes all grouped events, processes them (e.g. decompression) and populates
  // the examples array with complete example(s) ready to be used by vw for
  // training
  int process_joined(v_array<example *> &examples);
  // true if there are still event-groups to be processed from a deserialized
  // batch
  bool processing_batch();

private:
  int process_dedup(const v2::Event &event, const v2::Metadata &metadata);

  int process_interaction(const v2::Event &event, const v2::Metadata &metadata,
                          v_array<example *> &examples);

  int process_outcome(const v2::Event &event, const v2::Metadata &metadata);

  static example &get_or_create_example_f(void *vw);

  example *get_or_create_example();

  void clean_label_and_prediction(example *ex);

  // from dictionary id to example object
  // right now holding one dedup dictionary at a time, could be exented to a map
  // of maps holding more than one dedup dictionaries at a time
  std::unordered_map<uint64_t, example *> _dedup_examples;
  // from event id to all the information required to create a complete
  // (multi)example
  std::unordered_map<std::string, joined_event> _batch_grouped_examples;
  // from event id to all the events that have that event id
  std::unordered_map<std::string, std::vector<const v2::Event *>>
      _batch_grouped_events;
  std::queue<std::string> _batch_event_order;

  std::vector<example *> _example_pool;

  vw *_vw;

  float _default_reward = 0.f;
  RewardCalcType reward_calculation;
};
