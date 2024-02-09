#pragma once

#include "generated/v2/CaEvent_generated.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "generated/v2/MultiSlotEvent_generated.h"
#include "joined_event.h"
#include "loop.h"
#include "zstd.h"

namespace typed_event
{
template <typename T>
struct event_processor;
template <>
struct event_processor<reinforcement_learning::messages::flatbuff::v2::MultiSlotEvent>
{
  static bool is_valid(const reinforcement_learning::messages::flatbuff::v2::MultiSlotEvent& evt, const loop::loop_info& loop_info, VW::io::logger& logger)
  {
    if (evt.context() == nullptr || evt.slots() == nullptr) { return false; }

    if (evt.learning_mode() != loop_info.learning_mode_config)
    {
      logger.out_warn(
          "Online Trainer learning mode [{}] "
          "and Interaction event learning mode [{}]"
          "don't match.",
          EnumNameLearningModeType(loop_info.learning_mode_config), EnumNameLearningModeType(evt.learning_mode()));
      return false;
    }
    return true;
  }

  static reinforcement_learning::messages::flatbuff::v2::LearningModeType get_learning_mode(
    const reinforcement_learning::messages::flatbuff::v2::MultiSlotEvent& evt)
  {
    return evt.learning_mode();
  }

  static std::string get_context(const reinforcement_learning::messages::flatbuff::v2::MultiSlotEvent& evt)
  {
    return {reinterpret_cast<char const*>(evt.context()->data()), evt.context()->size()};
  }

  static joined_event::joined_event fill_in_joined_event(
    const reinforcement_learning::messages::flatbuff::v2::MultiSlotEvent& evt,
    const reinforcement_learning::messages::flatbuff::v2::Metadata& metadata,
    const TimePoint& enqueued_time_utc, std::string&& line_vec)
  {
    joined_event::MultiSlotInteraction multislot_data;
    bool is_ccb = metadata.payload_type() == reinforcement_learning::messages::flatbuff::v2::PayloadType_CCB;

    auto ccb_data = VW::make_unique<joined_event::ccb_joined_event>();
    auto slates_data = VW::make_unique<joined_event::slates_joined_event>();

    size_t slot_index = 0;
    for (auto* slot_event : *evt.slots())
    {
      VW::parsers::json::decision_service_interaction data;
      data.event_id = slot_event->id() == nullptr ? metadata.id()->str() : slot_event->id()->str();

      if (is_ccb && slot_event->id() != nullptr)
      {
        ccb_data->slot_id_to_index_map.insert(std::pair<std::string, int>(slot_event->id()->str(), slot_index));
      }

      data.actions.reserve(slot_event->action_ids()->size());
      for (const auto& a : *slot_event->action_ids()) { data.actions.emplace_back(a); }

      data.probabilities.reserve(slot_event->probabilities()->size());
      for (const auto& prob : *slot_event->probabilities()) { data.probabilities.emplace_back(prob); }

      multislot_data.interaction_data.emplace_back(std::move(data));
      slot_index++;
    }

    multislot_data.baseline_actions.assign(evt.baseline_actions()->begin(), evt.baseline_actions()->end());

    multislot_data.skip_learn = evt.deferred_action();
    multislot_data.probability_of_drop = 1.f - metadata.pass_probability();

    if (is_ccb)
    {
      ccb_data->multi_slot_interaction = multislot_data;
      return {TimePoint(enqueued_time_utc),
          {metadata.app_id() ? metadata.app_id()->str() : "", metadata.payload_type(), metadata.pass_probability(),
              metadata.encoding(), metadata.id()->str(), evt.learning_mode()},
          std::move(line_vec), std::string(evt.model_id() ? evt.model_id()->c_str() : "N/A"), std::move(ccb_data)};
    }
    else
    {
      slates_data->multi_slot_interaction = multislot_data;
      return {TimePoint(enqueued_time_utc),
          {metadata.app_id() ? metadata.app_id()->str() : "", metadata.payload_type(), metadata.pass_probability(),
              metadata.encoding(), metadata.id()->str(), evt.learning_mode()},
          std::move(line_vec), std::string(evt.model_id() ? evt.model_id()->c_str() : "N/A"), std::move(slates_data)};
    }
  }
};

template <>
struct event_processor<reinforcement_learning::messages::flatbuff::v2::CbEvent>
{
  static bool is_valid(const reinforcement_learning::messages::flatbuff::v2::CbEvent& evt,
    const loop::loop_info& loop_info, VW::io::logger& logger)
  {
    if (evt.context() == nullptr || evt.action_ids() == nullptr || evt.probabilities() == nullptr) { return false; }

    if (evt.learning_mode() != loop_info.learning_mode_config)
    {
      logger.out_warn(
          "Online Trainer learning mode [{}] "
          "and Interaction event learning mode [{}]"
          "don't match.",
          EnumNameLearningModeType(loop_info.learning_mode_config), EnumNameLearningModeType(evt.learning_mode()));
      return false;
    }
    return true;
  }

  static reinforcement_learning::messages::flatbuff::v2::LearningModeType get_learning_mode(
    const reinforcement_learning::messages::flatbuff::v2::CbEvent& evt) 
  {
    return evt.learning_mode();
  }

  static std::string get_context(const reinforcement_learning::messages::flatbuff::v2::CbEvent& evt)
  {
    return {reinterpret_cast<char const*>(evt.context()->data()), evt.context()->size()};
  }

  static joined_event::joined_event fill_in_joined_event(
      const reinforcement_learning::messages::flatbuff::v2::CbEvent& evt,
      const reinforcement_learning::messages::flatbuff::v2::Metadata& metadata,
      const TimePoint& enqueued_time_utc, std::string&& line_vec)
  {
    auto cb_data = VW::make_unique<joined_event::cb_joined_event>();

    cb_data->interaction_data.event_id = metadata.id()->str();
    cb_data->interaction_data.actions.reserve(evt.action_ids()->size());
    for (const auto& a : *evt.action_ids()) { cb_data->interaction_data.actions.emplace_back(a); }

    cb_data->interaction_data.probabilities.reserve(evt.probabilities()->size());
    for (const auto& prob : *evt.probabilities()) { cb_data->interaction_data.probabilities.emplace_back(prob); }

    cb_data->interaction_data.probability_of_drop = 1.f - metadata.pass_probability();
    cb_data->interaction_data.skip_learn = evt.deferred_action();

    return {TimePoint(enqueued_time_utc),
        {metadata.app_id() ? metadata.app_id()->str() : "", metadata.payload_type(), metadata.pass_probability(),
            metadata.encoding(), metadata.id()->str(), evt.learning_mode()},
        std::move(line_vec), std::string(evt.model_id() ? evt.model_id()->c_str() : "N/A"), std::move(cb_data)};
  }
};

template <>
struct event_processor<reinforcement_learning::messages::flatbuff::v2::CaEvent>
{
  static bool is_valid(const reinforcement_learning::messages::flatbuff::v2::CaEvent& evt,
    const loop::loop_info& loop_info, VW::io::logger& logger)
  {
    if (evt.context() == nullptr) { return false; }

    if (evt.learning_mode() != loop_info.learning_mode_config)
    {
      logger.out_warn(
          "Online Trainer learning mode [{}] "
          "and Interaction event learning mode [{}]"
          "don't match.",
          EnumNameLearningModeType(loop_info.learning_mode_config), EnumNameLearningModeType(evt.learning_mode()));
      return false;
    }
    return true;
  }

  static reinforcement_learning::messages::flatbuff::v2::LearningModeType get_learning_mode(
    const reinforcement_learning::messages::flatbuff::v2::CaEvent& evt)
  {
    return evt.learning_mode();
  }

  static std::string get_context(const reinforcement_learning::messages::flatbuff::v2::CaEvent& evt)
  {
    return {reinterpret_cast<char const*>(evt.context()->data()), evt.context()->size()};
  }

  static joined_event::joined_event fill_in_joined_event(
      const reinforcement_learning::messages::flatbuff::v2::CaEvent& evt,
      const reinforcement_learning::messages::flatbuff::v2::Metadata& metadata,
      const TimePoint& enqueued_time_utc, std::string&& line_vec)
  {
    auto ca_data = VW::make_unique<joined_event::ca_joined_event>();
    ca_data->interaction_data.event_id = metadata.id()->str();
    ca_data->interaction_data.action = evt.action();
    ca_data->interaction_data.pdf_value = evt.pdf_value();
    ca_data->interaction_data.probability_of_drop = 1.f - metadata.pass_probability();
    ca_data->interaction_data.skip_learn = evt.deferred_action();

    return {TimePoint(enqueued_time_utc),
        {metadata.app_id() ? metadata.app_id()->str() : "", metadata.payload_type(), metadata.pass_probability(),
            metadata.encoding(), metadata.id()->str(), evt.learning_mode()},
        std::move(line_vec), std::string(evt.model_id() ? evt.model_id()->c_str() : "N/A"), std::move(ca_data)};
  }
};

template <typename T>
bool process_compression(const uint8_t* data, size_t size,
  const reinforcement_learning::messages::flatbuff::v2::Metadata& metadata, const T*& payload,
  flatbuffers::DetachedBuffer& detached_buffer, VW::io::logger& logger)
{
  if (metadata.encoding() == reinforcement_learning::messages::flatbuff::v2::EventEncoding_Zstd)
  {
    size_t buff_size = ZSTD_getFrameContentSize(data, size);
    if (buff_size == ZSTD_CONTENTSIZE_ERROR)
    {
      logger.out_warn(
          "Received ZSTD_CONTENTSIZE_ERROR while "
          "decompressing event with id: "
          "[{}] of type: [{}]",
          metadata.id()->c_str(), EnumNamePayloadType(metadata.payload_type()));
      return false;
    }
    if (buff_size == ZSTD_CONTENTSIZE_UNKNOWN)
    {
      logger.out_warn(
          "Received ZSTD_CONTENTSIZE_UNKNOWN while "
          "decompressing event with id: "
          "[{}] of type: [{}]",
          metadata.id()->c_str(), EnumNamePayloadType(metadata.payload_type()));
      return false;
    }

    std::unique_ptr<uint8_t[]> buff_data(flatbuffers::DefaultAllocator().allocate(buff_size));
    size_t res = ZSTD_decompress(buff_data.get(), buff_size, data, size);

    if (ZSTD_isError(res))
    {
      logger.out_warn(
          "Received [{}] error while decompressing event with id: "
          "[{}] of type: [{}]",
          ZSTD_getErrorName(res), metadata.id()->c_str(), EnumNamePayloadType(metadata.payload_type()));
      return false;
    }

    auto data_ptr = buff_data.release();

    detached_buffer = flatbuffers::DetachedBuffer(nullptr, false, data_ptr, 0, data_ptr, res);
    payload = flatbuffers::GetRoot<T>(detached_buffer.data());
  }
  else { payload = flatbuffers::GetRoot<T>(data); }
  return true;
}
}  // namespace typed_event
