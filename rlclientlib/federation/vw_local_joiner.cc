#include "federation/vw_local_joiner.h"

#include "constants.h"
#include "err_constants.h"
#include "logger/message_type.h"
#include "logger/preamble.h"
#include "trace_logger.h"
#include "vw/config/options_cli.h"
#include "vw/core/cache.h"
#include "vw/core/parse_primitives.h"

namespace reinforcement_learning
{
vw_local_joiner::vw_local_joiner(const utility::configuration& config, i_trace* trace_logger, api_status* status)
    : _trace_logger(trace_logger)
{
  if (config.get_int(name::PROTOCOL_VERSION, 999) != 2)
  {
    TRACE_ERROR(_trace_logger, "Protocol version 2 is required");
    // TODO handle error
    // Can't return error code in constructor
  }

  std::string cmd_str = config.get(name::MODEL_VW_INITIAL_COMMAND_LINE,
      "--cb_explore_adf --driver_output_off --epsilon 0.2 --power_t 0 -l 0.001 --cb_type mtr -q :: "
      "--preserve-performance-counters");
  auto cmd_list = VW::split_command_line(cmd_str);
  auto options = VW::make_unique<VW::config::options_cli>(cmd_list);
  _joiner_workspace = VW::initialize_experimental(std::move(options));
  _joiner = VW::make_unique<example_joiner>(_joiner_workspace.get());

  std::string problem_type = config.get(name::JOINER_PROBLEM_TYPE, value::PROBLEM_TYPE_UNKNOWN);
  if (problem_type == value::PROBLEM_TYPE_CB)
    _joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CB);
  else if (problem_type == value::PROBLEM_TYPE_CCB)
    _joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CCB);
  else if (problem_type == value::PROBLEM_TYPE_SLATES)
    _joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_SLATES);
  else if (problem_type == value::PROBLEM_TYPE_CA)
    _joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_CA);
  else if (problem_type == value::PROBLEM_TYPE_MULTISTEP)
    _joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_MULTISTEP);
  else
    _joiner->set_problem_type_config(messages::flatbuff::v2::ProblemType_UNKNOWN);

  std::string learning_mode = config.get(name::JOINER_LEARNING_MODE, value::LEARNING_MODE_ONLINE);
  if (learning_mode == value::LEARNING_MODE_APPRENTICE)
    _joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_Apprentice);
  else if (learning_mode == value::LEARNING_MODE_LOGGINGONLY)
    _joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_LoggingOnly);
  else
    _joiner->set_learning_mode_config(messages::flatbuff::v2::LearningModeType_Online);

  std::string reward_function = config.get(name::JOINER_REWARD_FUNCTION, value::REWARD_FUNCTION_EARLIEST);
  if (reward_function == value::REWARD_FUNCTION_AVERAGE)
    _joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Average);
  else if (reward_function == value::REWARD_FUNCTION_MEDIAN)
    _joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Median);
  else if (reward_function == value::REWARD_FUNCTION_SUM)
    _joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Sum);
  else if (reward_function == value::REWARD_FUNCTION_MIN)
    _joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Min);
  else if (reward_function == value::REWARD_FUNCTION_MAX)
    _joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Max);
  else
    _joiner->set_reward_function(messages::flatbuff::v2::RewardFunctionType_Earliest);
  _joiner->set_default_reward(0.f);
}

int vw_local_joiner::add_events(const std::vector<buffer>& events, api_status* status)
{
  for (const auto& data : events)
  {
    logger::preamble pre;
    pre.read_from_bytes(data->preamble_begin(), pre.size());
    if (pre.msg_type != logger::message_type::fb_generic_event_collection)
    {
      RETURN_ERROR_LS(_trace_logger, status, invalid_argument)
          << " Message type " << pre.msg_type << " cannot be handled.";
    }

    auto res = messages::flatbuff::v2::GetEventBatch(data->body_begin());
    flatbuffers::Verifier verifier(data->body_begin(), data->body_filled_size());
    if (!res->Verify(verifier))
    { RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "verify failed for fb_generic_event_collection"; }

    if (res->metadata()->content_encoding()->str() != "IDENTITY")
    { RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "Can only handle IDENTITY encoding"; }

    for (auto message : *res->events())
    {
      flatbuffers::FlatBufferBuilder fbb;
      // TODO: real timestamp
      messages::flatbuff::v2::TimeStamp ts(2020, 3, 1, 10, 20, 30, 0);
      const auto* event_payload = message->payload();
      auto vec = fbb.CreateVector(event_payload->data(), event_payload->size());
      auto fb = messages::flatbuff::v2::CreateJoinedEvent(fbb, vec, &ts);
      fbb.Finish(fb);
      auto detached_buffer = fbb.Release();

      const auto* je = flatbuffers::GetRoot<messages::flatbuff::v2::JoinedEvent>(detached_buffer.data());
      _joiner->process_event(*je);
    }
  }

  return error_code::success;
}

int vw_local_joiner::invoke_join(std::unique_ptr<i_joined_log_batch>& batch_out, api_status* status)
{
  std::stringstream ss;
  ss << "Joining " << _joiner->events_in_queue() << " events." << std::endl;
  TRACE_INFO(_trace_logger, ss.str());

  std::vector<VW::example*> examples;
  std::unique_ptr<vw_joined_log_batch> joined_batch = VW::make_unique<vw_joined_log_batch>(_joiner_workspace);

  while (_joiner->processing_batch())
  {
    examples.push_back(VW::new_unused_example(*_joiner_workspace));

    // False means there was a problem and we should try reading the next one.
    if (!_joiner->process_joined(examples))
    {
      assert(examples.size() == 1);
      auto* ex_to_return = examples.back();
      examples.pop_back();
      VW::finish_example(*_joiner_workspace, *ex_to_return);
      continue;
    }
    VW::setup_examples(*_joiner_workspace, examples);

    // We must remove the trailing newline example.
    auto* newline_ex = examples.back();
    examples.pop_back();
    VW::finish_example(*_joiner_workspace, *newline_ex);

    // Add examples to batch
    for (VW::example* e : examples) { joined_batch->add_example(e); }
    examples.clear();
  }

  batch_out.reset(joined_batch.release());
  return error_code::success;
}

vw_joined_log_batch::vw_joined_log_batch(std::shared_ptr<VW::workspace> joiner_workspace)
    : _joiner_workspace(std::move(joiner_workspace))
{
}

vw_joined_log_batch::~vw_joined_log_batch()
{
  for (auto* example : _examples) { _joiner_workspace->finish_example(*example); }

  if (_output_example_ptr != nullptr)
  {
    _joiner_workspace->finish_example(*_output_example_ptr);
    _output_example_ptr = nullptr;
  }
}

void vw_joined_log_batch::add_example(VW::example* example) { _examples.push_back(example); }

int vw_joined_log_batch::next(std::unique_ptr<VW::io::reader>& chunk_reader, api_status* status)
{
  // free the previous output example, if it exists
  if (_output_example_ptr != nullptr)
  {
    _joiner_workspace->finish_example(*_output_example_ptr);
    _output_example_ptr = nullptr;
  }

  // clear output state
  chunk_reader.reset(nullptr);
  _output_example_buffer = std::make_shared<std::vector<char>>();

  // output the next example in the batch
  if (!_examples.empty())
  {
    io_buf io_writer;
    VW::details::cache_temp_buffer temp_buffer;

    VW::example* example = _examples.back();
    _examples.pop_back();

    io_writer.add_file(VW::io::create_vector_writer(_output_example_buffer));
    VW::write_example_to_cache(
        io_writer, example, _joiner_workspace->example_parser->lbl_parser, _joiner_workspace->parse_mask, temp_buffer);
    io_writer.flush();

    _output_example_ptr = example;
    chunk_reader = VW::io::create_buffer_view(_output_example_buffer->data(), _output_example_buffer->size());
  }

  return error_code::success;
}

int vw_joined_log_batch::next_example(VW::example** example_out, api_status* status)
{
  if (_examples.empty())
  {
    *example_out = nullptr;
  }
  else
  {
    *example_out = _examples.back();
    _examples.pop_back();
  }

  return error_code::success;
}

void vw_joined_log_batch::finish_example(VW::example* example)
{
  if (example != nullptr)
  {
    _joiner_workspace->finish_example(*example);
  }
}

}  // namespace reinforcement_learning
