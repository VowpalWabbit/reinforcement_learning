#pragma once

#include "err_constants.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "vw_model/safe_vw.h"

// VW headers
#include "json_utils.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;
namespace err = reinforcement_learning::error_code;

// example joiner will need to hold a safe_vw object for training in memory
// TODO: hold observations in memory too?
// example joiner will need to hold a mapping from eventid -> example object
// (easy to extend for dedup) when outcome comes in we join and then train

// example_joiner will take the object in flatbuffer format, call dsjson parser
// and create the example object when the example is complete with a label, it
// can then call training/dispatch an example interactions that are multiline
// come in one bundle so we can potentially do the check: vw->is_multiline and
// hold either single examples or multiexamples -> depends on command line args

struct event_info {
  std::string join_timestamp;
  v_array<example *> examples;
  DecisionServiceInteraction interaction_data;
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


  //   reinforcement_learning::safe_vw _vw;
  // from dictionary id to example object
  // right now holding one dedup dictionary at a time, could be exented to a map
  // of maps holding more than one dedup dictionaries at a time
  std::unordered_map<std::string, example *> _dedup_examples;
  // from event id to example object(s)
  std::unordered_map<std::string, event_info> _unjoined_examples;
  std::vector<std::string> _ready_events;

  std::string _initial_command_line;

  // WARNING: copying from save_vw for now to check code path, but should not
  // duplicate code
  vw *_vw;
  std::vector<example *> _example_pool;

  example *get_or_create_example();
  static example &get_or_create_example_f(void *vw);
};
