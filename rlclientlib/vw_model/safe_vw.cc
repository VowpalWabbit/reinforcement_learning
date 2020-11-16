#include "safe_vw.h"

// VW headers
#include "example.h"
#include "parse_example_json.h"
#include "parser.h"
#include "v_array.h"

#include <iostream>
namespace mm = reinforcement_learning::model_management;

namespace reinforcement_learning {
  static const std::string SEED_TAG = "seed=";

  safe_vw::safe_vw(const std::shared_ptr<safe_vw>& master) : _master(master)
  {
    _vw = VW::seed_vw_model(_master->_vw, "", nullptr, nullptr);
  }

  safe_vw::safe_vw(const char* model_data, size_t len)
  {
    io_buf buf;
    buf.add_file(VW::io::create_buffer_view(model_data, len));

    _vw = VW::initialize("--quiet --json", &buf, false, nullptr, nullptr);
  }

  safe_vw::safe_vw(const char* model_data, size_t len, const std::string& vw_commandline)
  {
	io_buf buf;
	buf.add_file(VW::io::create_buffer_view(model_data, len));

	_vw = VW::initialize(vw_commandline, &buf, false, nullptr, nullptr);
  }

  safe_vw::safe_vw(const std::string& vw_commandline)
  {	  
	  _vw = VW::initialize(vw_commandline);
  }

  safe_vw::~safe_vw()
  {
    // cleanup examples
    for (auto&& ex : _example_pool) {
      VW::dealloc_example(_vw->p->lp.delete_label, *ex);
      ::free_it(ex);
    }

    // cleanup VW instance
    reset_source(*_vw, _vw->num_bits);

    VW::finish(*_vw);
  }

  example* safe_vw::get_or_create_example()
  {
    // alloc new element if we don't have any left
    if (_example_pool.size() == 0) {
      auto ex = VW::alloc_examples(0, 1);
      _vw->p->lp.default_label(&ex->l);

      return ex;
    }

    // get last element
    example* ex = _example_pool.back();
    _example_pool.pop_back();

    VW::empty_example(*_vw, *ex);
    _vw->p->lp.default_label(&ex->l);

    return ex;
  }

  example& safe_vw::get_or_create_example_f(void* vw) { return *(((safe_vw*)vw)->get_or_create_example()); }

  void safe_vw::parse_context_with_pdf(const char* context, std::vector<int>& actions, std::vector<float>& scores)
  {
    DecisionServiceInteraction interaction;

    auto examples = v_init<example*>();
    examples.push_back(get_or_create_example());

    std::vector<char> line_vec(context, context + strlen(context) + 1);

    VW::read_line_decision_service_json<false>(*_vw, examples, &line_vec[0], line_vec.size(), false, get_or_create_example_f, this, &interaction);

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
    for (auto&& ex : examples) {
      _example_pool.emplace_back(ex);
    }

    // cleanup
    examples.delete_v();
  }

  void safe_vw::rank(const char* context, std::vector<int>& actions, std::vector<float>& scores)
  {
    auto examples = v_init<example*>();
    examples.push_back(get_or_create_example());

    std::vector<char> line_vec(context, context + strlen(context) + 1);

    VW::read_line_json<false>(*_vw, examples, &line_vec[0], get_or_create_example_f, this);

    // finalize example
    VW::setup_examples(*_vw, examples);

    // TODO: refactor setup_examples/read_line_json to take in multi_ex
    multi_ex examples2(examples.begin(), examples.end());

    _vw->predict(examples2);

    // prediction are in the first-example
    const auto& predictions = examples2[0]->pred.a_s;
    actions.resize(predictions.size());
    scores.resize(predictions.size());
    for (size_t i = 0; i < predictions.size(); ++i) {
      actions[i] = predictions[i].action;
      scores[i] = predictions[i].score;
    }

    // clean up examples and push examples back into pool for re-use
    for (auto&& ex : examples) {
      ex->pred.a_s.delete_v();
      _example_pool.emplace_back(ex);
    }

    // cleanup
    examples.delete_v();
  }

  void safe_vw::choose_continuous_action(const char* context, float& action, float& pdf_value)
  {
    auto examples = v_init<example*>();
    examples.push_back(get_or_create_example());

    std::vector<char> line_vec(context, context + strlen(context) + 1);

    VW::read_line_json<false>(*_vw, examples, &line_vec[0], get_or_create_example_f, this);
    
    // finalize example
    VW::setup_examples(*_vw, examples);

    _vw->predict(*examples[0]);

    action = examples[0]->pred.pdf_value.action;
    pdf_value = examples[0]->pred.pdf_value.pdf_value;

    for (auto&& ex : examples) {
      ex->l.cb_cont.costs.delete_v();
      _example_pool.emplace_back(ex);
    }

    // cleanup
    examples.delete_v();
  }

  void safe_vw::rank_decisions(const std::vector<const char*>& event_ids, const char* context, std::vector<std::vector<uint32_t>>& actions, std::vector<std::vector<float>>& scores)
  {
    auto examples = v_init<example*>();
    examples.push_back(get_or_create_example());

    std::vector<char> line_vec(context, context + strlen(context) + 1);

    VW::read_line_json<false>(*_vw, examples, &line_vec[0], get_or_create_example_f, this);

    // In order to control the seed for the sampling of each slot the event id + app id is passed in as the seed using the example tag.
    for(int i = 0; i < event_ids.size(); i++)
    {
      const size_t slot_example_indx = examples.size() - event_ids.size() + i;
      push_many(examples[slot_example_indx]->tag, SEED_TAG.c_str(), SEED_TAG.size());
      push_many(examples[slot_example_indx]->tag, event_ids[i], strlen(event_ids[i]));
    }

    // finalize example
    VW::setup_examples(*_vw, examples);

    // TODO: refactor setup_examples/read_line_json to take in multi_ex
    multi_ex examples2(examples.begin(), examples.end());

    _vw->predict(examples2);

    // prediction are in the first-example
    auto& predictions = examples2[0]->pred.decision_scores;
    actions.resize(predictions.size());
    scores.resize(predictions.size());
    for (size_t i = 0; i < predictions.size(); ++i) {
      actions[i].reserve(predictions[i].size());
      scores[i].reserve(predictions[i].size());
      for (size_t j = 0; j < predictions[i].size(); ++j) {
         actions[i].push_back(predictions[i][j].action);
         scores[i].push_back(predictions[i][j].score);
      }
    }

    // clean up examples and push examples back into pool for re-use
    for (auto a_s : examples[0]->pred.decision_scores)
    {
      a_s.delete_v();
    }
    examples[0]->pred.decision_scores.delete_v();
    for (auto&& ex : examples) {
      _example_pool.emplace_back(ex);
    }

    // cleanup
    examples.delete_v();
  }

  void safe_vw::rank_multi_slot_decisions(const char* event_id, uint32_t slot_count, const char* context, std::vector<std::vector<uint32_t>>& actions, std::vector<std::vector<float>>& scores)
  {
    auto examples = v_init<example*>();
    examples.push_back(get_or_create_example());

    std::vector<char> line_vec(context, context + strlen(context) + 1);

    VW::read_line_json<false>(*_vw, examples, &line_vec[0], get_or_create_example_f, this);
    // In order to control the seed for the sampling of each slot the event id + app id is passed in as the seed using the example tag.
    for(uint32_t i = 0; i < slot_count; i++)
    {
      const size_t slot_example_indx = examples.size() - slot_count + i;
      auto index_as_string = std::to_string(i);
      push_many(examples[slot_example_indx]->tag, SEED_TAG.c_str(), SEED_TAG.size());
      push_many(examples[slot_example_indx]->tag, event_id, strlen(event_id));
      push_many(examples[slot_example_indx]->tag, index_as_string.c_str(), index_as_string.size());
    }

    // finalize example
    VW::setup_examples(*_vw, examples);

    // TODO: refactor setup_examples/read_line_json to take in multi_ex
    multi_ex examples2(examples.begin(), examples.end());

    _vw->predict(examples2);

    // prediction are in the first-example
    auto& predictions = examples2[0]->pred.decision_scores;
    actions.resize(predictions.size());
    scores.resize(predictions.size());
    for (size_t i = 0; i < predictions.size(); ++i) {
      actions[i].reserve(predictions[i].size());
      scores[i].reserve(predictions[i].size());
      for (size_t j = 0; j < predictions[i].size(); ++j) {
         actions[i].push_back(predictions[i][j].action);
         scores[i].push_back(predictions[i][j].score);
      }
    }

    // clean up examples and push examples back into pool for re-use
    for (auto a_s : examples[0]->pred.decision_scores)
    {
      a_s.delete_v();
    }
    examples[0]->pred.decision_scores.delete_v();
    for (auto&& ex : examples) {
      _example_pool.emplace_back(ex);
    }

    // cleanup
    examples.delete_v();
  }

const char* safe_vw::id() const {
  return _vw->id.c_str();
}

mm::model_type_t safe_vw::get_model_type(const std::string& args)
{
  // slates == slates
  if (args.find("slates") != std::string::npos)
  {
    return mm::model_type_t::SLATES;
  }

  // ccb = ccb && !slates
  if (args.find("ccb_explore_adf") != std::string::npos)
  {
    return mm::model_type_t::CCB;
  }

  // cb = !slates && !ccb && cb
  if (args.find("cb_explore_adf") != std::string::npos)
  {
    return mm::model_type_t::CB;
  }

  if (args.find("cats") != std::string::npos)
  {
    return mm::model_type_t::CA;
  }

  return mm::model_type_t::UNKNOWN;
}

// TODO make this const when was_supplied becomes const.
mm::model_type_t safe_vw::get_model_type(const VW::config::options_i* args)
{
    // slates == slates
  if (args->was_supplied("slates"))
  {
    return mm::model_type_t::SLATES;
  }

  // ccb = ccb && !slates
  if (args->was_supplied("ccb_explore_adf"))
  {
    return mm::model_type_t::CCB;
  }

  // cb = !slates && !ccb && cb
  if (args->was_supplied("cb_explore_adf"))
  {
    return mm::model_type_t::CB;
  }

  if (args->was_supplied("cats"))
  {
    return mm::model_type_t::CA;
  }

  return mm::model_type_t::UNKNOWN;
}

bool safe_vw::is_compatible(const std::string& args) const {
  const auto local_model_type = get_model_type(args);
  const auto inbound_model_type = get_model_type(_vw->options);

  // This really is an error but errors cant be reported here...
  if(local_model_type == mm::model_type_t::UNKNOWN || inbound_model_type ==  mm::model_type_t::UNKNOWN)
  {
    return false;
  }

  return local_model_type == inbound_model_type;
}

bool safe_vw::is_CB_to_CCB_model_upgrade(const std::string& args) const
{
    const auto local_model_type = get_model_type(args);
    const auto inbound_model_type = get_model_type(_vw->options);

	return local_model_type == mm::model_type_t::CCB && inbound_model_type == mm::model_type_t::CB;    
}

safe_vw_factory::safe_vw_factory(const std::string& command_line)
  : _command_line(command_line)
{}

safe_vw_factory::safe_vw_factory(const model_management::model_data& master_data)
  : _master_data(master_data)
  {}

safe_vw_factory::safe_vw_factory(const model_management::model_data&& master_data)
  : _master_data(master_data)
  {}

safe_vw_factory::safe_vw_factory(const model_management::model_data& master_data, const std::string& command_line)
  : _master_data(master_data), _command_line(command_line)
  {}

safe_vw_factory::safe_vw_factory(const model_management::model_data&& master_data, const std::string& command_line)
  : _master_data(master_data), _command_line(command_line)
  {}

safe_vw* safe_vw_factory::operator()() 
{
    if (_master_data.data() && _command_line.size() > 0)
    {
      // Construct new vw object from raw model data and command line argument
      return new safe_vw(_master_data.data(), _master_data.data_sz(), _command_line);
    }
    else if (_master_data.data())
    {
      // Construct new vw object from raw model data.
      return new safe_vw(_master_data.data(), _master_data.data_sz());
    }
    else
    {
      return new safe_vw(_command_line);
    }
  }
}
