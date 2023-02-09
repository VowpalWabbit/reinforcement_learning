#include "federation/vw_trainable_model.h"

#include "constants.h"
#include "err_constants.h"
#include "joiners/example_joiner.h"
#include "joiners/multistep_example_joiner.h"
#include "parse_example_binary.h"
#include "str_util.h"
#include "utility/vw_logger_adapter.h"
#include "vw/config/options_cli.h"
#include "vw/core/learner.h"
#include "vw/core/parse_primitives.h"
#include "vw/core/shared_data.h"
#include "vw/core/vw.h"
#include "vw/io/logger.h"

namespace
{
// Helper function to train model on VW::multi_ex
// examples is cleared at the end of this function
// returns number of examples learned
int learn_and_finish_examples(VW::workspace& vw, VW::multi_ex& examples)
{
  if (examples.empty()) { return 0; }

  VW::setup_examples(vw, examples);

  if (vw.l->is_multiline())
  {
    vw.learn(examples);
    vw.finish_example(examples);
    examples.clear();
    return 1;
  }

  // single line
  for (auto example : examples) { vw.learn(*example); }
  for (auto example : examples) { vw.finish_example(*example); }
  int size = examples.size();
  examples.clear();
  return size;
}

// Helper function to call finish_example on VW::multi_ex
// examples is cleared at the end of this function
void finish_examples(VW::workspace& vw, VW::multi_ex& examples)
{
  if (examples.empty()) { return; }

  if (vw.l->is_multiline()) { vw.finish_example(examples); }
  else
  {
    for (auto example : examples) { vw.finish_example(*example); }
  }

  examples.clear();
}

}  // namespace

namespace reinforcement_learning
{
int trainable_vw_model::create(std::unique_ptr<trainable_vw_model>& output, const utility::configuration& config,
    i_trace* trace_logger, api_status* status)
{
  int protocol_version = config.get_int(name::PROTOCOL_VERSION, 1);
  if (protocol_version != 2)
  {
    RETURN_ERROR_LS(trace_logger, status, invalid_argument) << "Protocol version 2 is required";
  }

  std::string command_line = config.get(name::MODEL_VW_INITIAL_COMMAND_LINE, "--quiet --preserve_performance_counters");
  std::string problem_type = config.get(name::JOINER_PROBLEM_TYPE, value::PROBLEM_TYPE_UNKNOWN);
  std::string learning_mode = config.get(name::JOINER_LEARNING_MODE, value::LEARNING_MODE_ONLINE);
  std::string reward_function = config.get(name::JOINER_REWARD_FUNCTION, value::REWARD_FUNCTION_EARLIEST);

  try
  {
    output = std::unique_ptr<trainable_vw_model>(
        new trainable_vw_model(command_line, problem_type, learning_mode, reward_function, trace_logger));
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_ARG(trace_logger, status, model_update_error, e.what());
  }
  catch (...)
  {
    RETURN_ERROR_ARG(trace_logger, status, model_update_error, "Unknown error");
  }
  return error_code::success;
}

trainable_vw_model::trainable_vw_model(std::string command_line, std::string problem_type, std::string learning_mode,
    std::string reward_function, i_trace* trace_logger)
    : _command_line(std::move(command_line))
    , _problem_type(std::move(problem_type))
    , _learning_mode(std::move(learning_mode))
    , _reward_function(std::move(reward_function))
    , _trace_logger(trace_logger)
{
  auto options = VW::make_unique<VW::config::options_cli>(VW::split_command_line(_command_line));
  auto logger = utility::make_vw_trace_logger(_trace_logger);
  _model = VW::initialize_experimental(std::move(options), nullptr, nullptr, nullptr, &logger);
  copy_current_model_to_starting();
}

int trainable_vw_model::set_model(std::unique_ptr<VW::workspace>&& model, api_status* status)
{
  try
  {
    {
      std::lock_guard<std::mutex> lock(_mutex);
      _model = std::move(model);
    }
    copy_current_model_to_starting();
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, e.what());
  }
  catch (...)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, "Unknown error");
  }
  return error_code::success;
}

int trainable_vw_model::set_data(const model_management::model_data& data, api_status* status)
{
  try
  {
    auto opts =
        std::unique_ptr<VW::config::options_i>(new VW::config::options_cli(VW::split_command_line(_command_line)));
    {
      std::lock_guard<std::mutex> lock(_mutex);
      auto logger = utility::make_vw_trace_logger(_trace_logger);
      _model = VW::initialize_experimental(
          std::move(opts), VW::io::create_buffer_view(data.data(), data.data_sz()), nullptr, nullptr, &logger);
    }
    copy_current_model_to_starting();
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, e.what());
  }
  catch (...)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, "Unknown error");
  }
  return error_code::success;
}

int trainable_vw_model::get_data(model_management::model_data& data, api_status* status)
{
  try
  {
    int example_count = 0;
    io_buf io_buffer;
    auto backing_buffer = std::make_shared<std::vector<char>>();
    io_buffer.add_file(VW::io::create_vector_writer(backing_buffer));

    {
      std::lock_guard<std::mutex> lock(_mutex);
      example_count = _model->sd->weighted_labeled_examples;
      VW::save_predictor(*_model, io_buffer);
    }
    auto* destination_buffer = data.alloc(backing_buffer->size());
    std::memcpy(destination_buffer, backing_buffer->data(), backing_buffer->size());
    data.increment_refresh_count();

    TRACE_INFO(_trace_logger,
        utility::concat("trainable_vw_model::get_data() returning model trained on ", example_count, " examples"));
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, e.what());
  }
  catch (...)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, "Unknown error");
  }
  return error_code::success;
}

int trainable_vw_model::learn(std::unique_ptr<VW::io::reader>&& binary_log, api_status* status)
{
  if (binary_log.get() == nullptr)
  {
    // TODO handle this as error?
    TRACE_WARN(_trace_logger, "Received null binary log in trainable_vw_model::learn()");
    return error_code::success;
  }

  try
  {
    std::lock_guard<std::mutex> lock(_mutex);

    io_buf io_reader;
    io_reader.add_file(std::move(binary_log));

    std::unique_ptr<i_joiner> joiner;
    if (_problem_type == value::PROBLEM_TYPE_MULTISTEP)
    {
      joiner = std::unique_ptr<i_joiner>(new multistep_example_joiner(_model.get()));
    }
    else { joiner = std::unique_ptr<i_joiner>(new example_joiner(_model.get())); }

    // Set the default joiner options if no checkpoint message is present in the binary log
    configure_joiner(joiner);

    VW::external::binary_parser binary_parser(std::move(joiner), utility::make_vw_trace_logger(_trace_logger));

    int example_count = 0;
    bool example_was_parsed = false;
    VW::multi_ex example_out;
    do {
      example_out.push_back(VW::new_unused_example(*_model));
      example_was_parsed = binary_parser.parse_examples(_model.get(), io_reader, example_out);

      if (example_was_parsed)
      {
        auto has_newline = example_out.back()->is_newline;
        if (has_newline)
        {
          auto last = example_out.back();
          VW::finish_example(*_model, *last);
          example_out.pop_back();
        }

        example_count += learn_and_finish_examples(*_model, example_out);
      }
      else
      {
        // cleanup the unused example that the parser was called with
        assert(example_out.size() == 1);
        VW::finish_example(*_model, example_out);
        example_out.clear();
      }
    } while (example_was_parsed);

    TRACE_INFO(_trace_logger, utility::concat("trainable_vw_model::learn() learned on ", example_count, " examples"));
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_rank_error, e.what());
  }
  catch (...)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_rank_error, "Unknown error");
  }
  return error_code::success;
}

int trainable_vw_model::learn(VW::workspace& example_ws, VW::multi_ex& examples, api_status* status)
{
  try
  {
    std::lock_guard<std::mutex> lock(_mutex);
    VW::multi_ex examples_copied;

    // examples may be from a different workspace, and must be copied to this workspace
    for (auto example : examples)
    {
      io_buf io_writer;
      VW::parsers::cache::details::cache_temp_buffer temp_buffer;
      auto example_buffer = std::make_shared<std::vector<char>>();
      io_writer.add_file(VW::io::create_vector_writer(example_buffer));
      VW::parsers::cache::write_example_to_cache(
          io_writer, example, example_ws.example_parser->lbl_parser, example_ws.parse_mask, temp_buffer);
      io_writer.flush();

      io_buf io_reader;
      io_reader.add_file(VW::io::create_buffer_view(example_buffer->data(), example_buffer->size()));
      VW::multi_ex example_out;
      example_out.push_back(VW::new_unused_example(*_model));
      VW::parsers::cache::read_example_from_cache(_model.get(), io_reader, example_out);
      examples_copied.insert(examples_copied.end(), example_out.begin(), example_out.end());
    }

    int example_count = learn_and_finish_examples(*_model, examples_copied);
    TRACE_INFO(_trace_logger, utility::concat("trainable_vw_model::learn() learned on ", example_count, " examples"));
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_rank_error, e.what());
  }
  catch (...)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_rank_error, "Unknown error");
  }
  return error_code::success;
}

int trainable_vw_model::get_model_delta(VW::model_delta& output, api_status* status)
{
  try
  {
    int old_example_count = 0;
    int new_example_count = 0;
    VW::model_delta delta(nullptr);
    {
      std::lock_guard<std::mutex> lock(_mutex);
      old_example_count = _starting_model->sd->weighted_labeled_examples;
      new_example_count = _model->sd->weighted_labeled_examples;
      delta = *_model - *_starting_model;
    }
    copy_current_model_to_starting();
    output = std::move(delta);

    TRACE_INFO(_trace_logger,
        utility::concat("trainable_vw_model::get_model_delta() created model delta with ",
            new_example_count - old_example_count, " examples (current model: ", new_example_count,
            ", previous model: ", old_example_count, ")"));
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, e.what());
  }
  catch (...)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, "Unknown error");
  }
  return error_code::success;
}

int trainable_vw_model::get_model_delta(VW::io::writer& output, api_status* status)
{
  try
  {
    VW::model_delta delta(nullptr);
    RETURN_IF_FAIL(get_model_delta(delta, status));
    delta.serialize(output);
  }
  catch (const std::exception& e)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, e.what());
  }
  catch (...)
  {
    RETURN_ERROR_ARG(_trace_logger, status, model_update_error, "Unknown error");
  }
  return error_code::success;
}

void trainable_vw_model::copy_current_model_to_starting()
{
  auto backing_vector = std::make_shared<std::vector<char>>();
  io_buf temp_buffer;
  temp_buffer.add_file(VW::io::create_vector_writer(backing_vector));

  {
    std::lock_guard<std::mutex> lock(_mutex);
    VW::save_predictor(*_model, temp_buffer);
  }

  auto args = VW::split_command_line(_command_line);
  if (std::find(args.begin(), args.end(), "--preserve_performance_counters") == args.end())
  {
    args.emplace_back("--preserve_performance_counters");
  }
  auto options = VW::make_unique<VW::config::options_cli>(args);

  {
    std::lock_guard<std::mutex> lock(_mutex);
    auto logger = utility::make_vw_trace_logger(_trace_logger);
    _starting_model = VW::initialize_experimental(std::move(options),
        VW::io::create_buffer_view(backing_vector->data(), backing_vector->size()), nullptr, nullptr, &logger);
  }
}

void trainable_vw_model::configure_joiner(std::unique_ptr<i_joiner>& joiner) const
{
  if (_problem_type == value::PROBLEM_TYPE_CB)
  {
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CB);
  }
  else if (_problem_type == value::PROBLEM_TYPE_CCB)
  {
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CCB);
  }
  else if (_problem_type == value::PROBLEM_TYPE_SLATES)
  {
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_SLATES);
  }
  else if (_problem_type == value::PROBLEM_TYPE_CA)
  {
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CA);
  }
  else if (_problem_type == value::PROBLEM_TYPE_MULTISTEP)
  {
    joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_MULTISTEP);
  }
  else { joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_UNKNOWN); }

  if (_learning_mode == value::LEARNING_MODE_APPRENTICE)
  {
    joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_Apprentice);
  }
  else if (_learning_mode == value::LEARNING_MODE_LOGGINGONLY)
  {
    joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_LoggingOnly);
  }
  else { joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_Online); }

  if (_reward_function == value::REWARD_FUNCTION_AVERAGE)
  {
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Average);
  }
  else if (_reward_function == value::REWARD_FUNCTION_MEDIAN)
  {
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Median);
  }
  else if (_reward_function == value::REWARD_FUNCTION_SUM)
  {
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Sum);
  }
  else if (_reward_function == value::REWARD_FUNCTION_MIN)
  {
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Min);
  }
  else if (_reward_function == value::REWARD_FUNCTION_MAX)
  {
    joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Max);
  }
  else { joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Earliest); }
  joiner->set_default_reward(0.f);
}

}  // namespace reinforcement_learning
