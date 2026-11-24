#pragma once

#include "reward.h"
// VW headers
// clang-format off
#include "vw/core/ccb_label.h"
#include "vw/core/vw.h"
#include "vw/core/example.h"
#include "vw/io/logger.h"
// clang-format on
#include "vw/json_parser/parse_example_json.h"

namespace v2 = reinforcement_learning::messages::flatbuff::v2;

namespace joined_event
{
struct MultiSlotInteraction
{
  std::vector<VW::parsers::json::decision_service_interaction> interaction_data;
  std::vector<unsigned> baseline_actions;
  bool skip_learn;
  float probability_of_drop{0.f};
};

inline void calculate_multislot_interaction_metrics(VW::details::dsjson_metrics* metrics,
    MultiSlotInteraction multi_slot_interaction, float first_slot_original_reward_neg)
{
  if (metrics != nullptr)
  {
    metrics->dsjson_sum_cost_original_first_slot += first_slot_original_reward_neg;

    if (!multi_slot_interaction.interaction_data.empty() &&
        !multi_slot_interaction.interaction_data[0].actions.empty() && !multi_slot_interaction.baseline_actions.empty())
    {
      if (multi_slot_interaction.interaction_data[0].actions[0] == multi_slot_interaction.baseline_actions[0])
      {
        metrics->dsjson_number_of_label_equal_baseline_first_slot++;
        metrics->dsjson_sum_cost_original_label_equal_baseline_first_slot += first_slot_original_reward_neg;
      }
      else { metrics->dsjson_number_of_label_not_equal_baseline_first_slot++; }
    }
  }
}

struct typed_joined_event
{
  constexpr static float apprentice_matching_reward = 1.0f;
  constexpr static float apprentice_not_matching_reward = 0.0f;
  virtual ~typed_joined_event() = default;
  virtual bool is_skip_learn() const = 0;
  virtual void set_skip_learn(bool sl) = 0;
  virtual void set_apprentice_reward() = 0;
  virtual bool fill_in_label(VW::multi_ex& examples, VW::io::logger& logger) const = 0;
  virtual void calc_cost(float default_reward, reward::RewardFunctionType reward_function,
      const metadata::event_metadata_info& interaction_metadata,
      // TODO outcome_events should also idealy be const here but
      // we currently need it for ccb calculation
      std::vector<reward::outcome_event>& outcome_events, VW::io::logger& logger) = 0;

  virtual void calculate_metrics(VW::details::dsjson_metrics*) {}
  virtual float get_sum_original_reward() const = 0;
};

struct cb_joined_event : public typed_joined_event
{
  VW::parsers::json::decision_service_interaction interaction_data;
  // Default Baseline Action for CB is 1 (rl client recommended actions are 1
  // indexed in the CB case)
  static const unsigned CB_BASELINE_ACTION = 1;
  float reward = 0.0f;
  float original_reward = 0.0f;

  ~cb_joined_event() = default;

  bool is_skip_learn() const override { return interaction_data.skip_learn; }

  void set_skip_learn(bool sl) override { interaction_data.skip_learn = sl; }

  void set_apprentice_reward() override
  {
    if (!interaction_data.actions.empty() && interaction_data.actions[0] == CB_BASELINE_ACTION)
    {
      // The goal of Apprentice mode is to successfully imitate the baseline system.
      // Reward of 1 when we are match baseline and 0 otherwise
      reward = apprentice_matching_reward;
    }
    else { reward = apprentice_not_matching_reward; }
  }

  bool fill_in_label(VW::multi_ex& examples, VW::io::logger& logger) const override
  {
    if (interaction_data.actions.empty())
    {
      logger.out_warn("missing actions for event [{}]", interaction_data.event_id);
      return false;
    }

    if (interaction_data.probabilities.empty())
    {
      logger.out_warn("missing probabilities for event [{}]", interaction_data.event_id);
      return false;
    }

    if (std::any_of(interaction_data.probabilities.begin(), interaction_data.probabilities.end(),
            [](float p) { return std::isnan(p); }))
    {
      logger.out_warn("distribution for event [{}] contains invalid probabilities", interaction_data.event_id);
    }

    int index = interaction_data.actions[0];
    auto action = interaction_data.actions[0];
    auto probability = interaction_data.probabilities[0];
    if (interaction_data.probability_of_drop >= 1.f || interaction_data.probability_of_drop < 0)
    {
      logger.out_warn("Probability of drop should be within [0, 1): [{}]", interaction_data.event_id);
      return false;
    }

    examples[index]->l.cb.costs.push_back({-1.f * reward, action, probability});
    auto weight = 1.f / (1.f - interaction_data.probability_of_drop);

    for (auto* e : examples) { e->l.cb.weight = weight; }

    return true;
  }

  void calc_cost(float default_reward, reward::RewardFunctionType reward_function,
      const metadata::event_metadata_info& interaction_metadata, std::vector<reward::outcome_event>& outcome_events,
      VW::io::logger&) override
  {
    reward = default_reward;
    // original reward is used to record the observed reward of apprentice mode
    original_reward = reward_function(outcome_events, default_reward);

    if (interaction_metadata.learning_mode == v2::LearningModeType_Apprentice) { set_apprentice_reward(); }
    else { reward = original_reward; }
  }

  void calculate_metrics(VW::details::dsjson_metrics* metrics) override
  {
    if (metrics != nullptr)
    {
      if (interaction_data.actions.empty()) { metrics->number_of_events_zero_actions++; }
      else if (interaction_data.actions[0] == CB_BASELINE_ACTION)
      {
        metrics->dsjson_sum_cost_original_baseline += -1.f * original_reward;
      }
    }
  }

  float get_sum_original_reward() const override { return original_reward; }
};

struct ccb_joined_event : public typed_joined_event
{
  MultiSlotInteraction multi_slot_interaction;
  std::map<std::string, int> slot_id_to_index_map;
  std::vector<float> rewards;
  std::vector<float> original_rewards;
  std::map<int, std::vector<reward::outcome_event>> outcomes_map;

  ~ccb_joined_event() override = default;
  bool is_skip_learn() const override { return multi_slot_interaction.skip_learn; }
  void set_skip_learn(bool sl) override { multi_slot_interaction.skip_learn = sl; }

  void set_apprentice_reward() override
  {
    for (size_t i = 0; i < multi_slot_interaction.interaction_data.size(); i++)
    {
      // The goal of Apprentice mode is to successfully imitate the baseline system.
      // Reward of 1 when we are match baseline and 0 otherwise
      if (!multi_slot_interaction.interaction_data[i].actions.empty() &&
          multi_slot_interaction.interaction_data[i].actions[0] == multi_slot_interaction.baseline_actions[i])
      {
        rewards[i] = apprentice_matching_reward;
      }
      else { rewards[i] = apprentice_not_matching_reward; }
    }
  }

  bool fill_in_label(VW::multi_ex& examples, VW::io::logger& logger) const override
  {
    // index to interaction_data vector which holds per-slot info
    size_t slot_index = 0;
    auto weight = 1.f / (1.f - multi_slot_interaction.probability_of_drop);

    for (auto* ex : examples)
    {
      ex->l.conditional_contextual_bandit.weight = weight;

      if (ex->l.conditional_contextual_bandit.type == VW::ccb_example_type::SLOT)
      {
        auto& slot_label = ex->l.conditional_contextual_bandit;
        if (multi_slot_interaction.interaction_data.size() > slot_index)
        {
          const auto& slot_data = multi_slot_interaction.interaction_data[slot_index];
          if (!slot_data.actions.empty() && !slot_data.probabilities.empty())
          {
            auto* outcome = new VW::ccb_outcome();

            if (slot_data.actions.size() != slot_data.probabilities.size())
            {
              logger.out_warn(
                  "actions and probabilities for event [{}] don't have the "
                  "same size. Actions [{}], probabilities [{}]",
                  slot_data.event_id, slot_data.actions.size(), slot_data.probabilities.size());
              continue;
            }

            for (size_t i = 0; i < slot_data.actions.size(); i++)
            {
              outcome->probabilities.push_back({slot_data.actions[i], slot_data.probabilities[i]});
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

  void calc_cost(float default_reward, reward::RewardFunctionType reward_function,
      const metadata::event_metadata_info& metadata_info, std::vector<reward::outcome_event>& outcome_events,
      VW::io::logger& logger) override
  {
    size_t num_of_slots = multi_slot_interaction.interaction_data.size();

    if (metadata_info.learning_mode == v2::LearningModeType_Apprentice &&
        num_of_slots != multi_slot_interaction.baseline_actions.size())
    {
      logger.out_error(
          "slot size [{}] and baseline action size [{}] do not "
          "match for event: [{}]",
          num_of_slots, multi_slot_interaction.baseline_actions.size(), metadata_info.event_id);
      return;
    }

    if (!slot_id_to_index_map.empty())
    {
      for (auto& outcome : outcome_events)
      {
        if (outcome.index_type == v2::IndexValue_literal && !outcome.s_index.empty())
        {
          auto iterator = slot_id_to_index_map.find(outcome.s_index);
          if (iterator != slot_id_to_index_map.end()) { outcome.index = iterator->second; }
          else
          {
            logger.out_warn(
                "CCB outcome event with slot id: [{}] "
                "has no matching interaction slot event.",
                outcome.s_index);
          }
        }
      }
    }

    for (auto& o : outcome_events)
    {
      if (o.index != -1)
      {
        if (outcomes_map.find(o.index) == outcomes_map.end()) { outcomes_map.insert({o.index, {}}); }

        outcomes_map[o.index].emplace_back(o);
      }
    }

    rewards = std::vector<float>(num_of_slots, default_reward);
    original_rewards = std::vector<float>(num_of_slots, default_reward);

    for (int i = 0; i < static_cast<int>(num_of_slots); i++)
    {
      if (outcomes_map.find(i) != outcomes_map.end())
      {
        original_rewards[i] = reward_function(outcomes_map[i], default_reward);
      }
    }

    if (metadata_info.learning_mode == v2::LearningModeType_Apprentice) { set_apprentice_reward(); }
    else { rewards.assign(original_rewards.begin(), original_rewards.end()); }
  }

  void calculate_metrics(VW::details::dsjson_metrics* metrics) override
  {
    if (metrics != nullptr)
    {
      float first_slot_original_reward_neg = 0.f;
      if (!original_rewards.empty()) { first_slot_original_reward_neg = -1.f * original_rewards[0]; }
      calculate_multislot_interaction_metrics(metrics, multi_slot_interaction, first_slot_original_reward_neg);
    }
  }

  float get_sum_original_reward() const override
  {
    float ret = 0.f;
    for (auto reward : original_rewards) { ret += reward; }
    return ret;
  }
};

struct slates_joined_event : public typed_joined_event
{
  MultiSlotInteraction multi_slot_interaction;
  float reward;
  float original_reward;

  ~slates_joined_event() override = default;

  bool is_skip_learn() const override { return multi_slot_interaction.skip_learn; }
  void set_skip_learn(bool sl) override { multi_slot_interaction.skip_learn = sl; }

  void set_apprentice_reward() override {}

  bool fill_in_label(VW::multi_ex& examples, VW::io::logger& logger) const override
  {
    size_t slot_index = 0;
    auto weight = 1.f / (1.f - multi_slot_interaction.probability_of_drop);

    for (auto* ex : examples)
    {
      ex->l.slates.labeled = true;
      ex->l.slates.weight = weight;

      if (ex->l.slates.type == VW::slates::example_type::SHARED) { ex->l.slates.cost = -1.f * reward; }

      if (ex->l.slates.type == VW::slates::example_type::SLOT)
      {
        auto& slot_label = ex->l.slates;
        if (multi_slot_interaction.interaction_data.size() > slot_index)
        {
          const auto& slot_data = multi_slot_interaction.interaction_data[slot_index];
          if ((slot_data.actions.size() != 0) && (slot_data.probabilities.size() != 0))
          {
            if (slot_data.actions.size() != slot_data.probabilities.size())
            {
              logger.out_warn(
                  "actions and probabilities for event [{}] don't have the "
                  "same size. Actions [{}], probabilities [{}]",
                  slot_data.event_id, slot_data.actions.size(), slot_data.probabilities.size());
              continue;
            }

            for (size_t i = 0; i < slot_data.actions.size(); i++)
            {
              slot_label.probabilities.push_back({slot_data.actions[i], slot_data.probabilities[i]});
            }
          }
        }
        // process next slot from interaction_data vector
        slot_index++;
      }
    }
    return true;
  }

  void calc_cost(float default_reward, reward::RewardFunctionType reward_function,
      const metadata::event_metadata_info& metadata_info, std::vector<reward::outcome_event>& outcome_events,
      VW::io::logger& logger) override
  {
    reward = default_reward;
    original_reward = reward_function(outcome_events, default_reward);

    if (metadata_info.learning_mode == v2::LearningModeType_Apprentice)
    {
      logger.out_warn("Apprentice mode is not implmeneted for slates.");
    }
    else { reward = original_reward; }
  }

  void calculate_metrics(VW::details::dsjson_metrics* metrics) override
  {
    if (metrics != nullptr)
    {
      float original_reward_neg = -1.f * original_reward;
      calculate_multislot_interaction_metrics(metrics, multi_slot_interaction, original_reward_neg);
    }
  }

  float get_sum_original_reward() const override { return original_reward; }
};  // slates_joined_event

struct decision_service_interaction_cats
{
  std::string event_id;
  std::string timestamp;
  float action;
  float pdf_value;
  float probability_of_drop = 0.f;
  bool skip_learn{false};
};

struct ca_joined_event : public typed_joined_event
{
  decision_service_interaction_cats interaction_data;
  float reward = 0.0f;
  float original_reward = 0.0f;

  ~ca_joined_event() override = default;

  bool is_skip_learn() const override { return interaction_data.skip_learn; }

  void set_skip_learn(bool sl) override { interaction_data.skip_learn = sl; }

  void set_apprentice_reward() override
  {
    if (!std::isnan(interaction_data.action))
    {
      // TODO: default apprenticeReward should come from config
      // setting to default reward matches current behavior for now
      reward = original_reward;
    }
  }

  bool fill_in_label(VW::multi_ex& examples, VW::io::logger& logger) const override
  {
    if (std::isnan(interaction_data.action))
    {
      logger.out_warn("missing action for event [{}]", interaction_data.event_id);
      return false;
    }

    if (std::isnan(interaction_data.pdf_value))
    {
      logger.out_warn("missing pdf_value for event [{}]", interaction_data.event_id);
      return false;
    }

    if (examples.size() != 1)
    {
      logger.out_warn(
          "example size must be 1, instead got [{}] for event [{}]", examples.size(), interaction_data.event_id);
      return false;
    }

    VW::example* ex = examples[0];
    ex->l.cb_cont.costs.push_back({interaction_data.action, -1.f * reward, interaction_data.pdf_value});
    return true;
  }

  void calc_cost(float default_reward, reward::RewardFunctionType reward_function,
      const metadata::event_metadata_info& interaction_metadata, std::vector<reward::outcome_event>& outcome_events,
      VW::io::logger& logger) override
  {
    reward = default_reward;
    // original reward is used to record the observed reward of apprentice mode
    original_reward = reward_function(outcome_events, default_reward);

    if (interaction_metadata.learning_mode == v2::LearningModeType_Apprentice)
    {
      logger.out_warn("Apprentice mode is not implmeneted for cats.");
    }
    else { reward = original_reward; }
  }

  void calculate_metrics(VW::details::dsjson_metrics* metrics) override
  {
    if ((metrics != nullptr) && std::isnan(interaction_data.action)) { metrics->number_of_events_zero_actions++; }
  }

  float get_sum_original_reward() const override { return original_reward; }
};

struct joined_event
{
  joined_event(TimePoint&& tp, metadata::event_metadata_info&& mi, std::string&& ctx, std::string&& mid,
      std::unique_ptr<typed_joined_event>&& data)
      : joined_event_timestamp(std::move(tp))
      , interaction_metadata(std::move(mi))
      , context(std::move(ctx))
      , model_id(std::move(mid))
      , typed_data(std::move(data))
      , outcome_events({})
      , ok(true)
  {
  }
  joined_event() : ok(true) {}

  TimePoint joined_event_timestamp;
  metadata::event_metadata_info interaction_metadata;
  std::string context;
  std::string model_id;
  std::unique_ptr<typed_joined_event> typed_data;
  std::vector<reward::outcome_event> outcome_events;
  bool ok;  // ok till proved otherwise

  const typed_joined_event* get_hold_of_typed_data() const { return typed_data.get(); }

  void set_apprentice_reward() const { return typed_data->set_apprentice_reward(); }

  bool fill_in_label(VW::multi_ex& examples, VW::io::logger& logger) const
  {
    return typed_data->fill_in_label(examples, logger);
  }

  bool is_joined_event_learnable() const
  {
    bool deferred_action = typed_data->is_skip_learn();

    if (!deferred_action) { return true; }

    bool outcome_activated = std::any_of(outcome_events.begin(), outcome_events.end(),
        [](const reward::outcome_event& o) { return o.action_taken == true; });

    if (outcome_activated)
    {
      typed_data->set_skip_learn(false);
      return true;
    }
    else { return false; }
  }

  void calc_reward(float default_reward, reward::RewardFunctionType reward_function, VW::io::logger& logger)
  {
    typed_data->calc_cost(default_reward, reward_function, interaction_metadata, outcome_events, logger);
  }

  void calculate_metrics(VW::details::dsjson_metrics* metrics) { return typed_data->calculate_metrics(metrics); }

  float get_sum_original_reward() const { return typed_data->get_sum_original_reward(); }
};
}  // namespace joined_event
