#pragma once

#include "event_processors/reward.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/FileFormat_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "lru_dedup_cache.h"
#include "metrics/metrics.h"
#include "parse_example_external.h"
#include "vw/core/error_constants.h"
#include "vw/core/example.h"
#include "vw/core/parse_args.h"
#include "vw/core/v_array.h"

#include <list>
#include <queue>
#include <unordered_map>
// VW headers
// vw.h has to come before json_utils.h
// clang-format off
#include "vw/core/vw.h"
// clang-format on

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

class i_joiner
{
public:
  explicit i_joiner(VW::io::logger logger_) : logger(std::move(logger_)) {}
  virtual ~i_joiner() = default;

  virtual void set_reward_function(const v2::RewardFunctionType type, bool sticky = false) = 0;
  virtual void set_default_reward(float default_reward, bool sticky = false) = 0;
  virtual void set_learning_mode_config(v2::LearningModeType learning_mode, bool sticky = false) = 0;
  virtual void set_problem_type_config(v2::ProblemType problem_type, bool sticky = false) = 0;
  virtual void set_use_client_time(bool use_client_time, bool sticky = false) = 0;
  virtual void apply_cli_overrides(VW::workspace* all, const VW::external::parser_options& parsed_options) = 0;

  /**
   * @brief Tells whether config was provided such that it can start joining examples.
   * TODO This method is naive with respect to whether the config provided was both enough and valid.
   *
   * @return true if the joiner is ready, false otherwise.
   */
  virtual bool joiner_ready() = 0;

  // Takes an event which will have a timestamp and event payload
  // groups all events interactions with their event observations based on their
  // id. The grouped events can be processed when process_joined() is called
  virtual bool process_event(const v2::JoinedEvent& joined_event) = 0;
  // Takes all grouped events, processes them (e.g. decompression) and populates
  // the examples array with complete example(s) ready to be used by vw for
  // training
  virtual bool process_joined(VW::multi_ex& examples) = 0;
  // true if there are still event-groups to be processed from a deserialized
  // batch
  virtual bool processing_batch() = 0;

  // to be called after process_joined
  // returns true if the event that was just processed is a skip_learn event
  // otherwise returns false
  virtual bool current_event_is_skip_learn() { return false; }

  virtual void on_new_batch() = 0;

  virtual void on_batch_read() = 0;

  virtual void setup_metrics() {}

  virtual metrics::joiner_metrics get_metrics() = 0;

protected:
  VW::io::logger logger;
};
