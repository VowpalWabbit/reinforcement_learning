#pragma once

#include "api_status.h"
#include "configuration.h"
#include "federation/joined_log_provider.h"
#include "joiners/example_joiner.h"
#include "model_mgmt.h"
#include "vw/core/global_data.h"
#include "vw/core/merge.h"

#include <memory>

namespace reinforcement_learning
{
class trainable_vw_model
{
public:
  RL_ATTR(nodiscard)
  static int create(std::unique_ptr<trainable_vw_model>& output, const utility::configuration& config,
      i_trace* trace_logger = nullptr, api_status* status = nullptr);

  // Output current model state to buffer
  RL_ATTR(nodiscard)
  int get_data(model_management::model_data& data, api_status* status = nullptr);

  // Overwrite internal VW model with another model
  RL_ATTR(nodiscard)
  int set_model(std::unique_ptr<VW::workspace>&& model, api_status* status = nullptr);

  // Overwrite internal VW model with the given model data
  RL_ATTR(nodiscard)
  int set_data(const model_management::model_data& data, api_status* status = nullptr);

  // Train model on data from a joined binary log
  RL_ATTR(nodiscard)
  int learn(std::unique_ptr<VW::io::reader>&& binary_log, api_status* status = nullptr);

  // Train model on VW::example* objects
  // This does not call VW::finish_example on the examples passed into here
  RL_ATTR(nodiscard)
  int learn(VW::workspace& example_ws, VW::multi_ex& examples, api_status* status = nullptr);

  // Generate a model_delta from the current model state and the previous call to
  // get_model_delta() or set_model() or set_data()
  RL_ATTR(nodiscard)
  int get_model_delta(VW::model_delta& output, api_status* status = nullptr);

  RL_ATTR(nodiscard)
  int get_model_delta(VW::io::writer& output, api_status* status = nullptr);

private:
  // Private constructor because we should create objects with factory function
  trainable_vw_model(std::string command_line, std::string problem_type, std::string learning_mode,
      std::string reward_function, i_trace* trace_logger);

  // Need to keep both current and starting model in order to create model_delta
  std::unique_ptr<VW::workspace> _model = nullptr;
  std::unique_ptr<VW::workspace> _starting_model = nullptr;

  void copy_current_model_to_starting();

  const std::string _command_line;
  const std::string _problem_type;
  const std::string _learning_mode;
  const std::string _reward_function;

  void configure_joiner(std::unique_ptr<i_joiner>& joiner) const;

  i_trace* _trace_logger = nullptr;
  std::mutex _mutex;
};

}  // namespace reinforcement_learning
