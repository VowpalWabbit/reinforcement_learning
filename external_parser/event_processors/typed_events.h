#pragma once

#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "generated/v2/MultiSlotEvent_generated.h"
#include "joined_event.h"
#include "loop.h"
#include "zstd.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace typed_event {
template <typename T> struct event_processor;
template <> struct event_processor<v2::MultiSlotEvent> {
  static bool is_valid(const v2::MultiSlotEvent &, const loop::loop_info &) {
    return true;
  }

  static v2::LearningModeType get_learning_mode(const v2::MultiSlotEvent &evt) {
    return evt.learning_mode();
  }

  static std::string get_context(const v2::MultiSlotEvent &evt) {
    return {reinterpret_cast<char const *>(evt.context()->data()),
            evt.context()->size()};
  }

  static joined_event::joined_event fill_in_joined_event(
      const v2::MultiSlotEvent &evt, const v2::Metadata &metadata,
      const TimePoint &enqueued_time_utc, std::string &&line_vec) {

    auto ccb_data = VW::make_unique<joined_event::ccb_joined_event>();
    for (auto *slot_event : *evt.slots()) {

      DecisionServiceInteraction data;
      data.eventId = slot_event->id() == nullptr ? metadata.id()->str()
                                                 : slot_event->id()->str();
      data.actions.reserve(slot_event->action_ids()->size());
      for (const auto &a : *slot_event->action_ids()) {
        data.actions.emplace_back(a);
      }

      data.probabilities.reserve(slot_event->probabilities()->size());
      for (const auto &prob : *slot_event->probabilities()) {
        data.probabilities.emplace_back(prob);
      }

      data.probabilityOfDrop = 1.f - metadata.pass_probability();
      data.skipLearn = evt.deferred_action();
      ccb_data->interaction_data.emplace_back(std::move(data));
    }

    return {TimePoint(enqueued_time_utc),
            {metadata.client_time_utc()
                 ? timestamp_to_chrono(*metadata.client_time_utc())
                 : TimePoint(),
             metadata.app_id() ? metadata.app_id()->str() : "",
             metadata.payload_type(), metadata.pass_probability(),
             metadata.encoding(), metadata.id()->str(), evt.learning_mode()},
            std::move(line_vec),
            std::string(evt.model_id() ? evt.model_id()->c_str() : "N/A"),
            std::move(ccb_data)};
  }
};

template <> struct event_processor<v2::CbEvent> {
  static bool is_valid(const v2::CbEvent &evt,
                       const loop::loop_info &loop_info) {
    if (evt.context() == nullptr || evt.action_ids() == nullptr ||
        evt.probabilities() == nullptr) {
      return false;
    }

    if (evt.learning_mode() != loop_info.learning_mode_config) {
      VW::io::logger::log_warn(
          "Online Trainer learning mode [{}] "
          "and Interaction event learning mode [{}]"
          "don't match.",
          EnumNameLearningModeType(loop_info.learning_mode_config),
          EnumNameLearningModeType(evt.learning_mode()));
      return false;
    }
    return true;
  }

  static v2::LearningModeType get_learning_mode(const v2::CbEvent &evt) {
    return evt.learning_mode();
  }

  static std::string get_context(const v2::CbEvent &evt) {
    return {reinterpret_cast<char const *>(evt.context()->data()),
            evt.context()->size()};
  }

  static joined_event::joined_event
  fill_in_joined_event(const v2::CbEvent &evt, const v2::Metadata &metadata,
                       const TimePoint &enqueued_time_utc,
                       std::string &&line_vec) {

    auto cb_data = VW::make_unique<joined_event::cb_joined_event>();

    cb_data->interaction_data.eventId = metadata.id()->str();
    cb_data->interaction_data.actions.reserve(evt.action_ids()->size());
    for (const auto &a : *evt.action_ids()) {
      cb_data->interaction_data.actions.emplace_back(a);
    }

    cb_data->interaction_data.probabilities.reserve(
        evt.probabilities()->size());
    for (const auto &prob : *evt.probabilities()) {
      cb_data->interaction_data.probabilities.emplace_back(prob);
    }

    cb_data->interaction_data.probabilityOfDrop =
        1.f - metadata.pass_probability();
    cb_data->interaction_data.skipLearn = evt.deferred_action();

    return {TimePoint(enqueued_time_utc),
            {metadata.client_time_utc()
                 ? timestamp_to_chrono(*metadata.client_time_utc())
                 : TimePoint(),
             metadata.app_id() ? metadata.app_id()->str() : "",
             metadata.payload_type(), metadata.pass_probability(),
             metadata.encoding(), metadata.id()->str(), evt.learning_mode()},
            std::move(line_vec),
            std::string(evt.model_id() ? evt.model_id()->c_str() : "N/A"),
            std::move(cb_data)};
  }
};

template <typename T>
bool process_compression(const uint8_t *data, size_t size,
                         const v2::Metadata &metadata, const T *&payload,
                         flatbuffers::DetachedBuffer &detached_buffer) {

  if (metadata.encoding() == v2::EventEncoding_Zstd) {
    size_t buff_size = ZSTD_getFrameContentSize(data, size);
    if (buff_size == ZSTD_CONTENTSIZE_ERROR) {
      VW::io::logger::log_warn("Received ZSTD_CONTENTSIZE_ERROR while "
                               "decompressing event with id: "
                               "[{}] of type: [{}]",
                               metadata.id()->c_str(), metadata.payload_type());
      return false;
    }
    if (buff_size == ZSTD_CONTENTSIZE_UNKNOWN) {
      VW::io::logger::log_warn("Received ZSTD_CONTENTSIZE_UNKNOWN while "
                               "decompressing event with id: "
                               "[{}] of type: [{}]",
                               metadata.id()->c_str(), metadata.payload_type());
      return false;
    }

    std::unique_ptr<uint8_t[]> buff_data(
        flatbuffers::DefaultAllocator().allocate(buff_size));
    size_t res = ZSTD_decompress(buff_data.get(), buff_size, data, size);

    if (ZSTD_isError(res)) {
      VW::io::logger::log_warn(
          "Received [{}] error while decompressing event with id: "
          "[{}] of type: [{}]",
          ZSTD_getErrorName(res), metadata.id()->c_str(),
          metadata.payload_type());
      return false;
    }

    auto data_ptr = buff_data.release();

    detached_buffer =
        flatbuffers::DetachedBuffer(nullptr, false, data_ptr, 0, data_ptr, res);
    payload = flatbuffers::GetRoot<T>(detached_buffer.data());

  } else {
    payload = flatbuffers::GetRoot<T>(data);
  }
  return true;
}
} // namespace typed_event