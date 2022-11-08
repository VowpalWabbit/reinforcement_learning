#pragma once

#include "api_status.h"
#include "error_callback_fn.h"
#include "federation/event_cache.h"
#include "federation/federated_client.h"
#include "federation/joined_log_provider.h"
#include "federation/vw_trainable_model.h"
#include "model_mgmt.h"
#include "trace_logger.h"

#include <functional>
#include <memory>

namespace reinforcement_learning
{
class local_loop_controller : public model_management::i_data_transport
{
public:
  RL_ATTR(nodiscard)
  static int create_local_loop_controller(std::unique_ptr<local_loop_controller>& output,
      const reinforcement_learning::utility::configuration& config, i_trace* trace_logger = nullptr,
      api_status* status = nullptr);

  // Get model data in binary format
  // This will perform joining and training on any observed events, and then return the updated model
  RL_ATTR(nodiscard)
  virtual int get_data(model_management::model_data& data, api_status* status = nullptr) override;

  // This updates global state with the federated learning server.
  // If applicable, it will first report a model delta from local training.
  // Then it attempts to retreive a new global model.
  RL_ATTR(nodiscard)
  int update_global(api_status* status = nullptr);

  // Returns a i_sender proxy object to be used for sending events to the local joiner
  std::unique_ptr<i_sender> get_local_sender();
  // Returns a sender factory function bound to get_local_sender()
  std::function<int(i_sender**, const utility::configuration&, error_callback_fn*, i_trace*, api_status*)>
  get_local_sender_factory();

  virtual ~local_loop_controller() = default;

private:
  // Constructor is private because objects should be created using the factory function
  local_loop_controller(std::string app_id, std::unique_ptr<i_federated_client>&& federated_client,
      std::unique_ptr<i_joined_log_provider>&& joiner, std::unique_ptr<trainable_vw_model>&& trainable_model,
      std::unique_ptr<i_event_cache>&& event_source);

  // Internal implemetation to run joining and traning to update local state
  RL_ATTR(nodiscard)
  int update_local(api_status* status = nullptr);

  // Internal state
  std::string _app_id;
  std::unique_ptr<i_federated_client> _federated_client = nullptr;
  std::unique_ptr<i_joined_log_provider> _joiner = nullptr;
  std::unique_ptr<trainable_vw_model> _trainable_model = nullptr;
  std::unique_ptr<i_event_cache> _event_source = nullptr;

  // If the federated client has received a new global model,
  // we need to train on local events and upload a model delta.
  bool _need_to_send_model_delta = false;
};

}  // namespace reinforcement_learning
