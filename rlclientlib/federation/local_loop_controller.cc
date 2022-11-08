#include "federation/local_loop_controller.h"

#include "constants.h"
#include "err_constants.h"
#include "federation/local_client.h"
#include "model_mgmt.h"
#include "vw/io/io_adapter.h"

namespace reinforcement_learning
{
int local_loop_controller::create_local_loop_controller(std::unique_ptr<local_loop_controller>& output,
    const reinforcement_learning::utility::configuration& config, i_trace* trace_logger, api_status* status)
{
  std::string app_id = config.get(name::APP_ID, "");
  std::string command_line = config.get(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  std::unique_ptr<i_federated_client> federated_client;
  std::unique_ptr<i_joined_log_provider> joiner;
  std::unique_ptr<trainable_vw_model> trainable_model;

  // RETURN_IF_FAIL(vw_local_joiner::create_vw_local_joiner(joiner, config, trace_logger, status));
  RETURN_IF_FAIL(local_client::create_local_client(federated_client, config, trace_logger, status));
  RETURN_IF_FAIL(trainable_vw_model::create_trainable_vw_model(trainable_model, config, trace_logger, status));

  // TODO if additional types of i_event_cache are implemented, determine which type to create from config
  auto event_source = std::unique_ptr<i_event_cache>(new event_cache_memory());

  output = std::unique_ptr<local_loop_controller>(new local_loop_controller(std::move(app_id),
      std::move(federated_client), std::move(joiner), std::move(trainable_model), std::move(event_source)));
  return error_code::success;
}

local_loop_controller::local_loop_controller(std::string app_id, std::unique_ptr<i_federated_client>&& federated_client,
    std::unique_ptr<i_joined_log_provider>&& joiner, std::unique_ptr<trainable_vw_model>&& trainable_model,
    std::unique_ptr<i_event_cache>&& event_source)
    : _app_id(std::move(app_id))
    , _federated_client(std::move(federated_client))
    , _joiner(std::move(joiner))
    , _trainable_model(std::move(trainable_model))
    , _event_source(std::move(event_source))
{
}

int local_loop_controller::update_global(api_status* status)
{
  if (_need_to_send_model_delta)
  {
    // train on any remaining events
    RETURN_IF_FAIL(update_local(status));

    // get and send the model delta
    auto delta = _trainable_model->get_model_delta();
    auto buffer = std::make_shared<std::vector<char>>();
    auto writer = VW::io::create_vector_writer(buffer);
    delta.serialize(*writer);

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
    _trainable_model->set_data(data);
    _need_to_send_model_delta = true;
  }

  return error_code::success;
}

int local_loop_controller::update_local(api_status* status)
{
  if (_event_source != nullptr)
  {
    auto events = _event_source->get_events();

    std::unique_ptr<VW::io::reader> examples;
    RETURN_IF_FAIL(_joiner->invoke_join(examples, status));
    ;
    RETURN_IF_FAIL(_trainable_model->learn(std::move(examples), status));
  }
  return error_code::success;
}

int local_loop_controller::get_data(model_management::model_data& data, api_status* status)
{
  RETURN_IF_FAIL(update_local(status));
  RETURN_IF_FAIL(_trainable_model->get_data(data, status));
  return error_code::success;
}

std::unique_ptr<i_sender> local_loop_controller::get_local_sender() { return _event_source->get_sender_proxy(); }

std::function<int(i_sender**, const utility::configuration&, error_callback_fn*, i_trace*, api_status*)>
local_loop_controller::get_local_sender_factory()
{
  return [this](i_sender** retval, const utility::configuration& cfg, error_callback_fn* error_cb,
             i_trace* trace_logger, api_status* status) {
    std::unique_ptr<i_sender> proxy = this->get_local_sender();
    *retval = proxy.release();
    return error_code::success;
  };
}

}  // namespace reinforcement_learning
