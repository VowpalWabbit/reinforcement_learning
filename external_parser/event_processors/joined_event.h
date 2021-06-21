#pragma once

// FileFormat_generated.h used for the payload type and encoding enum's
#include "generated/v2/FileFormat_generated.h"

#include "event_processors/timestamp_helper.h"
// VW headers
// vw.h has to come before json_utils.h
// clang-format off
#include "vw.h"
#include "json_utils.h"
#include "example.h"
#include "io/logger.h"
// clang-format on

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace joined_event {

struct metadata_info {
  TimePoint client_time_utc;
  std::string app_id;
  v2::PayloadType payload_type;
  float pass_probability;
  v2::EventEncoding event_encoding;
  std::string event_id;
  v2::LearningModeType learning_mode;
};

struct outcome_event {
  metadata_info metadata;
  std::string s_index;
  int index;
  std::string s_value;
  float value;
  TimePoint enqueued_time_utc;
  bool action_taken;
};

struct typed_joined_event {
  virtual ~typed_joined_event() = default;
  virtual bool is_skip_learn() const = 0;
  virtual void set_skip_learn(bool sl) = 0;
  virtual bool should_calculate_apprentice_reward() const = 0;
  virtual void fill_in_label(v_array<example *> &examples) const = 0;
  virtual void set_cost(v_array<example *> &examples, float reward,
                        size_t index = 0) const = 0;
};

struct cb_joined_event : public typed_joined_event {
  DecisionServiceInteraction interaction_data;
  // Default Baseline Action for CB is 1 (rl client recommended actions are 1
  // indexed in the CB case)
  static const int baseline_action = 1;

  ~cb_joined_event() = default;

  bool is_skip_learn() const override { return interaction_data.skipLearn; }

  void set_skip_learn(bool sl) override { interaction_data.skipLearn = sl; }

  bool should_calculate_apprentice_reward() const override {
    return (!interaction_data.actions.empty() &&
            interaction_data.actions[0] == baseline_action);
  }

  void fill_in_label(v_array<example *> &examples) const override {

    if (interaction_data.actions.empty()) {
      VW::io::logger::log_warn("missing actions for event [{}]",
                               interaction_data.eventId);
      return;
    }

    if (interaction_data.probabilities.empty()) {
      VW::io::logger::log_warn("missing probabilities for event [{}]",
                               interaction_data.eventId);
      return;
    }

    if (std::any_of(interaction_data.probabilities.begin(),
                    interaction_data.probabilities.end(),
                    [](float p) { return std::isnan(p); })) {
      VW::io::logger::log_warn(
          "distribution for event [{}] contains invalid probabilities",
          interaction_data.eventId);
    }

    int index = interaction_data.actions[0];
    auto action = interaction_data.actions[0];
    auto probability = interaction_data.probabilities[0] *
                       (1.f - interaction_data.probabilityOfDrop);
    auto weight = 1.f - interaction_data.probabilityOfDrop;

    examples[index]->l.cb.costs.push_back({0.f, action, probability});
    examples[index]->l.cb.weight = weight;
  }

  void set_cost(v_array<example *> &examples, float reward,
                size_t index = 0) const override {
    if (interaction_data.actions.empty()) {
      return;
    }
    index = interaction_data.actions[0];
    if (examples.size() <= index) {
      VW::io::logger::log_warn(
          "trying to set index [{}] when there are [{}] examples", index,
          examples.size());
      return;
    }
    examples[index]->l.cb.costs[0].cost = -1.f * reward;
  }
};

struct ccb_joined_event : public typed_joined_event {
  std::vector<DecisionServiceInteraction> interaction_data;
  static const std::vector<int> baseline_actions;
  std::map<std::string, int> slot_id_to_index_map;

  ~ccb_joined_event() = default;
  // TODO fill in
  bool is_skip_learn() const override { return false; }
  void set_skip_learn(bool) override {}
  bool should_calculate_apprentice_reward() const override { return false; }
  void fill_in_label(v_array<example *> &examples) const override {

    // index to interaction_data vector which holds per-slot info
    size_t slot_index = 0;

    for (auto *ex : examples) {
      if (ex->l.conditional_contextual_bandit.type == CCB::example_type::slot) {
        auto &slot_label = ex->l.conditional_contextual_bandit;
        if (interaction_data.size() > slot_index) {
          const auto &slot_data = interaction_data[slot_index];
          if ((slot_data.actions.size() != 0) &&
              (slot_data.probabilities.size() != 0)) {
            auto outcome = new CCB::conditional_contextual_bandit_outcome();
            // outcome->cost = TODO assing reward for slot;

            if (slot_data.actions.size() != slot_data.probabilities.size()) {
              VW::io::logger::log_warn(
                  "actions and probabilities for event [{}] don't have the "
                  "same size. Actions [{}], probabilities [{}]",
                  slot_data.eventId, slot_data.actions.size(),
                  slot_data.probabilities.size());
              continue;
            }

            for (size_t i = 0; i < slot_data.actions.size(); i++) {
              outcome->probabilities.push_back(
                  {slot_data.actions[i], slot_data.probabilities[i]});
            }
            slot_label.outcome = outcome;
          }
        }
        // process next slot from interaction_data vector
        slot_index++;
      }
    }
  }

  void set_cost(v_array<example *> &examples, float reward,
                size_t slot_index = 0) const override {
    if (examples.size() <= slot_index) {
      VW::io::logger::log_warn(
          "trying to set index [{}] when there are [{}] examples", slot_index,
          examples.size());
      return;
    }

    size_t index = 0;
    for (auto &example : examples) {
      if (example->l.conditional_contextual_bandit.type !=
          CCB::example_type::slot) {
        index++;
        continue;
      }
    }

    size_t example_index = index + slot_index;
    if (example_index >= examples.size()) {
      VW::io::logger::log_error(
        "slot index is out of examples range");
    }

    if (examples[example_index]->l.conditional_contextual_bandit.type ==
      CCB::example_type::slot) {
      examples[example_index]->l.conditional_contextual_bandit.outcome->cost =
        -1.f * reward;
    } else {
      VW::io::logger::log_warn(
        "trying to set index [{}] on a CCB non-slot example", index);
    }
  }
};

struct joined_event {
  joined_event(TimePoint &&tp, metadata_info &&mi, std::string &&ctx,
               std::string &&mid, std::unique_ptr<typed_joined_event> &&data)
      : joined_event_timestamp(std::move(tp)),
        interaction_metadata(std::move(mi)), context(std::move(ctx)),
        model_id(std::move(mid)), typed_data(std::move(data)),
        outcome_events({}), ok(true) {}
  joined_event() : ok(true) {}

  TimePoint joined_event_timestamp;
  metadata_info interaction_metadata;
  std::string context;
  std::string model_id;
  std::unique_ptr<typed_joined_event> typed_data;
  std::vector<outcome_event> outcome_events;
  bool ok; // ok till proved otherwise

  const typed_joined_event *get_hold_of_typed_data() const {
    return typed_data.get();
  }

  bool should_calculate_apprentice_reward() const {
    return typed_data->should_calculate_apprentice_reward();
  }

  void fill_in_label(v_array<example *> &examples) const {
    typed_data->fill_in_label(examples);
  }

  void set_cost(v_array<example *> &examples, float reward,
                size_t index = 0) const {
    typed_data->set_cost(examples, reward, index);
  }

  bool is_joined_event_learnable() {
    bool deferred_action = typed_data->is_skip_learn();

    if (!deferred_action) {
      return true;
    }

    bool outcome_activated = std::any_of(
        outcome_events.begin(), outcome_events.end(),
        [](const outcome_event &o) { return o.action_taken == true; });

    if (outcome_activated) {
      typed_data->set_skip_learn(false);
      return true;
    } else {
      return false;
    }
  }

  bool should_calculate_reward() {
    return outcome_events.size() > 0 &&
           std::any_of(
               outcome_events.begin(), outcome_events.end(),
               [](const outcome_event &o) { return o.action_taken != true; });
  }

  void convert_outcome_slot_id_to_index() {
    const auto &ccb = reinterpret_cast<const ccb_joined_event *>(get_hold_of_typed_data());
    const auto &id_map = ccb->slot_id_to_index_map;

    // TODO: convert s_index to index
    for (auto &outcome : outcome_events) {
      if (!outcome.s_index.empty()) {
        auto it = id_map.find(outcome.s_index);
        if (it != id_map.end()) {

        }
      }
    }
  }
};
} // namespace joined_event