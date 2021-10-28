#pragma once

#include "reward.h"
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
  struct MultiSlotInteraction {
  std::vector<DecisionServiceInteraction> interaction_data;
  std::vector<unsigned> baseline_actions;
  bool skip_learn;
  float probability_of_drop {0.f};
};

inline void calculate_multislot_interaction_metrics(dsjson_metrics* metrics,
                                                       MultiSlotInteraction multi_slot_interaction,
                                                       float first_slot_original_reward_neg) {
    if(metrics) {
      metrics->DsjsonSumCostOriginalFirstSlot += first_slot_original_reward_neg;

      if (!multi_slot_interaction.interaction_data.empty() &&
          !multi_slot_interaction.interaction_data[0].actions.empty() &&
          !multi_slot_interaction.baseline_actions.empty()) {

        if (multi_slot_interaction.interaction_data[0].actions[0]
            == multi_slot_interaction.baseline_actions[0])  {
          metrics->DsjsonNumberOfLabelEqualBaselineFirstSlot++;
          metrics->DsjsonSumCostOriginalLabelEqualBaselineFirstSlot += first_slot_original_reward_neg;
        }
        else {
          metrics->DsjsonNumberOfLabelNotEqualBaselineFirstSlot++;
        }
      }
    }
  }

struct typed_joined_event {
  virtual ~typed_joined_event() = default;
  virtual bool is_skip_learn() const = 0;
  virtual void set_skip_learn(bool sl) = 0;
  virtual void set_apprentice_reward() = 0;
  virtual bool fill_in_label(v_array<example *> &examples) const = 0;
  virtual void
  calc_cost(float default_reward,
                    reward::RewardFunctionType reward_function,
                    const metadata::event_metadata_info &interaction_metadata,
                    // TODO outcome_events should also idealy be const here but
                    // we currently need it for ccb calculation
                    std::vector<reward::outcome_event> &outcome_events) = 0;

  virtual void calculate_metrics(dsjson_metrics*) {}
  virtual float get_sum_original_reward() const = 0;
};

struct cb_joined_event : public typed_joined_event {
  DecisionServiceInteraction interaction_data;
  // Default Baseline Action for CB is 1 (rl client recommended actions are 1
  // indexed in the CB case)
  static const unsigned CB_BASELINE_ACTION = 1;
  float reward = 0.0f;
  float original_reward = 0.0f;

  ~cb_joined_event() = default;

  bool is_skip_learn() const override { return interaction_data.skipLearn; }

  void set_skip_learn(bool sl) override { interaction_data.skipLearn = sl; }

  void set_apprentice_reward() override {
    if (!interaction_data.actions.empty() &&
        interaction_data.actions[0] == CB_BASELINE_ACTION)
    {
      // TODO: default apprenticeReward should come from config
      // setting to default reward matches current behavior for now
      reward = original_reward;
    }
  }

  bool fill_in_label(v_array<example *> &examples) const override {
    if (interaction_data.actions.empty()) {
      VW::io::logger::log_warn("missing actions for event [{}]",
                               interaction_data.eventId);
      return false;
    }

    if (interaction_data.probabilities.empty()) {
      VW::io::logger::log_warn("missing probabilities for event [{}]",
                               interaction_data.eventId);
      return false;
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
    auto probability = interaction_data.probabilities[0];
    if (interaction_data.probabilityOfDrop >= 1.f || interaction_data.probabilityOfDrop < 0) {
      VW::io::logger::log_warn("Probability of drop should be within [0, 1): [{}]",
                               interaction_data.eventId);
      return false;
    }

    examples[index]->l.cb.costs.push_back({-1.f * reward, action, probability});
    auto weight = 1.f / (1.f - interaction_data.probabilityOfDrop);

    for (auto *e : examples) {
      e->l.cb.weight = weight;
    }

    return true;
  }

  void calc_cost(
    float default_reward,
    reward::RewardFunctionType reward_function,
    const metadata::event_metadata_info &interaction_metadata,
    std::vector<reward::outcome_event> &outcome_events) override {
    reward = default_reward;
    // original reward is used to record the observed reward of apprentice mode
    original_reward = reward_function(outcome_events, default_reward);

    if (interaction_metadata.learning_mode == v2::LearningModeType_Apprentice) {
      set_apprentice_reward();
    } else {
      reward = original_reward;
    }
  }

  void calculate_metrics(dsjson_metrics* metrics) override {
    if (metrics) {
      if (interaction_data.actions.size() == 0) {
        metrics->NumberOfEventsZeroActions++;
      }
      else if (interaction_data.actions[0] == CB_BASELINE_ACTION) {
        metrics->DsjsonSumCostOriginalBaseline += -1. * original_reward;
      }
    }
  }

  float get_sum_original_reward() const override {
    return original_reward;
  }

  bool should_calculate_reward(
      const std::vector<reward::outcome_event> &outcome_events) {
    return outcome_events.size() > 0 &&
           std::any_of(outcome_events.begin(), outcome_events.end(),
                       [](const reward::outcome_event &o) {
                         return o.action_taken != true;
                       });
  }

  void calc_and_set_cost(
      v_array<example *> &examples, float default_reward,
      reward::RewardFunctionType reward_function,
      const metadata::event_metadata_info &interaction_metadata,
      std::vector<reward::outcome_event> &outcome_events) override {
    reward = default_reward;
    // original reward is used to record the observed reward of apprentice mode
    original_reward = default_reward;

    if (should_calculate_reward(outcome_events)) {
      original_reward = reward_function(outcome_events);

      if (interaction_metadata.learning_mode == v2::LearningModeType_Apprentice) {
        set_apprentice_reward();
      } else {
        reward = original_reward;
      }
    }

    set_cost(examples, reward);
  }

  void calculate_metrics(dsjson_metrics* metrics) override {
    if (metrics && interaction_data.actions.size() == 0) {
      metrics->NumberOfEventsZeroActions++;
    }
  }
};

struct ccb_joined_event : public typed_joined_event {
  MultiSlotInteraction multi_slot_interaction;
  std::map<std::string, int> slot_id_to_index_map;
  std::vector<float> rewards;
  std::vector<float> original_rewards;
  std::map<int, std::vector<reward::outcome_event>> outcomes_map;

  ~ccb_joined_event() = default;
  bool is_skip_learn() const override { return multi_slot_interaction.skip_learn; }
  void set_skip_learn(bool sl) override { multi_slot_interaction.skip_learn = sl; }

  void set_apprentice_reward() override {
    for (size_t i = 0; i < multi_slot_interaction.interaction_data.size(); i++) {
      if (!multi_slot_interaction.interaction_data[i].actions.empty() &&
        multi_slot_interaction.interaction_data[i].actions[0] == multi_slot_interaction.baseline_actions[i]) {
        rewards[i] = original_rewards[i];
      }
    }
  }

  bool fill_in_label(v_array<example *> &examples) const override {
    // index to interaction_data vector which holds per-slot info
    size_t slot_index = 0;
    auto weight = 1.f / (1.f - multi_slot_interaction.probability_of_drop);

    for (auto *ex : examples) {
      ex->l.conditional_contextual_bandit.weight = weight;

      if (ex->l.conditional_contextual_bandit.type == CCB::example_type::slot) {
        auto &slot_label = ex->l.conditional_contextual_bandit;
        if (multi_slot_interaction.interaction_data.size() > slot_index) {
          const auto &slot_data = multi_slot_interaction.interaction_data[slot_index];
          if ((slot_data.actions.size() != 0) &&
              (slot_data.probabilities.size() != 0)) {
            auto outcome = new CCB::conditional_contextual_bandit_outcome();

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
            outcome->cost = -1.f * rewards[slot_index];
            slot_label.outcome = outcome;
          }
        }
        // process next slot from interaction_data vector
        slot_index++;
      }
    }
    return true;
  }

  void calc_cost(
      float default_reward,
      reward::RewardFunctionType reward_function,
      const metadata::event_metadata_info &metadata_info,
      std::vector<reward::outcome_event> &outcome_events) override {
    size_t num_of_slots = multi_slot_interaction.interaction_data.size();

    if (metadata_info.learning_mode == v2::LearningModeType_Apprentice &&
      num_of_slots != multi_slot_interaction.baseline_actions.size()
    ) {
      VW::io::logger::log_error (
        "slot size [{}] and baseline action size [{}] do not match for event: [{}]",
        num_of_slots,
        multi_slot_interaction.baseline_actions.size(),
        metadata_info.event_id
      );
      return;
    }

    if (slot_id_to_index_map.size() > 0) {
      for (auto &outcome : outcome_events) {
        if (outcome.index_type == v2::IndexValue_literal &&
            !outcome.s_index.empty()) {
          auto iterator = slot_id_to_index_map.find(outcome.s_index);
          if (iterator != slot_id_to_index_map.end()) {
            outcome.index = iterator->second;
          } else {
            VW::io::logger::log_warn("CCB outcome event with slot id: [{}] "
                                     "has no matching interaction slot event.",
                                     outcome.s_index);
          }
        }
      }
    }

    for (auto &o : outcome_events) {
      if (o.index != -1) {
        if (outcomes_map.find(o.index) == outcomes_map.end()) {
          outcomes_map.insert({o.index, {}});
        }

        outcomes_map[o.index].emplace_back(o);
      }
    }

    rewards = std::vector<float>(num_of_slots, default_reward);
    original_rewards = std::vector<float>(num_of_slots, default_reward);

    for (size_t i = 0; i < num_of_slots; i++) {
      if (outcomes_map.find(i) != outcomes_map.end()) {
        original_rewards[i] = reward_function(outcomes_map[i], default_reward);
      }
    }

    if (metadata_info.learning_mode == v2::LearningModeType_Apprentice) {
      set_apprentice_reward();
    } else {
      rewards.assign(original_rewards.begin(), original_rewards.end());
    }
  }

  void calculate_metrics(dsjson_metrics* metrics) override {
    if (metrics) {
      float first_slot_original_reward_neg = 0.f;
      if (!original_rewards.empty()) {
        first_slot_original_reward_neg = -1. * original_rewards[0];
      }
      calculate_multislot_interaction_metrics(metrics, multi_slot_interaction, first_slot_original_reward_neg);
    }
  }

  float get_sum_original_reward() const override {
    float ret = 0.f;
    for(auto reward : original_rewards) {
      ret += reward;
    }
    return ret;
  }
};

struct slates_joined_event : public typed_joined_event {
  MultiSlotInteraction multi_slot_interaction;
  float reward;
  float original_reward;

  ~slates_joined_event() = default;

  bool is_skip_learn() const override { return multi_slot_interaction.skip_learn; }
  void set_skip_learn(bool sl) override { multi_slot_interaction.skip_learn = sl; }

  void set_apprentice_reward() override {}

  bool fill_in_label(v_array<example *> &examples) const override {
    size_t slot_index = 0;
    auto weight = 1.f / (1.f - multi_slot_interaction.probability_of_drop);

    for (auto *ex : examples) {
      ex->l.slates.labeled = true;
      ex->l.slates.weight = weight;

      if (ex->l.slates.type ==  VW::slates::example_type::shared) {
        ex->l.slates.cost= -1.f * reward;
      }

      if (ex->l.slates.type ==  VW::slates::example_type::slot) {
        auto &slot_label = ex->l.slates;
        if (multi_slot_interaction.interaction_data.size() > slot_index) {
          const auto &slot_data = multi_slot_interaction.interaction_data[slot_index];
          if ((slot_data.actions.size() != 0) &&
              (slot_data.probabilities.size() != 0)) {
            if (slot_data.actions.size() != slot_data.probabilities.size()) {
              VW::io::logger::log_warn(
                  "actions and probabilities for event [{}] don't have the "
                  "same size. Actions [{}], probabilities [{}]",
                  slot_data.eventId, slot_data.actions.size(),
                  slot_data.probabilities.size());
              continue;
            }

            for (size_t i = 0; i < slot_data.actions.size(); i++) {
              slot_label.probabilities.push_back(
                {slot_data.actions[i], slot_data.probabilities[i]}
              );
            }
          }
        }
        // process next slot from interaction_data vector
        slot_index++;
      }
    }
    return true;
  }

  void calc_cost(
      float default_reward,
      reward::RewardFunctionType reward_function,
      const metadata::event_metadata_info &metadata_info,
      std::vector<reward::outcome_event> &outcome_events) override {
    reward = default_reward;
    original_reward = reward_function(outcome_events, default_reward);

    if (metadata_info.learning_mode == v2::LearningModeType_Apprentice) {
      VW::io::logger::log_warn( "Apprentice mode is not implmeneted for slates.");
    } else {
      reward = original_reward;
    }
  }

  void calculate_metrics(dsjson_metrics* metrics) override {
    if (metrics) {
      float original_reward_neg = -1. * original_reward;
      calculate_multislot_interaction_metrics(metrics, multi_slot_interaction, original_reward_neg);
    }
  }

  float get_sum_original_reward() const override {
    return original_reward;
  }
}; // slates_joined_event

struct DecisionServiceInteractionCats {
  std::string eventId;
  std::string timestamp;
  float action;
  float pdf_value;
  float probabilityOfDrop = 0.f;
  bool skipLearn{false};
};

struct ca_joined_event : public typed_joined_event {
  DecisionServiceInteractionCats interaction_data;
  float reward = 0.0f;
  float original_reward = 0.0f;

  ~ca_joined_event() = default;

  bool is_skip_learn() const override { return interaction_data.skipLearn; }

  void set_skip_learn(bool sl) override { interaction_data.skipLearn = sl; }

  void set_apprentice_reward() override {
    if (!std::isnan(interaction_data.action)) {
      // TODO: default apprenticeReward should come from config
      // setting to default reward matches current behavior for now
      reward = original_reward;
    }
  }

  bool fill_in_label(v_array<example *> &examples) const override {

    if (std::isnan(interaction_data.action)) {
      VW::io::logger::log_warn("missing action for event [{}]",
                               interaction_data.eventId);
      return false;
    }

    if (std::isnan(interaction_data.pdf_value)) {
      VW::io::logger::log_warn("missing pdf_value for event [{}]",
                               interaction_data.eventId);
      return false;
    }

    if (examples.size() != 1) {
      VW::io::logger::log_warn("example size must be 1, instead got [{}] for event [{}]",
                               examples.size(), interaction_data.eventId);
      return false;
    }

    example *ex = examples[0];
    ex->l.cb_cont.costs.push_back(
        {interaction_data.action, -1.f * reward, interaction_data.pdf_value});
    return true;
  }

  void calc_cost(float default_reward,
                 reward::RewardFunctionType reward_function,
                 const metadata::event_metadata_info &interaction_metadata,
                 std::vector<reward::outcome_event> &outcome_events) override {
    reward = default_reward;
    // original reward is used to record the observed reward of apprentice mode
    original_reward = reward_function(outcome_events, default_reward);

    if (interaction_metadata.learning_mode == v2::LearningModeType_Apprentice) {
      VW::io::logger::log_warn(
          "Apprentice mode is not implmeneted for cats.");
    } else {
      reward = original_reward;
    }
  }

  void calculate_metrics(dsjson_metrics *metrics) override {
    if (metrics && std::isnan(interaction_data.action)) {
      metrics->NumberOfEventsZeroActions++;
    }
  }

  float get_sum_original_reward() const override {
    return original_reward;
  }
};

struct joined_event {
  joined_event(TimePoint &&tp, metadata::event_metadata_info &&mi,
               std::string &&ctx, std::string &&mid,
               std::unique_ptr<typed_joined_event> &&data)
      : joined_event_timestamp(std::move(tp)),
        interaction_metadata(std::move(mi)), context(std::move(ctx)),
        model_id(std::move(mid)), typed_data(std::move(data)),
        outcome_events({}), ok(true) {}
  joined_event() : ok(true) {}

  TimePoint joined_event_timestamp;
  metadata::event_metadata_info interaction_metadata;
  std::string context;
  std::string model_id;
  std::unique_ptr<typed_joined_event> typed_data;
  std::vector<reward::outcome_event> outcome_events;
  bool ok; // ok till proved otherwise

  const typed_joined_event *get_hold_of_typed_data() const {
    return typed_data.get();
  }

  void set_apprentice_reward() const {
    return typed_data->set_apprentice_reward();
  }

  bool fill_in_label(v_array<example *> &examples) const {
    return typed_data->fill_in_label(examples);
  }

  bool is_joined_event_learnable() const {
    bool deferred_action = typed_data->is_skip_learn();

    if (!deferred_action) {
      return true;
    }

    bool outcome_activated = std::any_of(
        outcome_events.begin(), outcome_events.end(),
        [](const reward::outcome_event &o) { return o.action_taken == true; });

    if (outcome_activated) {
      typed_data->set_skip_learn(false);
      return true;
    } else {
      return false;
    }
  }

  void calc_reward(float default_reward, reward::RewardFunctionType reward_function)
  {
    typed_data->calc_cost(default_reward, reward_function, interaction_metadata, outcome_events);
  }

  void calculate_metrics(dsjson_metrics *metrics) {
    return typed_data->calculate_metrics(metrics);
  }

  float get_sum_original_reward() const {
    return typed_data->get_sum_original_reward();
  }
};

struct multistep_joined_event {
  multistep_joined_event(metadata::event_metadata_info &&mi, std::unique_ptr<cb_joined_event> &&data)
      : interaction_metadata(std::move(mi))
      , cb_data(std::move(data))
      , outcome_events({}) {}
  multistep_joined_event()  {}

  metadata::event_metadata_info interaction_metadata;
  std::unique_ptr<cb_joined_event> cb_data;
  std::vector<reward::outcome_event> outcome_events;

  bool fill_in_label(v_array<example *> &examples) const {
    return cb_data->fill_in_label(examples);
  }

  bool is_joined_event_learnable() const {
    bool deferred_action = cb_data->is_skip_learn();

    if (!deferred_action) {
      return true;
    }

    bool outcome_activated = std::any_of(
        outcome_events.begin(), outcome_events.end(),
        [](const reward::outcome_event &o) { return o.action_taken == true; });

    if (outcome_activated) {
      cb_data->set_skip_learn(false);
      return true;
    } else {
      return false;
    }
  }

  void calc_reward(float default_reward, reward::RewardFunctionType reward_function)
  {
    cb_data->calc_cost(default_reward, reward_function, interaction_metadata, outcome_events);
  }
};
} // namespace joined_event
