#include "vw_trainable_model.h"

#include "constants.h"
#include "err_constants.h"

#include "vw/config/options_cli.h"
#include "vw/core/parse_primitives.h"

namespace reinforcement_learning
{

trainable_vw_model::trainable_vw_model(const utility::configuration& config)
{
  _command_line = config.get(name::MODEL_VW_INITIAL_COMMAND_LINE,
      "--cb_explore_adf --driver_output_off --epsilon 0.2 --power_t 0 -l 0.001 --cb_type mtr -q :: --preserve-performance-counters");
  auto options = VW::make_unique<VW::config::options_cli>(VW::split_command_line(_command_line));
  _model = VW::initialize_experimental(std::move(options));
  copy_current_model_to_starting();
}

void trainable_vw_model::set_model(std::unique_ptr<VW::workspace>&& model)
{
  _model = std::move(model);
  copy_current_model_to_starting();
}

int trainable_vw_model::get_data(model_management::model_data& data, api_status* status)
{
  io_buf buffer;
  auto backing_buffer = std::make_shared<std::vector<char>>();
  buffer.add_file(VW::io::create_vector_writer(backing_buffer));
  VW::save_predictor(*_model, buffer);

  auto* buffer_to_copy_to = data.alloc(backing_buffer->size());
  std::memcpy(buffer_to_copy_to, backing_buffer->data(), backing_buffer->size());

  return error_code::success;
}

int trainable_vw_model::learn(vw_joined_log_batch& joined_logs, api_status* status)
{
  VW::example *example = nullptr;
  while (joined_logs.next_example(&example, status), example != nullptr)
  {
    _model->learn(*example);
    joined_logs.finish_example(example);
  }
  return error_code::success;
}

VW::model_delta trainable_vw_model::get_model_delta()
{
  auto delta = *_model - *_starting_model;
  copy_current_model_to_starting();
  return delta;
}

void trainable_vw_model::copy_current_model_to_starting()
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf temp_buffer;
  temp_buffer.add_file(VW::io::create_vector_writer(backing_vector));
  VW::save_predictor(*_model, temp_buffer);

  auto options = VW::make_unique<VW::config::options_cli>(VW::split_command_line(_command_line));
  _starting_model = VW::initialize_experimental(std::move(options), VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()), nullptr, nullptr, nullptr);
}

}
