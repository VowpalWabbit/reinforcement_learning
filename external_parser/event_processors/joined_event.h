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
  virtual bool is_skip_learn() const = 0;
  virtual void set_skip_learn(bool sl) = 0;
  virtual bool should_learn_from_apprentice() const = 0;
  virtual void fill_in_label(v_array<example *> &examples,
                             float reward) const = 0;
};

struct cb_joined_event : public typed_joined_event {
  DecisionServiceInteraction interaction_data;
  // Default Baseline Action for CB is 1 (rl client recommended actions are 1
  // indexed in the CB case)
  static const int baseline_action = 1;

  bool is_skip_learn() const override { return interaction_data.skipLearn; }

  void set_skip_learn(bool sl) override { interaction_data.skipLearn = sl; }

  bool should_learn_from_apprentice() const override {
    return (!interaction_data.actions.empty() &&
            interaction_data.actions[0] == baseline_action);
  }

  void fill_in_label(v_array<example *> &examples,
                     float reward) const override {

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
    auto cost = -1.f * reward;
    auto probability = interaction_data.probabilities[0] *
                       (1.f - interaction_data.probabilityOfDrop);
    auto weight = 1.f - interaction_data.probabilityOfDrop;

    examples[index]->l.cb.costs.push_back({cost, action, probability});
    examples[index]->l.cb.weight = weight;
  }
};

struct ccb_joined_event : public typed_joined_event {
  std::vector<DecisionServiceInteraction> interaction_data;
  static const std::vector<int> baseline_actions;
  // TODO fill in
  bool is_skip_learn() const override { return false; }
  void set_skip_learn(bool) override {}
  bool should_learn_from_apprentice() const override { return false; }
  void fill_in_label(v_array<example *> &examples,
                     float reward) const override {

    // if label is ok
    for (auto *ex : examples) {
      if (ex->l.conditional_contextual_bandit.type == CCB::example_type::slot) {
        auto &ld = ex->l.conditional_contextual_bandit;
        // TODO fill in with all the info from interaction data vector
      }
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

  bool should_learn_from_apprentice() const {
    return typed_data->should_learn_from_apprentice();
  }

  void fill_in_label(v_array<example *> &examples, float reward) const {
    typed_data->fill_in_label(examples, reward);
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
};
} // namespace joined_event