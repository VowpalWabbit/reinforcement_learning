#include "federation/local_client.h"

#include "api_status.h"
#include "constants.h"
#include "err_constants.h"
#include "trace_logger.h"
#include "vw/config/options_cli.h"
#include "vw/core/global_data.h"
#include "vw/core/io_buf.h"
#include "vw/core/merge.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"
#include "vw/io/io_adapter.h"

reinforcement_learning::local_client::local_client(std::unique_ptr<VW::workspace> initial_model, i_trace* trace_logger)
    : _current_model(std::move(initial_model)), _state(state_t::model_available), _trace_logger(trace_logger)
{
}

reinforcement_learning::local_client::~local_client() = default;

int reinforcement_learning::local_client::try_get_model(const std::string& app_id,
    /* inout */ model_management::model_data& data, /* out */ bool& model_received, api_status* status)
{
  switch (_state)
  {
    case state_t::model_available:
    {
      io_buf buf;
      auto backing_buffer = std::make_shared<std::vector<char>>();
      buf.add_file(VW::io::create_vector_writer(backing_buffer));
      VW::save_predictor(*_current_model, buf);
      auto* dest_ptr = data.alloc(backing_buffer->size());
      std::memcpy(dest_ptr, backing_buffer->data(), backing_buffer->size());
      data.increment_refresh_count();
      model_received = true;
      _state = state_t::model_retrieved;
      // Return current model and switch into model retrieved.
    }
    break;
    case state_t::model_retrieved:
    {
      RETURN_ERROR_LS(_trace_logger, status, invalid_argument)
          << "Cannot call try_get_model again until report_result has been called.";
    }
    break;
    default:
      RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "Invalid state.";
  }
  return error_code::success;
}

int reinforcement_learning::local_client::report_result(const uint8_t* payload, size_t size, api_status* status)
{
  switch (_state)
  {
    case state_t::model_available:
    {
      RETURN_ERROR_LS(_trace_logger, status, invalid_argument)
          << "Cannot call report_result again until try_get_model has been called.";
    }
    break;
    case state_t::model_retrieved:
    {
      // Payload must be a delta
      // Apply delta to current model and move into model available state.
      auto view = VW::io::create_buffer_view(reinterpret_cast<const char*>(payload), size);
      auto delta = VW::model_delta::deserialize(*view);
      auto new_model = *_current_model + *delta;
      _current_model.reset(new_model.release());
      _state = state_t::model_available;
    }
    break;
    default:
      RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "Invalid state.";
  }
  return error_code::success;
}

int reinforcement_learning::create_local_client(const utility::configuration& config,
    /*out*/ std::unique_ptr<i_federated_client>& object, i_trace* trace_logger, api_status* status)
{
  // Create empty model based on ML args on first call
  std::string initial_command_line(config.get(
      name::MODEL_VW_INITIAL_COMMAND_LINE, "--cb_explore_adf --json --quiet --epsilon 0.0 --first_only --id N/A"));

  // TODO try catch
  auto args = VW::make_unique<VW::config::options_cli>(VW::split_command_line(initial_command_line));
  auto workspace = VW::initialize_experimental(std::move(args));

  object = VW::make_unique<local_client>(std::move(workspace), trace_logger);
  return reinforcement_learning::error_code::success;
}
