#include "example_joiner.h"

#include "generated/v2/DedupInfo_generated.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/Metadata_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "zstd.h"
#include "io/logger.h"

#include <limits.h>
#include <time.h>

// VW headers
#include "example.h"
#include "io/logger.h"
#include "parse_example_json.h"
#include "parser.h"
#include "v_array.h"

namespace RewardFunctions {
float average(const joined_event &event) {
  float sum = 0.f;
  for (const auto &o : event.outcome_events) {
    sum += o.value;
  }

  return sum / event.outcome_events.size();
}

float sum(const joined_event &event) {
  float sum = 0.f;
  for (const auto &o : event.outcome_events) {
    sum += o.value;
  }

  return sum;
}

float min(const joined_event &event) {
  float min_reward = std::numeric_limits<float>::max();
  for (const auto &o : event.outcome_events) {
    if (o.value < min_reward) {
      min_reward = o.value;
    }
  }
  return min_reward;
}

float max(const joined_event &event) {
  float max_reward = std::numeric_limits<float>::min();
  for (const auto &o : event.outcome_events) {
    if (o.value > max_reward) {
      max_reward = o.value;
    }
  }
  return max_reward;
}

float median(const joined_event &event) {
  std::vector<float> values;
  for (const auto &o : event.outcome_events) {
    values.push_back(o.value);
  }

  int outcome_events_size = values.size();

  sort(values.begin(), values.end());
  if (outcome_events_size % 2 == 0) {
    return (values[outcome_events_size / 2 - 1] +
            values[outcome_events_size / 2]) /
           2;
  } else {
    return values[outcome_events_size / 2];
  }
}

float earliest(const joined_event &event) {
  time_t oldest_valid_observation = std::numeric_limits<time_t>::max();
  float earliest_reward = 0.f;

  for (const auto &o : event.outcome_events) {
    if (o.enqueued_time_utc < oldest_valid_observation) {
      oldest_valid_observation = o.enqueued_time_utc;
      earliest_reward = o.value;
    }
  }

  return earliest_reward;
}
} // namespace RewardFunctions

example_joiner::example_joiner(vw *vw)
    : _vw(vw), _reward_calculation(&RewardFunctions::earliest) {}

example_joiner::~example_joiner() {
  // cleanup examples
  _dedup_cache.clear(return_example_f, this);
  for (auto *ex : _example_pool) {
    VW::dealloc_examples(ex, 1);
  }
}

example *example_joiner::get_or_create_example() {
  // alloc new element if we don't have any left
  if (_example_pool.size() == 0) {
    auto ex = VW::alloc_examples(1);
    _vw->example_parser->lbl_parser.default_label(&ex->l);

    return ex;
  }

  // get last element
  example *ex = _example_pool.back();
  _example_pool.pop_back();

  _vw->example_parser->lbl_parser.default_label(&ex->l);
  VW::empty_example(*_vw, *ex);

  return ex;
}

void example_joiner::return_example(example *ex) {
  _example_pool.push_back(ex);
}

example &example_joiner::get_or_create_example_f(void *vw) {
  return *(((example_joiner *)vw)->get_or_create_example());
}

void example_joiner::return_example_f(void *vw, example *ex) {
  ((example_joiner *)vw)->return_example(ex);
}

int example_joiner::process_event(const v2::JoinedEvent &joined_event) {
  auto event = flatbuffers::GetRoot<v2::Event>(joined_event.event()->data());
  std::string id = event->meta()->id()->str();
  if (event->meta()->payload_type() == v2::PayloadType_DedupInfo) {
    process_dedup(*event, *event->meta());
    return 0;
  }
  if (_batch_grouped_events.find(id) != _batch_grouped_events.end()) {
    _batch_grouped_events[id].push_back(&joined_event);
  } else {
    _batch_grouped_events.insert({id, {&joined_event}});
    _batch_event_order.emplace(id);
  }
  return 0;
}

void example_joiner::set_default_reward(float default_reward) {
  _default_reward = default_reward;
}

void example_joiner::set_learning_mode_config(const v2::LearningModeType& learning_mode) {
  _learning_mode_config = learning_mode;
}

void example_joiner::set_problem_type_config(const v2::ProblemType& problem_type) {
  _problem_type_config = problem_type;
}

void example_joiner::set_reward_function(const v2::RewardFunctionType type) {
  using namespace RewardFunctions;

  switch (type) {
  case v2::RewardFunctionType_Earliest:
    _reward_calculation = &earliest;
    break;
  case v2::RewardFunctionType_Average:
    _reward_calculation = &average;
    break;

  case v2::RewardFunctionType_Sum:
    _reward_calculation = &sum;
    break;

  case v2::RewardFunctionType_Min:
    _reward_calculation = &min;
    break;

  case v2::RewardFunctionType_Max:
    _reward_calculation = &max;
    break;

  case v2::RewardFunctionType_Median:
    _reward_calculation = &median;
    break;

  default:
    break;
  }
}

template <typename T>
const T *example_joiner::process_compression(const uint8_t *data, size_t size,
                                             const v2::Metadata &metadata) {

  const T *payload = nullptr;

  if (metadata.encoding() == v2::EventEncoding_Zstd) {
    size_t buff_size = ZSTD_getFrameContentSize(data, size);
    if (buff_size == ZSTD_CONTENTSIZE_ERROR) {
      // TODO figure out error handling behaviour for parser
      throw("Invalid compressed content.");
    }
    if (buff_size == ZSTD_CONTENTSIZE_UNKNOWN) {
      // TODO figure out error handling behaviour for parser
      throw("Unknown compressed size.");
    }

    std::unique_ptr<uint8_t[]> buff_data(
        flatbuffers::DefaultAllocator().allocate(buff_size));
    size_t res = ZSTD_decompress(buff_data.get(), buff_size, data, size);

    if (ZSTD_isError(res)) {
      // TODO figure out error handling behaviour for parser
      throw(ZSTD_getErrorName(res));
    }

    auto data_ptr = buff_data.release();

    _detached_buffer =
        flatbuffers::DetachedBuffer(nullptr, false, data_ptr, 0, data_ptr, res);
    payload = flatbuffers::GetRoot<T>(_detached_buffer.data());

  } else {
    payload = flatbuffers::GetRoot<T>(data);
  }
  return payload;
}

void example_joiner::try_set_label(const joined_event &je, float reward,
                                   v_array<example *> &examples) {
  if (je.interaction_data.actions.empty()) {
    VW::io::logger::log_error("missing actions for event [{}]",
                              je.interaction_data.eventId);
    return;
  }

  if (je.interaction_data.probabilities.empty()) {
    VW::io::logger::log_error("missing probabilities for event [{}]",
                              je.interaction_data.eventId);
    return;
  }

  if (std::any_of(je.interaction_data.probabilities.begin(),
                  je.interaction_data.probabilities.end(),
                  [](float p) { return std::isnan(p); })) {
    VW::io::logger::log_error(
        "distribution for event [{}] contains invalid probabilities",
        je.interaction_data.eventId);
  }

  int index = je.interaction_data.actions[0];
  auto action = je.interaction_data.actions[0];
  auto cost = -1.f * _reward;
  auto probability = je.interaction_data.probabilities[0] *
                     (1.f - je.interaction_data.probabilityOfDrop);
  auto weight = 1.f - je.interaction_data.probabilityOfDrop;

  examples[index]->l.cb.costs.push_back({cost, action, probability});
  examples[index]->l.cb.weight = weight;
}

int example_joiner::process_interaction(const v2::Event &event,
                                        const v2::Metadata &metadata,
                                        v_array<example *> &examples) {

  if (metadata.payload_type() == v2::PayloadType_CB) {

    auto cb = process_compression<v2::CbEvent>(
        event.payload()->data(), event.payload()->size(), metadata);

    v2::LearningModeType learning_mode = cb->learning_mode();

    if (learning_mode != _learning_mode_config) {
      VW::io::logger::log_critical(
        "Online Trainer learning mode [{}] "
        "and Interaction event learning mode [{}]"
        "don't match. Skipping interaction from processing."
        "EventId: [{}]",
        EnumNameLearningModeType(_learning_mode_config),
        EnumNameLearningModeType(learning_mode),
        metadata.id()->c_str()
      );

      return 0;
    }

    metadata_info meta = {"client_time_utc",
                          metadata.app_id() ? metadata.app_id()->str() : "",
                          metadata.payload_type(),
                          metadata.pass_probability(),
                          metadata.encoding(),
                          learning_mode};

    DecisionServiceInteraction data;
    data.eventId = metadata.id()->str();
    data.actions = {cb->action_ids()->data(),
                    cb->action_ids()->data() + cb->action_ids()->size()};
    data.probabilities = {cb->probabilities()->data(),
                          cb->probabilities()->data() +
                              cb->probabilities()->size()};
    data.probabilityOfDrop = 1.f - metadata.pass_probability();
    data.skipLearn = cb->deferred_action();

    std::string line_vec(reinterpret_cast<char const *>(cb->context()->data()),
                         cb->context()->size());

    if (_vw->audit || _vw->hash_inv) {
      VW::template read_line_json<true>(
          *_vw, examples, const_cast<char *>(line_vec.c_str()),
          reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), _vw,
          &_dedup_cache.dedup_examples);
    } else {
      VW::template read_line_json<false>(
          *_vw, examples, const_cast<char *>(line_vec.c_str()),
          reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), _vw,
          &_dedup_cache.dedup_examples);
    }

    _batch_grouped_examples.emplace(std::make_pair<std::string, joined_event>(
        metadata.id()->str(),
        {"joiner_timestamp", std::move(meta), std::move(data)}));
  }
  return 0;
}

int example_joiner::process_outcome(const v2::Event &event,
                                    const v2::Metadata &metadata,
                                    const time_t &enqueued_time_utc) {
  outcome_event o_event;
  o_event.metadata = {"client_time_utc",
                      metadata.app_id() ? metadata.app_id()->str() : "",
                      metadata.payload_type(), metadata.pass_probability(),
                      metadata.encoding()};
  o_event.enqueued_time_utc = enqueued_time_utc;

  auto outcome = process_compression<v2::OutcomeEvent>(
      event.payload()->data(), event.payload()->size(), metadata);

  int index = -1;

  if (outcome->value_type() == v2::OutcomeValue_literal) {
    o_event.s_value = outcome->value_as_literal()->c_str();
  } else if (outcome->value_type() == v2::OutcomeValue_numeric) {
    o_event.value = outcome->value_as_numeric()->value();
  }

  if (outcome->index_type() == v2::IndexValue_literal) {
    o_event.s_index = outcome->index_as_literal()->c_str();
    index = std::stoi(outcome->index_as_literal()->c_str());
  } else if (outcome->index_type() == v2::IndexValue_numeric) {
    o_event.s_index = outcome->index_as_numeric()->index();
    index = outcome->index_as_numeric()->index();
  }

  if (_batch_grouped_examples.find(metadata.id()->str()) !=
      _batch_grouped_examples.end()) {
    auto &joined_event = _batch_grouped_examples[metadata.id()->str()];
    joined_event.outcome_events.push_back(o_event);
  }

  return 0;
}

int example_joiner::process_dedup(const v2::Event &event,
                                  const v2::Metadata &metadata) {

  auto dedup = process_compression<v2::DedupInfo>(
      event.payload()->data(), event.payload()->size(), metadata);

  auto examples = v_init<example *>();
  // TODO check optional fields and act accordingly if missing
  for (size_t i = 0; i < dedup->ids()->size(); i++) {
    auto dedup_id = dedup->ids()->Get(i);
    if (!_dedup_cache.exists(dedup_id)) {

      examples.push_back(get_or_create_example());

      if (_vw->audit || _vw->hash_inv) {
        VW::template read_line_json<true>(
            *_vw, examples,
            const_cast<char *>(dedup->values()->Get(i)->c_str()),
            get_or_create_example_f, this);
      } else {
        VW::template read_line_json<false>(
            *_vw, examples,
            const_cast<char *>(dedup->values()->Get(i)->c_str()),
            get_or_create_example_f, this);
      }

      _dedup_cache.add(dedup_id, examples[0]);
      examples.clear();
    } else {
      _dedup_cache.update(dedup_id);
    }
  }

  if (dedup->ids()->size() > 0) {
    // location of first item in dedup payload will be the "last" item in the
    // cache that we care about keeping
    _dedup_cache.clear_after(dedup->ids()->Get(0), return_example_f, this);
  }

  return 0;
}

int example_joiner::process_joined(v_array<example *> &examples) {
  if (_batch_event_order.empty()) {
    return 0;
  }

  auto &id = _batch_event_order.front();

  bool multiline = true;
  for (auto &joined_event : _batch_grouped_events[id]) {
    auto event = flatbuffers::GetRoot<v2::Event>(joined_event->event()->data());
    auto metadata = event->meta();

    if (metadata->payload_type() == v2::PayloadType_Outcome) {
      time_t raw_time;
      time(&raw_time);
      struct tm *enqueued_time;
      // TODO use gmtime_s? windows tests break when accessing enqueue_time internals
      enqueued_time = gmtime(&raw_time);
      joined_event->timestamp()->year() - 1900;
      joined_event->timestamp()->month() - 1;
      joined_event->timestamp()->day();
      joined_event->timestamp()->hour();
      joined_event->timestamp()->minute();
      joined_event->timestamp()->second();

      time_t enqueued_time_utc;// = mktime(enqueued_time);
      process_outcome(*event, *metadata, enqueued_time_utc);
    } else {
      v2::PayloadType payload_type = metadata->payload_type();

      if (EnumNamePayloadType(payload_type) != EnumNameProblemType(_problem_type_config)) {
        VW::io::logger::log_critical(
          "Online Trainer mode [{}] "
          "and Interaction event type [{}] "
          "don't match. Skipping interaction from processing."
          "EventId: [{}]",
          EnumNameProblemType(_problem_type_config),
          EnumNamePayloadType(payload_type),
          metadata->id()->c_str());
        continue;
      }

      if (payload_type == v2::PayloadType_CA) {
        multiline = false;
      }
      process_interaction(*event, *metadata, examples);
    }
  }

  auto &je = _batch_grouped_examples[id];

  if (je.outcome_events.size() > 0) {
    if (je.interaction_metadata.payload_type == v2::PayloadType_CB &&
        je.interaction_metadata.learning_mode ==
            v2::LearningModeType_Apprentice) {
      if (je.interaction_data.actions[0] == je.baseline_action) {
        // TODO: default apprenticeReward should come from config
        // setting to default reward matches current behavior for now
        _reward = _reward_calculation(je);
      }
    } else {
      _reward = _reward_calculation(je);
    }
  }

  try_set_label(je, _reward, examples);

  if (multiline) {
    // add an empty example to signal end-of-multiline
    examples.push_back(&VW::get_unused_example(_vw));
    _vw->example_parser->lbl_parser.default_label(&examples.back()->l);
  }

  _batch_grouped_events.erase(id);
  _batch_event_order.pop();
  _batch_grouped_examples.erase(id);

  return 0;
}

bool example_joiner::processing_batch() { return !_batch_event_order.empty(); }
float example_joiner::get_reward() { return _reward; }