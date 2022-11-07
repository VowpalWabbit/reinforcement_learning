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
  static int create_trainable_vw_model(std::unique_ptr<trainable_vw_model>& output,
      const utility::configuration& config, i_trace* trace_logger = nullptr, api_status* status = nullptr);

  // output current model state to buffer
  RL_ATTR(nodiscard)
  int get_data(model_management::model_data& data, api_status* status = nullptr);

  // overwrite internal VW model
  void set_model(std::unique_ptr<VW::workspace>&& model);
  void set_data(const model_management::model_data& data);

  // train model on data from a joined binary log
  RL_ATTR(nodiscard)
  int learn(std::unique_ptr<VW::io::reader>&& binary_log, api_status* status = nullptr);

  // generate a model_delta from the current model state and the previous call to get_model_delta() or set_model() or
  // set_data()
  VW::model_delta get_model_delta();

private:
  // private constructor because we should create objects with factory function
  trainable_vw_model(
      std::string command_line, std::string problem_type, std::string learning_mode, std::string reward_function);

  // need to keep both current and starting model in order to create model_delta
  std::unique_ptr<VW::workspace> _model = nullptr;
  std::unique_ptr<VW::workspace> _starting_model = nullptr;
  void copy_current_model_to_starting();

  std::string _command_line;
  std::string _problem_type;
  std::string _learning_mode;
  std::string _reward_function;
  void configure_joiner(std::unique_ptr<i_joiner>& joiner);
};

}  // namespace reinforcement_learning
