#pragma once

#include "loop.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "io/logger.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace check_stuff {
template <typename T> struct event_processor;
template <> struct event_processor<v2::CbEvent> {

  static bool is_valid(const v2::CbEvent &evt) {
    return (evt.context() == nullptr || evt.action_ids() == nullptr ||
            evt.probabilities() == nullptr);
  }

  static v2::LearningModeType get_learning_mode(const v2::CbEvent &evt) {
    return evt.learning_mode();
  }

  static std::string get_context(const v2::CbEvent &evt) {
    return {reinterpret_cast<char const *>(evt.context()->data()),
            evt.context()->size()};
  }

  static joined_event fill_in_joined_event(const v2::CbEvent &evt,
                                           const v2::Metadata &metadata,
                                           const TimePoint &enqueued_time_utc,
                                           const std::string &line_vec) {

    DecisionServiceInteraction data;
    data.eventId = metadata.id()->str();
    data.actions = {evt.action_ids()->data(),
                    evt.action_ids()->data() + evt.action_ids()->size()};
    data.probabilities = {evt.probabilities()->data(),
                          evt.probabilities()->data() +
                              evt.probabilities()->size()};
    data.probabilityOfDrop = 1.f - metadata.pass_probability();
    data.skipLearn = evt.deferred_action();

    return {TimePoint(enqueued_time_utc),
            {metadata.client_time_utc()
                 ? timestamp_to_chrono(*metadata.client_time_utc())
                 : TimePoint(),
             metadata.app_id() ? metadata.app_id()->str() : "",
             metadata.payload_type(), metadata.pass_probability(),
             metadata.encoding(), metadata.id()->str(), evt.learning_mode()},
            std::move(data),
            std::string(line_vec),
            std::string(evt.model_id() ? evt.model_id()->c_str() : "N/A")};
  }
};
} // namespace check_stuff