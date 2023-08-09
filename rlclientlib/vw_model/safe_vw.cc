#include "safe_vw.h"

// VW headers
#include "vw/config/options.h"
#include "vw/core/debug_print.h"
#include "vw/core/example.h"
#include "vw/core/parse_example_json.h"
#include "vw/core/parser.h"
#include "vw/core/v_array.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <utility>

namespace mm = reinforcement_learning::model_management;

namespace reinforcement_learning
{
static const std::string SEED_TAG = "seed=";

safe_vw::safe_vw(std::shared_ptr<safe_vw> master) : _master(std::move(master))
{
  _vw = VW::seed_vw_model(_master->_vw, "", nullptr, nullptr);
  init();
}

safe_vw::safe_vw(const char* model_data, size_t len)
{
  io_buf buf;
  buf.add_file(VW::io::create_buffer_view(model_data, len));

  _vw = VW::initialize("--quiet --json", &buf, false, nullptr, nullptr);
  init();
}

safe_vw::safe_vw(const char* model_data, size_t len, const std::string& vw_commandline)
{
  io_buf buf;
  buf.add_file(VW::io::create_buffer_view(model_data, len));

  _vw = VW::initialize(vw_commandline, &buf, false, nullptr, nullptr);
  init();
}

safe_vw::safe_vw(const std::string& vw_commandline)
{
  _vw = VW::initialize(vw_commandline);
  init();
}

safe_vw::~safe_vw()
{
  // cleanup examples
  for (auto&& ex : _example_pool) { VW::dealloc_examples(ex, 1); }

  // cleanup VW instance
  VW::details::reset_source(*_vw, _vw->initial_weights_config.num_bits);

  VW::finish(*_vw);
}

VW::example* safe_vw::get_or_create_example()
{
  // alloc new element if we don't have any left
  if (_example_pool.empty())
  {
    auto* ex = VW::alloc_examples(1);
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

void safe_vw::parse_context_with_pdf(string_view context, std::vector<int>& actions, std::vector<float>& scores)
{
  VW::parsers::json::decision_service_interaction interaction;

  VW::multi_ex examples;
  examples.push_back(get_or_create_example());

  // copy due to destructive parsing by rapidjson
  std::string line_vec(context);
  VW::example_factory_t ex_fac = [this]() -> VW::example& { return get_or_create_example_f(this); };

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
  examples.push_back(get_or_create_example());

  // copy due to destructive parsing by rapidjson
  std::string line_vec(context);
  VW::example_factory_t ex_fac = [this]() -> VW::example& { return get_or_create_example_f(this); };

  if (_vw->output_config.audit)
  {
    _vw->output_runtime.audit_buffer->clear();
    VW::parsers::json::read_line_json<true>(*_vw, examples, &line_vec[0], line_vec.size(), ex_fac);
  }
  else { VW::parsers::json::read_line_json<false>(*_vw, examples, &line_vec[0], line_vec.size(), ex_fac); }

  // finalize example
  VW::setup_examples(*_vw, examples);

  // TODO: refactor setup_examples to take in multi_ex
  VW::multi_ex examples2(examples.begin(), examples.end());

  _vw->predict(examples2);

  // prediction are in the first-example
  const auto& predictions = examples2[0]->pred.a_s;
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
  examples.push_back(get_or_create_example());

  // copy due to destructive parsing by rapidjson
  std::string line_vec(context);
  VW::example_factory_t ex_fac = [this]() -> VW::example& { return get_or_create_example_f(this); };

  if (_vw->output_config.audit)
  {
    _vw->output_runtime.audit_buffer->clear();
    VW::parsers::json::read_line_json<true>(*_vw, examples, &line_vec[0], line_vec.size(), ex_fac);
  }
  else { VW::parsers::json::read_line_json<false>(*_vw, examples, &line_vec[0], line_vec.size(), ex_fac); }

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
  examples.push_back(get_or_create_example());

  // copy due to destructive parsing by rapidjson
  std::string line_vec(context);
  VW::example_factory_t ex_fac = [this]() -> VW::example& { return get_or_create_example_f(this); };

  if (_vw->output_config.audit)
  {
    _vw->output_runtime.audit_buffer->clear();
    VW::parsers::json::read_line_json<true>(*_vw, examples, &line_vec[0], line_vec.size(), ex_fac);
  }
  else { VW::parsers::json::read_line_json<false>(*_vw, examples, &line_vec[0], line_vec.size(), ex_fac); }

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

  // TODO: refactor setup_examples to take in multi_ex
  VW::multi_ex examples2(examples.begin(), examples.end());

  _vw->predict(examples2);

  // prediction are in the first-example
  auto& predictions = examples2[0]->pred.decision_scores;
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
  examples.push_back(get_or_create_example());

  // copy due to destructive parsing by rapidjson
  std::string line_vec(context);
  VW::example_factory_t ex_fac = [this]() -> VW::example& { return get_or_create_example_f(this); };

  if (_vw->output_config.audit)
  {
    _vw->output_runtime.audit_buffer->clear();
    VW::parsers::json::read_line_json<true>(*_vw, examples, &line_vec[0], line_vec.size(), ex_fac);
  }
  else { VW::parsers::json::read_line_json<false>(*_vw, examples, &line_vec[0], line_vec.size(), ex_fac); }

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

  // TODO: refactor setup_examples to take in multi_ex
  VW::multi_ex examples2(examples.begin(), examples.end());

  _vw->predict(examples2);

  // prediction are in the first-example
  auto& predictions = examples2[0]->pred.decision_scores;
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
  if (_vw->output_config.audit) { return string_view(_vw->output_runtime.audit_buffer->data(), _vw->output_runtime.audit_buffer->size()); }
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

safe_vw_factory::safe_vw_factory(std::string command_line) : _command_line(std::move(command_line)) {}

safe_vw_factory::safe_vw_factory(const model_management::model_data& master_data) : _master_data(master_data) {}

safe_vw_factory::safe_vw_factory(const model_management::model_data&& master_data) : _master_data(master_data) {}

safe_vw_factory::safe_vw_factory(const model_management::model_data& master_data, std::string command_line)
    : _master_data(master_data), _command_line(std::move(command_line))
{
}

safe_vw_factory::safe_vw_factory(const model_management::model_data&& master_data, std::string command_line)
    : _master_data(master_data), _command_line(std::move(command_line))
{
}

safe_vw* safe_vw_factory::operator()()
{
  if ((_master_data.data() != nullptr) && !_command_line.empty())
  {
    // Construct new vw object from raw model data and command line argument
    return new safe_vw(_master_data.data(), _master_data.data_sz(), _command_line);
  }
  if (_master_data.data() != nullptr)
  {
    // Construct new vw object from raw model data.
    return new safe_vw(_master_data.data(), _master_data.data_sz());
  }
  return new safe_vw(_command_line);
}
}  // namespace reinforcement_learning
