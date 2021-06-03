#pragma once

#include "example.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "io/logger.h"
#include "joined_event.h"
#include "loop.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace typed_event {
template <typename T> struct event_processor;
template <> struct event_processor<v2::CbEvent> {

  static bool is_valid(const v2::CbEvent &evt,
                       const loop::loop_info &loop_info) {
    if (!(evt.context() == nullptr || evt.action_ids() == nullptr ||
          evt.probabilities() == nullptr)) {
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

// this doesn't actually depend on the template type
static void fill_in_cb_label(const joined_event::joined_event &je,
                             v_array<example *> &examples, float reward) {
  if (je.interaction_data.actions.empty()) {
    VW::io::logger::log_warn("missing actions for event [{}]",
                             je.interaction_data.eventId);
    return;
  }

  if (je.interaction_data.probabilities.empty()) {
    VW::io::logger::log_warn("missing probabilities for event [{}]",
                             je.interaction_data.eventId);
    return;
  }

  if (std::any_of(je.interaction_data.probabilities.begin(),
                  je.interaction_data.probabilities.end(),
                  [](float p) { return std::isnan(p); })) {
    VW::io::logger::log_warn(
        "distribution for event [{}] contains invalid probabilities",
        je.interaction_data.eventId);
  }

  int index = je.interaction_data.actions[0];
  auto action = je.interaction_data.actions[0];
  auto cost = -1.f * reward;
  auto probability = je.interaction_data.probabilities[0] *
                     (1.f - je.interaction_data.probabilityOfDrop);
  auto weight = 1.f - je.interaction_data.probabilityOfDrop;

  examples[index]->l.cb.costs.push_back({cost, action, probability});
  examples[index]->l.cb.weight = weight;
}
} // namespace typed_event