#include "federation/vw_trainable_model.h"

#include "constants.h"
#include "err_constants.h"
#include "joiners/example_joiner.h"
#include "joiners/multistep_example_joiner.h"
#include "parse_example_binary.h"
#include "vw/config/options_cli.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

namespace reinforcement_learning
{
int trainable_vw_model::create(std::unique_ptr<trainable_vw_model>& output, const utility::configuration& config,
    i_trace* trace_logger, api_status* status)
{
  int protocol_version = config.get_int(name::PROTOCOL_VERSION, 0);
  if (protocol_version != 2)
  { RETURN_ERROR_LS(trace_logger, status, invalid_argument) << "Protocol version 2 is required"; }

  std::string command_line = config.get(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  std::string problem_type = config.get(name::JOINER_PROBLEM_TYPE, value::PROBLEM_TYPE_UNKNOWN);
  std::string learning_mode = config.get(name::JOINER_LEARNING_MODE, value::LEARNING_MODE_ONLINE);
  std::string reward_function = config.get(name::JOINER_REWARD_FUNCTION, value::REWARD_FUNCTION_EARLIEST);

  output = std::unique_ptr<trainable_vw_model>(
      new trainable_vw_model(command_line, problem_type, learning_mode, reward_function));
  return error_code::success;
}

trainable_vw_model::trainable_vw_model(
    std::string command_line, std::string problem_type, std::string learning_mode, std::string reward_function)
    : _command_line(std::move(command_line))
    , _problem_type(std::move(problem_type))
    , _learning_mode(std::move(learning_mode))
    , _reward_function(std::move(reward_function))
{
  auto options = VW::make_unique<VW::config::options_cli>(VW::split_command_line(_command_line));
  _model = VW::initialize_experimental(std::move(options));
  copy_current_model_to_starting();
}

void trainable_vw_model::set_model(std::unique_ptr<VW::workspace>&& model)
{
  _model = std::move(model);
  copy_current_model_to_starting();
}

void trainable_vw_model::set_data(const model_management::model_data& data)
{
  auto opts =
      std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(VW::split_command_line(_command_line)));
  _model = VW::initialize_experimental(std::move(opts), VW::io::create_buffer_view(data.data(), data.data_sz()));
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

int trainable_vw_model::learn(std::unique_ptr<VW::io::reader>&& binary_log, api_status* status)
{
  if (binary_log.get() == nullptr)
  {
    // TODO handle this as error?
    return error_code::success;
  }

  io_buf io_reader;
  io_reader.add_file(std::move(binary_log));

  std::unique_ptr<i_joiner> joiner;
  if (_problem_type == value::PROBLEM_TYPE_MULTISTEP)
  { joiner = std::unique_ptr<i_joiner>(new example_joiner(_model.get())); }
  else
  {
    joiner = std::unique_ptr<i_joiner>(new multistep_example_joiner(_model.get()));
  }

  // Set the default joiner options if no checkpoint message is present in the binary log
  configure_joiner(joiner);

  // TODO should we have an actual logger?
  VW::external::binary_parser binary_parser(std::move(joiner), VW::io::create_null_logger());

  bool example_was_parsed = false;
  VW::multi_ex example_out;
  do
  {
    example_out.push_back(VW::new_unused_example(*_model));
    example_was_parsed = binary_parser.parse_examples(_model.get(), io_reader, example_out);

    if (example_was_parsed)
    {
      VW::setup_examples(*_model, example_out);
      if (_problem_type == value::PROBLEM_TYPE_MULTISTEP) { _model->learn(example_out); }
      else
      {
        _model->learn(*example_out[0]);
      }
    }

    for (auto* ex : example_out) { VW::finish_example(*_model, *ex); }
    example_out.clear();
  } while (example_was_parsed);

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

  auto args = VW::split_command_line(_command_line);
  if (std::find(args.begin(), args.end(), "--preserve_performance_counters") == args.end())
  { args.emplace_back("--preserve_performance_counters"); }
  auto options = VW::make_unique<VW::config::options_cli>(args);
  _starting_model = VW::initialize_experimental(std::move(options),
      VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()), nullptr, nullptr, nullptr);
}

void trainable_vw_model::configure_joiner(std::unique_ptr<i_joiner>& joiner)
{
  if (_problem_type == value::PROBLEM_TYPE_CB)
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CB);
  else if (_problem_type == value::PROBLEM_TYPE_CCB)
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CCB);
  else if (_problem_type == value::PROBLEM_TYPE_SLATES)
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_SLATES);
  else if (_problem_type == value::PROBLEM_TYPE_CA)
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CA);
  else if (_problem_type == value::PROBLEM_TYPE_MULTISTEP)
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_MULTISTEP);
  else
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_UNKNOWN);

  if (_learning_mode == value::LEARNING_MODE_APPRENTICE)
    joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_Apprentice);
  else if (_learning_mode == value::LEARNING_MODE_LOGGINGONLY)
    joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_LoggingOnly);
  else
    joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_Online);

  if (_reward_function == value::REWARD_FUNCTION_AVERAGE)
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Average);
  else if (_reward_function == value::REWARD_FUNCTION_MEDIAN)
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Median);
  else if (_reward_function == value::REWARD_FUNCTION_SUM)
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Sum);
  else if (_reward_function == value::REWARD_FUNCTION_MIN)
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Min);
  else if (_reward_function == value::REWARD_FUNCTION_MAX)
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Max);
  else
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Earliest);
  joiner->set_default_reward(0.f);
}

}  // namespace reinforcement_learning
