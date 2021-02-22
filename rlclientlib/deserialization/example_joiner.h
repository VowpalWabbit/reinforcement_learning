#pragma once

#include "err_constants.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "vw_model/safe_vw.h"

// VW headers
#include "json_utils.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;
namespace err = reinforcement_learning::error_code;


struct metadata_info
{
  std::string client_time_utc;
  std::string app_id;
  v2::PayloadType payload_type;
  float pass_probability;
  v2::EventEncoding event_encoding;
};

struct outcome_event
{
  metadata_info metadata;
  std::string s_index;
  int index;
  std::string s_value;
  float value;
};

// TODO move to own file
struct event_info {
  std::string joined_event_timestamp;
  v_array<example *> examples;
  metadata_info interaction_metadata;
  DecisionServiceInteraction interaction_data;
  std::vector<outcome_event> outcome_events;
};

class ExampleJoiner {
public:
  explicit ExampleJoiner(
      const std::string &initial_command_line); // TODO rule of 5
  ~ExampleJoiner();
  // takes an event which will have a timestamp and event payload unwraps it
  // from its flatbuffer wrapping, performs any needed pre-processing (e.g.
  // decompressing), keeps the relevant event information and then depending on
  // whether the event is an interaction or an observation will either store the
  // interaction until it can be used for training, or perform the join and call
  // training on a complete example
  int process_event(const v2::JoinedEvent &joined_event);
  // train on joined examples
  int train_on_joined();

private:
  // TODO: should we pretend that this is not a flatbuffers parser and pass
  // around c-style byte arrays instead?
  int process_interaction(const flatbuffers::Vector<uint8_t> *payload,
                          const v2::Metadata &metadata);

  // process outcome will find the examples that reside under the same event id,
  // populate the example label by performing the join logic
  int process_outcome(const flatbuffers::Vector<uint8_t> *payload,
                      const v2::Metadata &metadata);


  // from dictionary id to example object
  // right now holding one dedup dictionary at a time, could be exented to a map
  // of maps holding more than one dedup dictionaries at a time
  std::unordered_map<std::string, example *> _dedup_examples;
  // from event id to all the information required to create a complete (multi)example
  std::unordered_map<std::string, event_info> _unjoined_examples;

  std::string _initial_command_line;

  // WARNING: copying from save_vw for now to check code path, but should not
  // duplicate code
  vw *_vw;
  std::vector<example *> _example_pool;

  example *get_or_create_example();
  static example &get_or_create_example_f(void *vw);
};
