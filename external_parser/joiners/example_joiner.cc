// Windows may define a preprocessor macro named GetObject, which conflicts with RapidJSON
// Undefine it if it's defined, then include the RapidJSON header
// This step should come first before all other includes
// clang-format off
#ifdef GetObject
#  define RL_WINDOWS_GETOBJECT_MACRO_UNDEF
#  pragma push_macro("GetObject")
#  undef GetObject
#endif
#include <rapidjson/document.h>
// clang-format on

#include "joiners/example_joiner.h"

#include "event_processors/typed_events.h"
#include "generated/v2/DedupInfo_generated.h"
#include "generated/v2/Event_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "log_converter.h"
#include "zstd.h"

#include <limits.h>
#include <time.h>

#include <algorithm>
#include <cctype>

// VW headers
#include "vw/core/parse_example_json.h"
#include "vw/core/parser.h"
#include "vw/core/scope_exit.h"

example_joiner::example_joiner(VW::workspace* vw)
    : i_joiner(vw->logger), _vw(vw), _reward_calculation(&reward::earliest), _binary_to_json(false)
{
}

example_joiner::example_joiner(VW::workspace* vw, bool binary_to_json, const std::string& outfile_name)
    : i_joiner(vw->logger), _vw(vw), _reward_calculation(&reward::earliest), _binary_to_json(binary_to_json)
{
  _outfile.open(outfile_name, std::ofstream::out);
}

example_joiner::~example_joiner()
{
  // cleanup examples
  _dedup_cache.clear(return_example_f, this);
  for (auto* ex : _example_pool) { VW::dealloc_examples(ex, 1); }
  if (_binary_to_json) { _outfile.close(); }
}

VW::example* example_joiner::get_or_create_example()
{
  // alloc new element if we don't have any left
  if (_example_pool.empty())
  {
    auto* ex = VW::alloc_examples(1);
    _vw->example_parser->lbl_parser.default_label(ex->l);

    return ex;
  }

  // get last element
  VW::example* ex = _example_pool.back();
  _example_pool.pop_back();

  _vw->example_parser->lbl_parser.default_label(ex->l);
  VW::empty_example(*_vw, *ex);

  return ex;
}

void example_joiner::return_example(VW::example* ex) { _example_pool.push_back(ex); }

VW::example& example_joiner::get_or_create_example_f(void* vw)
{
  return *(((example_joiner*)vw)->get_or_create_example());
}

void example_joiner::return_example_f(void* vw, VW::example* ex) { ((example_joiner*)vw)->return_example(ex); }

bool example_joiner::process_event(const v2::JoinedEvent& joined_event)
{
  if (joined_event.event() == nullptr || joined_event.timestamp() == nullptr)
  {
    logger.out_error("JoinedEvent is malformed, can not process JoinedEvent");
    return false;
  }

  auto event = flatbuffers::GetRoot<v2::Event>(joined_event.event()->data());

  if (event->meta() == nullptr || event->meta()->id() == nullptr)
  {
    logger.out_error("Event of JoinedEvent is malformed, can not process JoinedEvent");
    return false;
  }

  std::string id = event->meta()->id()->str();

  if (event->meta()->payload_type() == v2::PayloadType_DedupInfo)
  {
    if (!process_dedup(*event, *event->meta()))
    {
      // clean everything this batch is ruined without the dedup info
      // dedup cache will clear itself up when next dedup payload arrives
      clear_batch_info();
      return false;
    }
    return true;
  }
  else if (event->meta()->payload_type() == v2::PayloadType_Episode)
  {
    logger.out_error("Episode type events require multistep");
    return false;
  }
  if (_batch_grouped_events.find(id) != _batch_grouped_events.end())
  {
    _batch_grouped_events[id].push_back(&joined_event);
  }
  else
  {
    _batch_grouped_events.insert({id, {&joined_event}});
    _batch_event_order.emplace(id);
  }
  return true;
}

void example_joiner::set_default_reward(float default_reward, bool sticky)
{
  _loop_info.default_reward.set(default_reward, sticky);
}

void example_joiner::set_learning_mode_config(v2::LearningModeType learning_mode, bool sticky)
{
  _loop_info.learning_mode_config.set(learning_mode, sticky);
}

void example_joiner::set_problem_type_config(v2::ProblemType problem_type, bool sticky)
{
  _loop_info.problem_type_config.set(problem_type, sticky);
}

void example_joiner::set_use_client_time(bool use_client_time, bool sticky)
{
  _loop_info.use_client_time.set(use_client_time, sticky);
}

bool example_joiner::joiner_ready() { return _loop_info.is_configured() && _reward_calculation.is_valid(); }

void example_joiner::set_reward_function(const v2::RewardFunctionType type, bool sticky)
{
  reward::RewardFunctionType reward_calculation = nullptr;
  switch (type)
  {
    case v2::RewardFunctionType_Earliest:
      reward_calculation = &reward::earliest;
      break;
    case v2::RewardFunctionType_Average:
      reward_calculation = &reward::average;
      break;

    case v2::RewardFunctionType_Sum:
      reward_calculation = &reward::sum;
      break;

    case v2::RewardFunctionType_Min:
      reward_calculation = &reward::min;
      break;

    case v2::RewardFunctionType_Max:
      reward_calculation = &reward::max;
      break;

    case v2::RewardFunctionType_Median:
      reward_calculation = &reward::median;
      break;

    default:
      break;
  }

  if (reward_calculation != nullptr) { _reward_calculation.set(reward_calculation, sticky); }
}

void example_joiner::clear_batch_info()
{
  _batch_grouped_events.clear();
  _batch_grouped_examples.clear();
  while (!_batch_event_order.empty()) { _batch_event_order.pop(); }
}

void example_joiner::clear_vw_examples(VW::multi_ex& examples)
{
  // cleanup examples since something might have gone wrong and left the
  // existing example's in a bad state
  VW::return_multiple_example(*_vw, examples);
  // add one new example since all parsers expect to be called with one unused
  // example
  examples.push_back(VW::new_unused_example(*_vw));
}

void example_joiner::clear_event_id_batch_info(const std::string& id)
{
  _batch_grouped_events.erase(id);
  if (!_batch_event_order.empty() && _batch_event_order.front() == id) { _batch_event_order.pop(); }
  _batch_grouped_examples.erase(id);
}

void example_joiner::invalidate_joined_event(const std::string& id)
{
  if (_batch_grouped_examples.find(id) != _batch_grouped_examples.end()) { _batch_grouped_examples[id].ok = false; }
}

// Case insensitive equality
bool iequals(const std::string& a, const std::string& b)
{
  if (a.size() != b.size()) { return false; }

  return std::equal(a.begin(), a.end(), b.begin(), [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

bool example_joiner::process_interaction(
    const v2::Event& event, const v2::Metadata& metadata, const TimePoint& enqueued_time_utc, VW::multi_ex& examples)
{
  std::string payload_type(EnumNamePayloadType(metadata.payload_type()));
  std::string loop_type(EnumNameProblemType(_loop_info.problem_type_config));

  if (!iequals(payload_type, loop_type))
  {
    logger.out_warn(
        "Online Trainer mode [{}] "
        "and Interaction event type [{}] "
        "don't match. Skipping interaction from processing. "
        "EventId: [{}]",
        EnumNameProblemType(_loop_info.problem_type_config), EnumNamePayloadType(metadata.payload_type()),
        metadata.id()->c_str());
    return false;
  }

  joined_event::joined_event je;

  if (metadata.payload_type() == v2::PayloadType_CB)
  {
    const v2::CbEvent* cb = nullptr;
    if (!typed_event::process_compression<v2::CbEvent>(
            event.payload()->data(), event.payload()->size(), metadata, cb, _detached_buffer, logger) ||
        cb == nullptr)
    {
      return false;
    }

    if (!typed_event::event_processor<v2::CbEvent>::is_valid(*cb, _loop_info, logger))
    {
      logger.out_warn(
          "CB payload with event id [{}] is malformed. "
          "Skipping interaction from processing.",
          metadata.id()->c_str());
      return false;
    }

    je = typed_event::event_processor<v2::CbEvent>::fill_in_joined_event(
        *cb, metadata, enqueued_time_utc, typed_event::event_processor<v2::CbEvent>::get_context(*cb));
  }
  else if (metadata.payload_type() == v2::PayloadType_CCB || metadata.payload_type() == v2::PayloadType_Slates)
  {
    const v2::MultiSlotEvent* multislot = nullptr;
    if (!typed_event::process_compression<v2::MultiSlotEvent>(
            event.payload()->data(), event.payload()->size(), metadata, multislot, _detached_buffer, logger) ||
        multislot == nullptr)
    {
      return false;
    }

    if (!typed_event::event_processor<v2::MultiSlotEvent>::is_valid(*multislot, _loop_info, logger))
    {
      logger.out_warn(
          "[{}] payload with event id [{}] is malformed. "
          "Skipping interaction from processing.",
          EnumNamePayloadType(metadata.payload_type()), metadata.id()->c_str());
      return false;
    }

    je = typed_event::event_processor<v2::MultiSlotEvent>::fill_in_joined_event(*multislot, metadata, enqueued_time_utc,
        typed_event::event_processor<v2::MultiSlotEvent>::get_context(*multislot));
  }
  else if (metadata.payload_type() == v2::PayloadType_CA)
  {
    const v2::CaEvent* ca = nullptr;
    if (!typed_event::process_compression<v2::CaEvent>(
            event.payload()->data(), event.payload()->size(), metadata, ca, _detached_buffer, logger) ||
        ca == nullptr)
    {
      return false;
    }

    if (!typed_event::event_processor<v2::CaEvent>::is_valid(*ca, _loop_info, logger))
    {
      logger.out_warn(
          "CA payload with event id [{}] is malformed. "
          "Skipping interaction from processing.",
          metadata.id()->c_str());
      return false;
    }

    je = typed_event::event_processor<v2::CaEvent>::fill_in_joined_event(
        *ca, metadata, enqueued_time_utc, typed_event::event_processor<v2::CaEvent>::get_context(*ca));
  }
  else
  {
    // for now only CB is supported so log and return false
    logger.out_error(
        "Interaction event learning mode [{}] not "
        "currently supported, skipping interaction "
        "EventId: [{}]",
        metadata.payload_type(), metadata.id()->c_str());
    return false;
  }

  if (!_binary_to_json)
  {
    std::string context(je.context);
    try
    {
      VW::example_factory_t ex_fac = [this]() -> VW::example& { return *(VW::new_unused_example(*this->_vw)); };
      if (_vw->audit || _vw->hash_inv)
      {
        VW::parsers::json::read_line_json<true>(
            *_vw, examples, const_cast<char*>(context.c_str()), context.size(), ex_fac, &_dedup_cache.dedup_examples);
      }
      else
      {
        VW::parsers::json::read_line_json<false>(
            *_vw, examples, const_cast<char*>(context.c_str()), context.size(), ex_fac, &_dedup_cache.dedup_examples);
      }
    }
    catch (VW::vw_exception& e)
    {
      logger.out_warn(
          "JSON parsing during interaction processing failed "
          "with error: [{}] for event with id: [{}]",
          e.what(), metadata.id()->c_str());
      return false;
    }
  }

  _batch_grouped_examples.emplace(
      std::make_pair<std::string, joined_event::joined_event>(metadata.id()->str(), std::move(je)));
  return true;
}

bool example_joiner::process_outcome(
    const v2::Event& event, const v2::Metadata& metadata, const TimePoint& enqueued_time_utc)
{
  reward::outcome_event o_event;
  o_event.metadata = {metadata.app_id() != nullptr ? metadata.app_id()->str() : "", metadata.payload_type(),
      metadata.pass_probability(), metadata.encoding(), metadata.id()->str(), v2::LearningModeType_Online};

  o_event.enqueued_time_utc = enqueued_time_utc;

  const v2::OutcomeEvent* outcome = nullptr;
  if (!typed_event::process_compression<v2::OutcomeEvent>(
          event.payload()->data(), event.payload()->size(), metadata, outcome, _detached_buffer, logger) ||
      outcome == nullptr)
  {
    // invalidate joined_event so that we don't learn from it
    invalidate_joined_event(metadata.id()->str());
    return false;
  }

  if (outcome->value_type() == v2::OutcomeValue_literal) { o_event.s_value = outcome->value_as_literal()->c_str(); }
  else if (outcome->value_type() == v2::OutcomeValue_numeric) { o_event.value = outcome->value_as_numeric()->value(); }

  o_event.index_type = outcome->index_type();

  if (outcome->index_type() == v2::IndexValue_literal) { o_event.s_index = outcome->index_as_literal()->c_str(); }
  else if (outcome->index_type() == v2::IndexValue_numeric) { o_event.index = outcome->index_as_numeric()->index(); }

  o_event.action_taken = outcome->action_taken();

  if (_batch_grouped_examples.find(metadata.id()->str()) != _batch_grouped_examples.end())
  {
    auto& joined_event = _batch_grouped_examples[metadata.id()->str()];
    joined_event.outcome_events.push_back(o_event);
  }

  return true;
}

bool example_joiner::process_dedup(const v2::Event& event, const v2::Metadata& metadata)
{
  const v2::DedupInfo* dedup = nullptr;
  if (!typed_event::process_compression<v2::DedupInfo>(
          event.payload()->data(), event.payload()->size(), metadata, dedup, _detached_buffer, logger) ||
      dedup == nullptr)
  {
    return false;
  }

  if (dedup->ids()->size() != dedup->values()->size())
  {
    logger.out_error("Can not process dedup payload, id and value sizes do not match");
    return false;
  }

  VW::multi_ex examples;

  for (flatbuffers::uoffset_t i = 0; i < dedup->ids()->size(); i++)
  {
    auto dedup_id = dedup->ids()->Get(i);
    if (!_dedup_cache.exists(dedup_id))
    {
      examples.push_back(get_or_create_example());
      VW::example_factory_t ex_fac = [this]() -> VW::example& { return get_or_create_example_f(this); };
      try
      {
        if (_vw->audit || _vw->hash_inv)
        {
          VW::parsers::json::template read_line_json<true>(*_vw, examples,
              const_cast<char*>(dedup->values()->Get(i)->c_str()), dedup->values()->Get(i)->size(), ex_fac);
        }
        else
        {
          VW::parsers::json::template read_line_json<false>(*_vw, examples,
              const_cast<char*>(dedup->values()->Get(i)->c_str()), dedup->values()->Get(i)->size(), ex_fac);
        }
      }
      catch (VW::vw_exception& e)
      {
        logger.out_error(
            "JSON parsing during dedup processing failed "
            "with error: [{}]",
            e.what());
        return false;
      }

      _dedup_cache.add(dedup_id, examples[0]);
      examples.clear();
    }
    else { _dedup_cache.update(dedup_id); }
  }

  if (dedup->ids()->size() > 0)
  {
    // location of first item in dedup payload will be the "last" item in the
    // cache that we care about keeping
    _dedup_cache.clear_after(dedup->ids()->Get(0), return_example_f, this);
  }

  return true;
}

bool example_joiner::process_joined(VW::multi_ex& examples)
{
  _current_je_is_skip_learn = false;

  if (_batch_event_order.empty()) { return true; }

  auto id = _batch_event_order.front();
  bool multiline = false;

  for (auto& joined_event : _batch_grouped_events[id])
  {
    const auto* event = flatbuffers::GetRoot<v2::Event>(joined_event->event()->data());
    const auto* metadata = event->meta();
    auto enqueued_time_utc =
        get_enqueued_time(joined_event->timestamp(), metadata->client_time_utc(), _loop_info.use_client_time, logger);
    const auto& payload_type = metadata->payload_type();

    if (payload_type == v2::PayloadType_Outcome) { process_outcome(*event, *metadata, enqueued_time_utc); }
    else
    {
      multiline = (payload_type != v2::PayloadType_CA);
      if (!process_interaction(*event, *metadata, enqueued_time_utc, examples)) { continue; }
    }
  }

  joined_event::joined_event* je = nullptr;

  // this scope exit guard will execute when this method returns
  // that way we can guarantee clean-up no matter where the return happens
  // without having to duplicate the cleanup code
  bool clear_examples = false;
  auto clear_event_id_on_exit = VW::scope_exit(
      [&]
      {
        if (je != nullptr)
        {
          if (_vw->example_parser->metrics)
          {
            if (!je->is_joined_event_learnable()) { _joiner_metrics.number_of_skipped_events++; }
            else
            {
              je->calculate_metrics(_vw->example_parser->metrics.get());
              _joiner_metrics.sum_cost_original += -1.f * je->get_sum_original_reward();
              if (_joiner_metrics.first_event_id.empty())
              {
                _joiner_metrics.first_event_id = std::move(je->interaction_metadata.event_id);
                _joiner_metrics.first_event_timestamp = std::move(je->joined_event_timestamp);
              }
              else
              {
                _joiner_metrics.last_event_id = std::move(je->interaction_metadata.event_id);
                _joiner_metrics.last_event_timestamp = std::move(je->joined_event_timestamp);
              }
            }
          }

          if (_binary_to_json) { log_converter::build_json(_outfile, *je, logger); }
        }

        clear_event_id_batch_info(id);
        if (clear_examples) { clear_vw_examples(examples); }
      });

  if (_batch_grouped_examples.find(id) == _batch_grouped_examples.end())
  {
    // can't learn from this interaction
    logger.out_warn(
        "Events with event id [{}] were processed but "
        "no valid interaction found. Skipping..",
        id);
    clear_examples = true;
    return false;
  }

  je = &_batch_grouped_examples[id];
  if (!je->ok)
  {
    // don't learn from this interaction
    logger.out_warn(
        "Interaction with event id [{}] has been invalidated due to malformed "
        "observation. Skipping...",
        id);
    clear_examples = true;
    return false;
  }

  je->calc_reward(_loop_info.default_reward, _reward_calculation.value(), logger);

  if (!je->is_joined_event_learnable())
  {
    _current_je_is_skip_learn = true;
    clear_examples = true;
    return false;
  }

  if (_binary_to_json)
  {
    clear_examples = true;
    return true;
  }

  if (!je->fill_in_label(examples, logger))
  {
    clear_examples = true;
    return false;
  }

  if (multiline)
  {
    // add an empty example to signal end-of-multiline
    examples.push_back(VW::new_unused_example(*_vw));
    _vw->example_parser->lbl_parser.default_label(examples.back()->l);
    examples.back()->is_newline = true;
  }

  return true;
}

void example_joiner::persist_metrics(VW::metric_sink& metrics)
{
  if (_vw->example_parser->metrics)
  {
    _vw->example_parser->metrics->number_of_skipped_events = _joiner_metrics.number_of_skipped_events;
    metrics.set_uint("number_skipped_events", _vw->example_parser->metrics->number_of_skipped_events, true);

    _vw->example_parser->metrics->dsjson_sum_cost_original = _joiner_metrics.sum_cost_original;
    metrics.set_float("dsjson_sum_cost_original", _vw->example_parser->metrics->dsjson_sum_cost_original, true);

    if (!_joiner_metrics.first_event_id.empty())
    {
      _vw->example_parser->metrics->first_event_id = std::move(_joiner_metrics.first_event_id);
      metrics.set_string("first_event_id", _vw->example_parser->metrics->first_event_id, true);

      _vw->example_parser->metrics->first_event_time = std::move(
          date::format("%FT%TZ", date::floor<std::chrono::microseconds>(_joiner_metrics.first_event_timestamp)));
      metrics.set_string("first_event_time", _vw->example_parser->metrics->first_event_time, true);
    }
    if (!_joiner_metrics.last_event_id.empty())
    {
      _vw->example_parser->metrics->last_event_id = std::move(_joiner_metrics.last_event_id);
      metrics.set_string("last_event_id", _vw->example_parser->metrics->last_event_id, true);

      _vw->example_parser->metrics->last_event_time = std::move(
          date::format("%FT%TZ", date::floor<std::chrono::microseconds>(_joiner_metrics.last_event_timestamp)));
      metrics.set_string("last_event_time", _vw->example_parser->metrics->last_event_time, true);
    }
  }
}

bool example_joiner::processing_batch() { return !_batch_event_order.empty(); }
bool example_joiner::current_event_is_skip_learn() { return _current_je_is_skip_learn; }
void example_joiner::on_new_batch() {}
void example_joiner::on_batch_read() {}

metrics::joiner_metrics example_joiner::get_metrics() { return _joiner_metrics; }

void example_joiner::apply_cli_overrides(VW::workspace*, const VW::external::parser_options&) {}

#ifdef RL_WINDOWS_GETOBJECT_MACRO_UNDEF
#  undef RL_WINDOWS_GETOBJECT_MACRO_UNDEF
#  pragma pop_macro("GetObject")
#endif
