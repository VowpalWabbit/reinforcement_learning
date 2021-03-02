#pragma once

#include "../../../rlclientlib/generated/v2/FileFormat_generated.h"
#include "../../../rlclientlib/generated/v2/Metadata_generated.h"
#include "err_constants.h"

#include "example.h"
#include "v_array.h"
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

using RewardCalcType = int (*)(const joined_event &, v_array<example *> &);

int default_reward_calculation(const joined_event &event, v_array<example *> &);

class ExampleJoiner {
public:
  ExampleJoiner(
      vw *vw,
      RewardCalcType jl = default_reward_calculation); // TODO rule of 5
  // takes an event which will have a timestamp and event payload, performs any
  // needed pre-processing (e.g. decompressing), keeps the relevant event
  // information and then depending on whether the event is an interaction or an
  // observation it sends it to the correct event processor
  int process_event(const v2::JoinedEvent &joined_event,
                    v_array<example *> &examples);
  // train on joined examples
  int train_on_joined(v_array<example *> &examples);

private:
  int process_interaction(const v2::Event &event, const v2::Metadata &metadata,
                          v_array<example *> &examples);

  // process outcome will find the examples that reside under the same event id,
  // and store the outcome until all the information is available to join and
  // train
  int process_outcome(const v2::Event &event, const v2::Metadata &metadata);

  // from dictionary id to example object
  // right now holding one dedup dictionary at a time, could be exented to a map
  // of maps holding more than one dedup dictionaries at a time
  std::unordered_map<std::string, example *> _dedup_examples;
  // from event id to all the information required to create a complete
  // (multi)example
  std::unordered_map<std::string, joined_event> _unjoined_examples;

  vw *_vw;

  RewardCalcType reward_calculation;
};
