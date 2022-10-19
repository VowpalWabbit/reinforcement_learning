#include "local_loop.h"

#include "err_constants.h"
#include "constants.h"
#include "api_status.h"
#include "vw/core/parse_primitives.h"
#include "vw/config/options_cli.h"
#include "../../rlclientlib/logger/preamble.h"
#include "../../rlclientlib/logger/message_type.h"
#include "trace_logger.h"

namespace rl = reinforcement_learning;


void local_model::set_trace_logger(rl::i_trace *trace_logger) {
    _trace_logger = trace_logger;
  }

  // Get data causes a "join" to take place.
  int local_model::get_data(reinforcement_learning::model_management::model_data &data,
               reinforcement_learning::api_status *status) {
    std::lock_guard<std::mutex> lock(_mutex);
    data.increment_refresh_count();

    // If this is called before init is done then exit early.
    if (!_joiner)
    {
      return rl::error_code::success;
    }

    std::stringstream ss;
    ss << "Joining " << _joiner->events_in_queue() << " events." <<std::endl;
    _trace_logger->log(rl::LEVEL_INFO, ss.str());

    VW::multi_ex ex;
    while(_joiner->processing_batch())
    {
      ex.push_back(VW::new_unused_example(*_training_workspace));

      // False means there was a problem and we should try reading the next one.
      if(!_joiner->process_joined(ex)){
        assert(ex.size() == 1);
        auto* ex_to_return = ex.back();
        ex.pop_back();
        VW::finish_example(*_training_workspace, *ex_to_return);
        continue;
      }
      VW::setup_examples(*_training_workspace, ex);

      // We must remove the trailing newline example.
      auto* newline_ex = ex.back();
      ex.pop_back();
      VW::finish_example(*_training_workspace, *newline_ex);

      // Learn and finish the given examples
      _training_workspace->learn(ex);
      _training_workspace->finish_example(ex);
      ex.clear();
    }

    // Clear all joined buffers.
    _detached_buffers.clear();

    // Save current model state to a buffer and return to client.
    io_buf buffer;
    auto backing_buffer = std::make_shared<std::vector<char>>();
    buffer.add_file(VW::io::create_vector_writer(backing_buffer));
    VW::save_predictor(*_training_workspace, buffer);

    auto* buffer_to_copy_to = data.alloc(backing_buffer->size());
    std::memcpy(buffer_to_copy_to, backing_buffer->data(), backing_buffer->size());

    return rl::error_code::success;
  }

  int local_model::init(const reinforcement_learning::utility::configuration &config,
           reinforcement_learning::api_status *status) {

    if (config.get_int(rl::name::PROTOCOL_VERSION, 999) != 2)
    {
      RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << " protocol version 2 required";
    }

    if (!_vw_model) {
      _vw_model = std::unique_ptr<rl::model_management::vw_model>(
          new rl::model_management::vw_model(_trace_logger, config));
    }
    std::string cmd_str = config.get(
        rl::name::MODEL_VW_INITIAL_COMMAND_LINE,
        "--cb_explore_adf --driver_output_off --epsilon 0.2 --power_t 0 -l 0.001 --cb_type mtr -q ::");
    auto cmd_list = VW::split_command_line(cmd_str);
    auto options = VW::make_unique<VW::config::options_cli>(cmd_list);
    _training_workspace = VW::initialize_experimental(std::move(options));
    _joiner = VW::make_unique<example_joiner>(_training_workspace.get());
    _joiner->set_problem_type_config(rl::messages::flatbuff::v2::ProblemType_CB);
    _joiner->set_learning_mode_config(rl::messages::flatbuff::v2::LearningModeType_Online);
    _joiner->set_reward_function(rl::messages::flatbuff::v2::RewardFunctionType_Earliest);
    _joiner->set_default_reward(0.f);
    return rl::error_code::success;
  }

  int local_model::v_send(const buffer &data,
             reinforcement_learning::api_status *status) {
    std::lock_guard<std::mutex> lock(_mutex);
    rl::logger::preamble pre;
    pre.read_from_bytes(data->preamble_begin(), pre.size());

    if (pre.msg_type == rl::logger::message_type::fb_generic_event_collection) {
      auto res = reinforcement_learning::messages::flatbuff::v2::GetEventBatch(
          data->body_begin());
      flatbuffers::Verifier verifier(data->body_begin(),
                                     data->body_filled_size());
      auto result = res->Verify(verifier);

      if (!result)
      {
        RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "verify failed for fb_generic_event_collection";
      }

      if (res->metadata()->content_encoding()->str() != "IDENTITY") {
        RETURN_ERROR_LS(_trace_logger, status, invalid_argument) << "Can only handle IDENTITY encoding";
      }

      for (auto message : *res->events()) {
        flatbuffers::FlatBufferBuilder fbb;
        // TODO: real timestamp
        rl::messages::flatbuff::v2::TimeStamp ts(2020, 3, 1, 10, 20, 30, 0);
        const auto *event_payload = message->payload();
        auto vec = fbb.CreateVector(event_payload->data(), event_payload->size());
        auto fb = rl::messages::flatbuff::v2::CreateJoinedEvent(fbb, vec, &ts);
        fbb.Finish(fb);
        _detached_buffers.push_back(fbb.Release());

        const auto *je = flatbuffers::GetRoot<rl::messages::flatbuff::v2::JoinedEvent>(
            _detached_buffers.back().data());
        _joiner->process_event(*je);
      }
      return rl::error_code::success;
    }

    RETURN_ERROR_LS(_trace_logger, status, invalid_argument)
        << " Message type " << pre.msg_type << " cannot be handled.";
  }
