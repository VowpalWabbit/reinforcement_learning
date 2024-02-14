#include "safe_vw.h"
#include "constants.h"
#include "api_status.h"

// VW headers
#include "vw/config/options.h"
#include "vw/core/debug_print.h"
#include "vw/core/example.h"
#include "vw/core/parse_example_json.h"
#include "vw/fb_parser/parse_example_flatbuffer.h"
#include "vw/core/parser.h"
#include "vw/core/v_array.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <utility>
#include <sstream>

namespace mm = reinforcement_learning::model_management;

namespace reinforcement_learning
{
static const std::string SEED_TAG = "seed=";

const char* get_config_value_for_input_serialization(input_serialization input_format)
{
  switch (input_format)
  {
    case input_serialization::vwjson: return value::VWJSON_INPUT_SERIALIZATION;
    case input_serialization::dsjson: return value::DSJSON_INPUT_SERIALIZATION;
    case input_serialization::flatbuffer: return value::VWFB_INPUT_SERIALIZATION;
    default: return nullptr;
  }
}

input_serialization get_serializtion_for_config_value(const char* config_value, input_serialization default)
{
  if (config_value == nullptr) { return default; }

  if (strcmp(config_value, value::VWJSON_INPUT_SERIALIZATION) == 0) { return input_serialization::vwjson; }
  if (strcmp(config_value, value::DSJSON_INPUT_SERIALIZATION) == 0) { return input_serialization::dsjson; }
  if (strcmp(config_value, value::VWFB_INPUT_SERIALIZATION) == 0) { return input_serialization::flatbuffer; }

  return input_serialization::unknown;
}

const vw_input_type_configurator& vw_input_type_configurator::get_vw_model_input_adapter_factory()
{
  static const input_serialization allowed_formats[2] = {input_serialization::vwjson, input_serialization::flatbuffer};
  static const vw_input_type_configurator vw_model_input_adapter_factory(allowed_formats);
  return vw_model_input_adapter_factory;
}

const vw_input_type_configurator& vw_input_type_configurator::get_pdf_model_input_adapter_factory()
{
  static const input_serialization allowed_formats[1] = {input_serialization::dsjson};
  static const vw_input_type_configurator pdf_model_input_adapter_factory(allowed_formats);
  return pdf_model_input_adapter_factory;
}

input_serialization vw_input_type_configurator::configure_input_serialization(const utility::configuration& config, i_trace* trace_logger, api_status* status) const
{
  const char* default_input_serialization = get_config_value_for_input_serialization(_allowed_formats[0]);
  const auto input_serialization_str = config.get(name::INPUT_SERIALIZATION, default_input_serialization);
  auto input_serialization = get_serializtion_for_config_value(input_serialization_str, _allowed_formats[0]);

  if (input_serialization == input_serialization::unknown ||
      std::find(_allowed_formats, _allowed_formats + _allowed_format_count, input_serialization) == _allowed_formats + _allowed_format_count)
  {
    std::stringstream ss;
    std::for_each(_allowed_formats, _allowed_formats + _allowed_format_count, [&ss](const auto& format) {
      ss << get_config_value_for_input_serialization(format) << ", ";
    });

    TRACE_ERROR_LS(trace_logger, status, input_serialization_unsupported)
        << "Input serialization type is not supported: " << input_serialization_str << ". Supported types are: " << ss.str();

    return input_serialization::unknown;
  }

  return input_serialization;
}

safe_vw::safe_vw(std::shared_ptr<safe_vw> master) : _master(std::move(master))
{
  _vw = VW::seed_vw_model(_master->_vw, "", nullptr, nullptr);
  _input_format = _master->_input_format;
  init();
}

safe_vw::safe_vw(const char* model_data, size_t len, input_serialization input_format)
{
  VW::io_buf buf;
  buf.add_file(VW::io::create_buffer_view(model_data, len));

  _vw = VW::initialize("--quiet --json", &buf, false, nullptr, nullptr);
  _input_format = input_format;
  init();
}

safe_vw::safe_vw(const char* model_data, size_t len, const std::string& vw_commandline, input_serialization input_format)
{
  VW::io_buf buf;
  buf.add_file(VW::io::create_buffer_view(model_data, len));

  _vw = VW::initialize(vw_commandline, &buf, false, nullptr, nullptr);
  _input_format = input_format;
  init();
}

safe_vw::safe_vw(const std::string& vw_commandline, input_serialization input_format)
{
  _vw = VW::initialize(vw_commandline);
  _input_format = input_format;
  init();
}

safe_vw::~safe_vw()
{
  // cleanup examples
  for (auto&& ex : _example_pool) { delete ex; }

  // cleanup VW instance
  VW::details::reset_source(*_vw, _vw->initial_weights_config.num_bits);

  VW::finish(*_vw);
}

VW::example* safe_vw::get_or_create_example()
{
  // alloc new element if we don't have any left
  if (_example_pool.empty())
  {
    auto* ex = new VW::example; // new VW APIs suggest allocating an example using new (VW::alloc_examples is deprecated)
    _vw->parser_runtime.example_parser->lbl_parser.default_label(ex->l);

    return ex;
  }

  // get last element
  VW::example* ex = _example_pool.back();
  _example_pool.pop_back();

  VW::empty_example(*_vw, *ex);
  _vw->parser_runtime.example_parser->lbl_parser.default_label(ex->l);

  return ex;
}

VW::example& safe_vw::get_or_create_example_f(void* vw) { return *(((safe_vw*)vw)->get_or_create_example()); }

namespace detail
{
  template <bool audit = false>
  inline void ensure_audit_buffer(VW::workspace& w)
  {
    if VW_STD17_CONSTEXPR (audit)
    {
      w.output_runtime.audit_buffer->clear();
    }
  }

  template <input_serialization input = input_serialization::unknown>
  struct example_parser
  {
    //static_assert(false, "Unsupported input format");
  };

  template <>
  struct example_parser<input_serialization::vwjson>
  {
    template <bool audit>
    static void parse_context(VW::workspace& w, string_view context, VW::example_factory_t ex_fac, VW::multi_ex& examples)
    {
      // copy due to destructive parsing by rapidjson
      std::string line_vec(context);

      VW::parsers::json::read_line_json<audit>(w, examples, &line_vec[0], line_vec.size(), ex_fac);
    }
  };

  template <>
  struct example_parser<input_serialization::flatbuffer>
  {
    template <bool audit>
    static void parse_context(VW::workspace& w, string_view context, VW::example_factory_t ex_fac, VW::multi_ex& examples, VW::example_sink_f ex_sink)
    {
      VW::parsers::flatbuffer::read_span_flatbuffer(&w, reinterpret_cast<const uint8_t*>(context.data()), context.size(), ex_fac, examples, ex_sink);
    }
  };

  template <bool audit>
  void parse_context(VW::workspace& w, string_view context, input_serialization input_format, VW::example_factory_t ex_fac, VW::multi_ex& examples, VW::example_sink_f ex_sink)
  {
    examples.push_back(&ex_fac());
    ensure_audit_buffer<audit>(w);

    switch (input_format)
    {
      case input_serialization::vwjson:
        example_parser<input_serialization::vwjson>::parse_context<audit>(w, context, ex_fac, examples);
        break;
      case input_serialization::flatbuffer:
        example_parser<input_serialization::flatbuffer>::parse_context<audit>(w, context, ex_fac, examples, ex_sink);
        break;
      default:
        throw std::runtime_error("Unsupported input format");
  }
}
}

void safe_vw::parse_context(string_view context, VW::multi_ex& examples)
{
  VW::example_factory_t ex_fac = [this]() -> VW::example& { return get_or_create_example_f(this); };
  VW::example_sink_f ex_sink = [this](VW::multi_ex&& ec)
  {
    for (auto&& ex : ec)
    {
      ex->pred.a_s.clear();
      _example_pool.emplace_back(ex);
    }
  };
  

  if (_vw->output_config.audit)
  {
    detail::parse_context<true>(*_vw, context, _input_format, ex_fac, examples, ex_sink);
  }
  else
  { detail::parse_context<false>(*_vw, context, _input_format, ex_fac, examples, ex_sink);
  }
}

void safe_vw::parse_context_with_pdf(string_view context, std::vector<int>& actions, std::vector<float>& scores)
{
  VW::example_factory_t ex_fac = [this]() -> VW::example& { return get_or_create_example_f(this); };

  // TODO: Technically, it is an error to call this IFF input_format != dsjson, though we should consider supporting
  // Flatbuffers here, too.
  // if (_input_format != input_serialization::dsjson)
  // {
  //   throw std::runtime_error("Input format must be DSJSON when using parsing input with PDF/PMF.");
  // }

  VW::parsers::json::decision_service_interaction interaction;

  VW::multi_ex examples;
  examples.push_back(get_or_create_example());

  // copy due to destructive parsing by rapidjson
  std::string line_vec(context);

  if (_vw->output_config.audit)
  {
    _vw->output_runtime.audit_buffer->clear();
    VW::read_line_decision_service_json<true>(
        *_vw, examples, &line_vec[0], line_vec.size(), false, ex_fac, &interaction);
  }
  else
  {
    VW::read_line_decision_service_json<false>(
        *_vw, examples, &line_vec[0], line_vec.size(), false, ex_fac, &interaction);
  }

  // finalize example
  VW::setup_examples(*_vw, examples);

  actions.resize(interaction.probabilities.size());
  scores.resize(interaction.probabilities.size());
  for (int i = 0; i < interaction.probabilities.size(); i++)
  {
    actions[i] = i;
    scores[i] = interaction.probabilities[i];
  }

  // clean up examples and push examples back into pool for re-use
  for (auto&& ex : examples) { _example_pool.emplace_back(ex); }
}

void safe_vw::rank(string_view context, std::vector<int>& actions, std::vector<float>& scores)
{
  VW::multi_ex examples;
  parse_context(context, examples);

  // finalize example
  VW::setup_examples(*_vw, examples);

  _vw->predict(examples);

  // prediction are in the first-example
  const auto& predictions = examples[0]->pred.a_s;
  actions.resize(predictions.size());
  scores.resize(predictions.size());
  for (size_t i = 0; i < predictions.size(); ++i)
  {
    actions[i] = predictions[i].action;
    scores[i] = predictions[i].score;
  }

  // clean up examples and push examples back into pool for re-use
  for (auto&& ex : examples)
  {
    ex->pred.a_s.clear();
    _example_pool.emplace_back(ex);
  }
}

void safe_vw::choose_continuous_action(string_view context, float& action, float& pdf_value)
{
  VW::multi_ex examples;
  parse_context(context, examples);

  // finalize example
  VW::setup_examples(*_vw, examples);

  _vw->predict(*examples[0]);

  action = examples[0]->pred.pdf_value.action;
  pdf_value = examples[0]->pred.pdf_value.pdf_value;

  for (auto&& ex : examples)
  {
    ex->l.cb_cont.costs.clear();
    _example_pool.emplace_back(ex);
  }
}

void safe_vw::rank_decisions(const std::vector<const char*>& event_ids, string_view context,
    std::vector<std::vector<uint32_t>>& actions, std::vector<std::vector<float>>& scores)
{
  VW::multi_ex examples;
  parse_context(context, examples);

  // In order to control the seed for the sampling of each slot the event id + app id is passed in as the seed using the
  // example tag.
  for (int i = 0; i < event_ids.size(); i++)
  {
    const size_t slot_example_indx = examples.size() - event_ids.size() + i;
    auto& tag = examples[slot_example_indx]->tag;
    std::copy(SEED_TAG.begin(), SEED_TAG.end(), std::back_inserter(tag));
    std::copy(event_ids[i], event_ids[i] + strlen(event_ids[i]), std::back_inserter(tag));
  }

  // finalize example
  VW::setup_examples(*_vw, examples);

  _vw->predict(examples);

  // prediction are in the first-example
  auto& predictions = examples[0]->pred.decision_scores;
  actions.resize(predictions.size());
  scores.resize(predictions.size());
  for (size_t i = 0; i < predictions.size(); ++i)
  {
    actions[i].reserve(predictions[i].size());
    scores[i].reserve(predictions[i].size());
    for (size_t j = 0; j < predictions[i].size(); ++j)
    {
      actions[i].push_back(predictions[i][j].action);
      scores[i].push_back(predictions[i][j].score);
    }
  }

  // clean up examples and push examples back into pool for re-use
  examples[0]->pred.decision_scores.clear();
  for (auto&& ex : examples) { _example_pool.emplace_back(ex); }
}

void safe_vw::rank_multi_slot_decisions(const char* event_id, const std::vector<std::string>& slot_ids,
    string_view context, std::vector<std::vector<uint32_t>>& actions, std::vector<std::vector<float>>& scores)
{
  VW::multi_ex examples;
  parse_context(context, examples);

  // In order to control the seed for the sampling of each slot the event id + app id is passed in as the seed using the
  // example tag.
  for (uint32_t i = 0; i < slot_ids.size(); i++)
  {
    const size_t slot_example_indx = examples.size() - slot_ids.size() + i;
    auto& tag = examples[slot_example_indx]->tag;
    std::copy(SEED_TAG.begin(), SEED_TAG.end(), std::back_inserter(tag));
    std::copy(event_id, event_id + strlen(event_id), std::back_inserter(tag));
    std::copy(slot_ids[i].begin(), slot_ids[i].end(), std::back_inserter(tag));
  }

  // finalize example
  VW::setup_examples(*_vw, examples);

  _vw->predict(examples);

  // prediction are in the first-example
  auto& predictions = examples[0]->pred.decision_scores;
  actions.resize(predictions.size());
  scores.resize(predictions.size());
  for (size_t i = 0; i < predictions.size(); ++i)
  {
    actions[i].reserve(predictions[i].size());
    scores[i].reserve(predictions[i].size());
    for (size_t j = 0; j < predictions[i].size(); ++j)
    {
      actions[i].push_back(predictions[i][j].action);
      scores[i].push_back(predictions[i][j].score);
    }
  }

  // clean up examples and push examples back into pool for re-use
  examples[0]->pred.decision_scores.clear();
  for (auto&& ex : examples) { _example_pool.emplace_back(ex); }
}

const char* safe_vw::id() const { return _vw->id.c_str(); }

mm::model_type_t safe_vw::get_model_type(const std::string& args)
{
  // slates == slates
  if (args.find("slates") != std::string::npos) { return mm::model_type_t::SLATES; }

  // ccb = ccb && !slates
  if (args.find("ccb_explore_adf") != std::string::npos) { return mm::model_type_t::CCB; }

  // cb = !slates && !ccb && cb
  if (args.find("cb_explore_adf") != std::string::npos) { return mm::model_type_t::CB; }

  if (args.find("cats") != std::string::npos) { return mm::model_type_t::CA; }

  return mm::model_type_t::UNKNOWN;
}

// TODO make this const when was_supplied becomes const.
mm::model_type_t safe_vw::get_model_type(const VW::config::options_i* args)
{
  // slates == slates
  if (args->was_supplied("slates")) { return mm::model_type_t::SLATES; }

  // ccb = ccb && !slates
  if (args->was_supplied("ccb_explore_adf")) { return mm::model_type_t::CCB; }

  // cb = !slates && !ccb && cb
  if (args->was_supplied("cb_explore_adf")) { return mm::model_type_t::CB; }

  if (args->was_supplied("cats")) { return mm::model_type_t::CA; }

  return mm::model_type_t::UNKNOWN;
}

bool safe_vw::is_compatible(const std::string& args) const
{
  const auto local_model_type = get_model_type(args);
  const auto inbound_model_type = get_model_type(_vw->options.get());

  // This really is an error but errors cant be reported here...
  if (local_model_type == mm::model_type_t::UNKNOWN || inbound_model_type == mm::model_type_t::UNKNOWN)
  {
    return false;
  }

  return local_model_type == inbound_model_type;
}

bool safe_vw::is_CB_to_CCB_model_upgrade(const std::string& args) const
{
  const auto local_model_type = get_model_type(args);
  const auto inbound_model_type = get_model_type(_vw->options.get());

  return local_model_type == mm::model_type_t::CCB && inbound_model_type == mm::model_type_t::CB;
}

string_view safe_vw::get_audit_data() const
{
  if (_vw->output_config.audit)
  {
    return string_view(_vw->output_runtime.audit_buffer->data(), _vw->output_runtime.audit_buffer->size());
  }
  else { return string_view(); }
}

void safe_vw::init()
{
  if (_vw->output_config.audit)
  {
    _vw->output_runtime.audit_buffer = std::make_shared<std::vector<char>>();
    _vw->output_runtime.audit_writer = VW::io::create_vector_writer(_vw->output_runtime.audit_buffer);
  }
}

safe_vw_factory::safe_vw_factory(std::string command_line, input_serialization input_format) : _command_line(std::move(command_line)), _input_format(input_format) {}

safe_vw_factory::safe_vw_factory(const model_management::model_data& master_data, input_serialization input_format) : _master_data(master_data), _input_format(input_format) {}

safe_vw_factory::safe_vw_factory(const model_management::model_data&& master_data, input_serialization input_format) : _master_data(master_data), _input_format(input_format) {}

safe_vw_factory::safe_vw_factory(const model_management::model_data& master_data, std::string command_line, input_serialization input_format)
    : _master_data(master_data), _command_line(std::move(command_line)), _input_format(input_format)
{
}

safe_vw_factory::safe_vw_factory(const model_management::model_data&& master_data, std::string command_line, input_serialization input_format)
    : _master_data(master_data), _command_line(std::move(command_line)), _input_format(input_format)
{
}

safe_vw* safe_vw_factory::operator()()
{
  if ((_master_data.data() != nullptr) && !_command_line.empty())
  {
    // Construct new vw object from raw model data and command line argument
    return new safe_vw(_master_data.data(), _master_data.data_sz(), _command_line, _input_format);
  }
  if (_master_data.data() != nullptr)
  {
    // Construct new vw object from raw model data.
    return new safe_vw(_master_data.data(), _master_data.data_sz(), _input_format);
  }
  return new safe_vw(_command_line, _input_format);
}
}  // namespace reinforcement_learning
