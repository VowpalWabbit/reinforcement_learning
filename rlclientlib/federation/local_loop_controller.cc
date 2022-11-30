#include "federation/local_loop_controller.h"

#include "constants.h"
#include "err_constants.h"
#include "federation/local_client.h"
#include "federation/sender_joined_log_provider.h"
#include "model_mgmt.h"
#include "vw/io/io_adapter.h"

namespace reinforcement_learning
{
int local_loop_controller::create(std::unique_ptr<local_loop_controller>& output,
    const reinforcement_learning::utility::configuration& config, i_trace* trace_logger, api_status* status)
{
  std::string app_id = config.get(name::APP_ID, "");

  std::unique_ptr<i_federated_client> federated_client;
  std::unique_ptr<trainable_vw_model> trainable_model;
  std::unique_ptr<sender_joined_log_provider> sender_joiner;
  RETURN_IF_FAIL(local_client::create(federated_client, config, trace_logger, status));
  RETURN_IF_FAIL(trainable_vw_model::create(trainable_model, config, trace_logger, status));
  RETURN_IF_FAIL(sender_joined_log_provider::create(sender_joiner, config, trace_logger, status));

  // sender_joiner is both an i_joined_log_provider and an i_event_sink
  // we need to convert to shared_ptr and create copies for each base type
  std::shared_ptr<sender_joined_log_provider> sender_joiner_shared;
  std::shared_ptr<i_joined_log_provider> joiner;
  std::shared_ptr<i_event_sink> event_sink;
  sender_joiner_shared = std::move(sender_joiner);
  joiner = std::static_pointer_cast<i_joined_log_provider>(sender_joiner_shared);
  event_sink = std::static_pointer_cast<i_event_sink>(sender_joiner_shared);

  output = std::unique_ptr<local_loop_controller>(new local_loop_controller(std::move(app_id),
      std::move(federated_client), std::move(trainable_model), std::move(joiner), std::move(event_sink)));
  return error_code::success;
}

local_loop_controller::local_loop_controller(std::string app_id, std::unique_ptr<i_federated_client>&& federated_client,
    std::unique_ptr<trainable_vw_model>&& trainable_model, std::shared_ptr<i_joined_log_provider>&& joiner,
    std::shared_ptr<i_event_sink>&& event_sink)
    : _app_id(std::move(app_id))
    , _federated_client(std::move(federated_client))
    , _trainable_model(std::move(trainable_model))
    , _joiner(std::move(joiner))
    , _event_sink(std::move(event_sink))
{
}

int local_loop_controller::update_global(api_status* status)
{
  if (_need_to_send_model_delta)
  {
    // get and send the model delta
    auto buffer = std::make_shared<std::vector<char>>();
    auto writer = VW::io::create_vector_writer(buffer);
    RETURN_IF_FAIL(_trainable_model->get_model_delta(*writer, status));

    char* data_ptr = buffer->data();
    RETURN_IF_FAIL(_federated_client->report_result(reinterpret_cast<uint8_t*>(data_ptr), buffer->size(), status));

    _need_to_send_model_delta = false;
  }

  // ask for a new global model
  model_management::model_data data;
  bool model_received = false;
  RETURN_IF_FAIL(_federated_client->try_get_model(_app_id, data, model_received, status));

  if (model_received)
  {
    RETURN_IF_FAIL(_trainable_model->set_data(data, status));
    _need_to_send_model_delta = true;
  }
  return error_code::success;
}

int local_loop_controller::update_local(api_status* status)
{
  std::unique_ptr<VW::io::reader> binary_log;
  RETURN_IF_FAIL(_joiner->invoke_join(binary_log, status));
  RETURN_IF_FAIL(_trainable_model->learn(std::move(binary_log), status));
  return error_code::success;
}

int local_loop_controller::get_data(model_management::model_data& data, api_status* status)
{
  RETURN_IF_FAIL(update_local(status));
  RETURN_IF_FAIL(update_global(status));
  RETURN_IF_FAIL(_trainable_model->get_data(data, status));
  return error_code::success;
}

std::unique_ptr<i_sender> local_loop_controller::get_local_sender() { return _event_sink->get_sender_proxy(); }

sender_factory_t::create_fn local_loop_controller::get_local_sender_factory()
{
  return [this](i_sender** retval, const utility::configuration& cfg, error_callback_fn* error_cb,
             i_trace* trace_logger, api_status* status) {
    std::unique_ptr<i_sender> proxy = this->get_local_sender();
    *retval = proxy.release();
    return error_code::success;
  };
}

}  // namespace reinforcement_learning
