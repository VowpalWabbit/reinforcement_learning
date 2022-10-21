#pragma once

#include "configuration.h"
#include "federation/federated_client.h"
#include "trace_logger.h"
#include "vw/core/vw_fwd.h"

namespace reinforcement_learning
{
struct local_client : i_federated_client
{
  local_client(std::unique_ptr<VW::workspace> initial_model, i_trace* trace_logger);
  ~local_client() override;
  RL_ATTR(nodiscard)
  int try_get_model(const std::string& app_id,
      /* inout */ model_management::model_data& data, /* out */ bool& model_received,
      api_status* status = nullptr) override;

  RL_ATTR(nodiscard) int report_result(const uint8_t* payload, size_t size, api_status* status = nullptr) override;

private:
  enum class state_t
  {
    model_available,
    model_retrieved
  };

  state_t _state;
  std::unique_ptr<VW::workspace> _current_model;
  i_trace* _trace_logger;
};

// Read MODEL_VW_INITIAL_COMMAND_LINE

RL_ATTR(nodiscard)
int create_local_client(const reinforcement_learning::utility::configuration& config,
    /*out*/ std::unique_ptr<reinforcement_learning::i_federated_client>& object, i_trace* trace_logger,
    reinforcement_learning::api_status* status = nullptr);

}  // namespace reinforcement_learning