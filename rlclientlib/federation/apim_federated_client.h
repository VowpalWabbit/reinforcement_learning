#pragma once

#include "configuration.h"
#include "model_mgmt.h"
#include "federation/federated_client.h"
#include "trace_logger.h"
#include "vw/core/vw_fwd.h"

namespace reinforcement_learning
{
class apim_federated_client : i_federated_client
{
public:
  RL_ATTR(nodiscard)
  static int create(std::unique_ptr<i_federated_client>& output, const utility::configuration& config,
      i_trace* trace_logger = nullptr, std::unique_ptr<model_management::i_data_transport> transport = nullptr, api_status* status = nullptr);

  RL_ATTR(nodiscard)
  int try_get_model(const std::string& app_id,
      /* inout */ model_management::model_data& data, /* out */ bool& model_received,
      api_status* status = nullptr) override;

  RL_ATTR(nodiscard) int report_result(const uint8_t* payload, size_t size, api_status* status = nullptr) override;

  ~apim_federated_client() override;

private:
  enum class state_t
  {
    model_available,
    model_retrieved
  };

  apim_federated_client(std::unique_ptr<VW::workspace> initial_model, i_trace* trace_logger, std::unique_ptr<model_management::i_data_transport> transport);

  state_t _state;
  std::unique_ptr<VW::workspace> _current_model;
  i_trace* _trace_logger;
  std::unique_ptr<model_management::i_data_transport> _transport;
};

// Read MODEL_VW_INITIAL_COMMAND_LINE

}  // namespace reinforcement_learning