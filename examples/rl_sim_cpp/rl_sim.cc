#include "constants.h"
#include "live_model.h"
#include "multistep.h"
#include "person.h"
#include "rl_sim_cpp.h"
#include "simulation_stats.h"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <thread>

using namespace std;

std::string get_dist_str(const reinforcement_learning::ranking_response& response);
std::string get_dist_str(const reinforcement_learning::slot_response& response);
std::string get_dist_str(const reinforcement_learning::slot_entry& response);

int rl_sim::loop()
{
  if (!init()) return -1;

  switch (_loop_kind)
  {
    case CB:
      return cb_loop();
    case CA:
      return ca_loop();
    case CCB:
      return ccb_loop();
    case Slates:
      return slates_loop();
    case Multistep:
      return multistep_loop();
    default:
      std::cout << "Invalid loop kind:" << _loop_kind << std::endl;
      return -1;
  }
}

int rl_sim::cb_loop()
{
  r::ranking_response response;
  simulation_stats<size_t> stats;

  while (_run_loop)
  {
    auto& p = pick_a_random_person();
    const auto context_features = p.get_features();
    const auto action_features = get_action_features();
    const auto context_json = create_context_json(context_features, action_features);
    const auto req_id = create_event_id();
    r::api_status status;

    // Choose an action
    if (_rl->choose_rank(req_id.c_str(), context_json.c_str(), response, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    // Use the chosen action
    size_t chosen_action;
    if (response.get_chosen_action_id(chosen_action) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    // What outcome did this action get?
    const auto outcome = p.get_outcome(_topics[chosen_action]);

    // Report outcome received
    if (_rl->report_outcome(req_id.c_str(), outcome, &status) != err::success && outcome > 0.00001f)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    stats.record(p.id(), chosen_action, outcome);

    std::cout << " " << stats.count() << ", ctxt, " << p.id() << ", action, " << chosen_action << ", outcome, "
              << outcome << ", dist, " << get_dist_str(response) << ", " << stats.get_stats(p.id(), chosen_action)
              << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  return 0;
}

std::string create_episode_id(size_t episode_id)
{
  std::ostringstream oss;
  oss << "episode"
      << "-" << episode_id;
  return oss.str();
}

int rl_sim::multistep_loop()
{
  r::ranking_response response;
  simulation_stats<size_t> stats;

  const size_t episode_length = 2;
  size_t episode_indx = 0;
  while (_run_loop)
  {
    const std::string episode_id = create_episode_id(episode_indx++);
    r::api_status status;
    r::episode_state episode(episode_id.c_str());

    std::string previous_id;
    float episodic_outcome = 0;

    for (size_t i = 0; i < episode_length; ++i)
    {
      auto& p = pick_a_random_person();
      const auto context_features = p.get_features();
      const auto action_features = get_action_features();
      const auto context_json = create_context_json(context_features, action_features);
      const auto req_id = create_event_id();

      r::ranking_response response1;
      if (_rl->request_episodic_decision(req_id.c_str(), i == 0 ? nullptr : previous_id.c_str(), context_json.c_str(),
              response, episode, &status) != err::success)
      {
        std::cout << status.get_error_msg() << std::endl;
        return -1;
      }

      size_t chosen_action;
      if (response.get_chosen_action_id(chosen_action) != err::success)
      {
        std::cout << status.get_error_msg() << std::endl;
        continue;
      }

      const auto outcome_per_step = p.get_outcome(_topics[chosen_action]);
      stats.record(p.id(), chosen_action, outcome_per_step);

      std::cout << " " << stats.count() << ", ctxt, " << p.id() << ", action, " << chosen_action << ", outcome, "
                << outcome_per_step << ", dist, " << get_dist_str(response) << ", "
                << stats.get_stats(p.id(), chosen_action) << std::endl;

      episodic_outcome += outcome_per_step;
      previous_id = req_id;
    }

    if (_rl->report_outcome(episode.get_episode_id(), episodic_outcome, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  return 0;
}

int rl_sim::ca_loop()
{
  r::continuous_action_response response;
  simulation_stats<float> stats;
  while (_run_loop)
  {
    auto& joint = pick_a_random_joint();
    const auto context_features = joint.get_features();
    const auto context_json = create_context_json(context_features);
    const auto req_id = create_event_id();
    r::api_status status;

    if (_rl->request_continuous_action(req_id.c_str(), context_json, response, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }
    const auto chosen_action = response.get_chosen_action();
    const auto outcome = joint.get_outcome(chosen_action);
    if (_rl->report_outcome(req_id.c_str(), outcome, &status) != err::success && outcome > 0.00001f)
    {
      std::cout << status.get_error_msg() << std::endl;
    }

    stats.record(joint.id(), chosen_action, outcome);

    std::cout << " " << stats.count() << " - ctxt: " << joint.id() << ", action: " << chosen_action
              << ", outcome: " << outcome << ", dist: " << response.get_chosen_action_pdf_value() << ", "
              << stats.get_stats(joint.id(), chosen_action) << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
  return 0;
}

int rl_sim::ccb_loop()
{
  r::multi_slot_response decision;
  simulation_stats<size_t> stats;

  while (_run_loop)
  {
    auto& p = pick_a_random_person();
    const auto context_features = p.get_features();
    const auto action_features = get_action_features();
    const auto event_id = create_event_id();
    const auto slot_json = get_slot_features();
    const auto context_json = create_context_json(context_features, action_features, slot_json);
    std::cout << context_json << std::endl;
    r::api_status status;

    // Choose an action
    if (_rl->request_multi_slot_decision(event_id.c_str(), context_json, decision, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    auto index = 0;
    for (auto& response : decision)
    {
      const auto chosen_action = response.get_action_id();
      const auto outcome = p.get_outcome(_topics[chosen_action]);

      // Report outcome received
      if (_rl->report_outcome(event_id.c_str(), index, outcome, &status) != err::success && outcome > 0.00001f)
      {
        std::cout << status.get_error_msg() << std::endl;
        continue;
      }

      stats.record(p.id(), chosen_action, outcome);

      std::cout << " " << stats.count() << ", ctxt, " << p.id() << ", action, " << chosen_action << ", slot, " << index
                << ", outcome, " << outcome << ", dist, " << get_dist_str(response) << ", "
                << stats.get_stats(p.id(), chosen_action) << std::endl;
      index++;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  return 0;
}

std::string get_slates_slot_features(size_t slot_count)
{
  std::ostringstream oss;
  // example
  // R"("_slots": [ { "slot_id":"__0"}, {"slot_id":"__1"} ])";
  oss << R"("_slots": [ )";
  for (auto idx = 0; idx < slot_count - 1; ++idx) { oss << R"({ "slot_id":"__)" << idx << R"("}, )"; }
  oss << R"({ "slot_id":"__)" << slot_count << R"("}] )";
  return oss.str();
}

int rl_sim::slates_loop()
{
  r::multi_slot_response decision;
  simulation_stats<size_t> stats;

  while (_run_loop)
  {
    auto& p = pick_a_random_person();
    const auto context_features = p.get_features();
    const auto action_features = get_slates_action_features();
    const auto event_id = create_event_id();

    const auto slot_json = get_slates_slot_features(NUM_SLATES_SLOTS);
    const auto context_json = create_context_json(context_features, action_features, slot_json);
    std::cout << context_json << std::endl;
    r::api_status status;

    // Choose an action
    if (_rl->request_multi_slot_decision(event_id.c_str(), context_json.c_str(), decision, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    float outcome = 0;
    int index = 0;
    auto actions_per_slot = _topics.size() / NUM_SLATES_SLOTS;

    for (auto& response : decision)
    {
      const auto chosen_action = response.get_action_id() + index * actions_per_slot;
      const auto slot_outcome = p.get_outcome(_topics[chosen_action]);  // TODO per-slot weights?
      stats.record(event_id, chosen_action, slot_outcome);
      outcome += slot_outcome;

      std::cout << " " << stats.count() << ", ctxt, " << p.id() << ", action, " << chosen_action << ", slot, " << index
                << ", outcome, " << outcome << ", dist, " << get_dist_str(response) << ", "
                << stats.get_stats(p.id(), chosen_action) << std::endl;

      index++;
    }

    // Report outcome received
    if (_rl->report_outcome(event_id.c_str(), outcome, &status) != err::success && outcome > 0.00001f)
    {
      std::cout << status.get_error_msg() << std::endl;
      continue;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  return 0;
}

person& rl_sim::pick_a_random_person() { return _people[rand() % _people.size()]; }

joint& rl_sim::pick_a_random_joint() { return _robot_joints[rand() % _robot_joints.size()]; }

int rl_sim::load_config_from_json(const std::string& file_name, u::configuration& config, r::api_status* status)
{
  std::string config_str;

  // Load contents of config file into a string
  RETURN_IF_FAIL(load_file(file_name, config_str, status));

  // Use library supplied convenience method to parse json and build config object
  return cfg::create_from_json(config_str, config, nullptr, status);
}

int rl_sim::load_file(const std::string& file_name, std::string& config_str, r::api_status* status)
{
  std::ifstream fs;
  fs.open(file_name);
  if (!fs.good()) { RETURN_ERROR_LS(nullptr, status, invalid_argument) << "Cannot open file: " << file_name; }
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return err::success;
}

void _on_error(const reinforcement_learning::api_status& status, rl_sim* psim) { psim->on_error(status); }

int rl_sim::init_rl()
{
  r::api_status status;
  u::configuration config;
  // Load configuration from json config file
  const auto config_file = _options["json_config"].as<std::string>();
  if (load_config_from_json(config_file, config, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  if (_options["log_to_file"].as<bool>())
  {
    config.set(r::name::INTERACTION_SENDER_IMPLEMENTATION, r::value::INTERACTION_FILE_SENDER);
    config.set(r::name::OBSERVATION_SENDER_IMPLEMENTATION, r::value::OBSERVATION_FILE_SENDER);
  }

  if (!_options["get_model"].as<bool>())
  {
    // Set the time provider to the clock time provider
    config.set(r::name::MODEL_SRC, r::value::NO_MODEL_DATA);
  }

  if (_options["log_timestamp"].as<bool>())
  {
    // Set the time provider to the clock time provider
    config.set(r::name::TIME_PROVIDER_IMPLEMENTATION, r::value::CLOCK_TIME_PROVIDER);
  }

  // Trace log API calls to the console
  config.set(r::name::TRACE_LOG_IMPLEMENTATION, r::value::CONSOLE_TRACE_LOGGER);

  // Initialize the API
  _rl = std::unique_ptr<r::live_model>(new r::live_model(config, _on_error, this));
  if (_rl->init(&status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  std::cout << " API Config " << config;

  return err::success;
}

bool rl_sim::init_sim_world()
{
  //  Initilize topics
  _topics = {"SkiConditions-VT", "HerbGarden", "BeyBlades", "NYCLiving", "MachineLearning"};

  _slot_sizes = {"large", "medium", "small"};

  // Initialize click probability for p1
  person::topic_prob tp = {
      {_topics[0], 0.08f}, {_topics[1], 0.03f}, {_topics[2], 0.05f}, {_topics[3], 0.03f}, {_topics[4], 0.25f}};
  _people.emplace_back("rnc", "engineering", "hiking", "spock", tp);

  // Initialize click probability for p2
  tp = {{_topics[0], 0.08f}, {_topics[1], 0.30f}, {_topics[2], 0.02f}, {_topics[3], 0.02f}, {_topics[4], 0.10f}};
  _people.emplace_back("mk", "psychology", "kids", "7of9", tp);

  return true;
}

bool rl_sim::init_continuous_sim_world()
{
  // initialize continuous actions robot joints

  _friction = {25.4f, 41.2f, 66.5f, 81.9f, 104.4f};

  // temperature (C) range: 20.f to 45.f
  // angular velocity range: 0.f to 200.f
  // load range: -60.f to 60.f

  // first joint j1
  joint::friction_prob fb = {{_friction[0], 0.08f}, {_friction[1], 0.03f}, {_friction[2], 0.05f}, {_friction[3], 0.03f},
      {_friction[4], 0.25f}};

  _robot_joints.emplace_back("j1", 20.3, 102.4, -10.2, fb);

  // second joint j2
  fb = {{_friction[0], 0.08f}, {_friction[1], 0.30f}, {_friction[2], 0.02f}, {_friction[3], 0.02f},
      {_friction[4], 0.10f}};

  _robot_joints.emplace_back("j2", 40.6, 30.8, 98.5, fb);

  return true;
}

bool rl_sim::init()
{
  if (init_rl() != err::success) return false;
  if (!init_sim_world()) return false;
  if (!init_continuous_sim_world()) return false;
  return true;
}

std::string rl_sim::get_action_features()
{
  std::ostringstream oss;
  // example
  // R"("_multi": [ { "TAction":{"topic":"HerbGarden"} }, { "TAction":{"topic":"MachineLearning"} } ])";
  oss << R"("_multi": [ )";
  for (auto idx = 0; idx < _topics.size() - 1; ++idx)
  {
    oss << R"({ "TAction":{"topic":")" << _topics[idx] << R"("} }, )";
  }
  oss << R"({ "TAction":{"topic":")" << _topics.back() << R"("} } ])";
  return oss.str();
}

std::string rl_sim::get_slates_action_features()
{
  std::ostringstream oss;
  // example
  // R"("_multi": [ { "_slot_id": 0, "TAction":{"topic":"HerbGarden"} }, { "_slot_id": 1,
  // "TAction":{"topic":"MachineLearning"} } ])";
  oss << R"("_multi": [ )";
  for (auto idx = 0; idx < _topics.size() - 1; ++idx)
  {
    oss << R"({ "_slot_id":)" << (idx / NUM_SLATES_SLOTS);
    oss << R"(, "TAction":{"topic":")" << _topics[idx] << R"("} }, )";
  }
  oss << R"({ "_slot_id":)" << (NUM_SLATES_SLOTS - 1);
  oss << R"(, "TAction":{"topic":")" << _topics.back() << R"("} } ])";
  return oss.str();
}

std::string rl_sim::get_slot_features()
{
  std::ostringstream oss;
  // example
  // R"("_slots": [ { "_size":"large"}, {"_size":"medium"}, {"_size":"small"} ])";
  oss << R"("_slots": [ )";
  for (auto idx = 0; idx < NUM_SLOTS - 1; ++idx) { oss << R"({ "_size":")" << _slot_sizes[idx] << R"("}, )"; }
  oss << R"({ "_size":")" << _slot_sizes.back() << R"("}] )";
  return oss.str();
}

void rl_sim::on_error(const reinforcement_learning::api_status& status)
{
  std::cout << "Background error in Inference API: " << status.get_error_msg() << std::endl;
  std::cout << "Exiting simulation loop." << std::endl;
  _run_loop = false;
}

std::string rl_sim::create_context_json(const std::string& cntxt)
{
  std::ostringstream oss;
  oss << "{ " << cntxt << " }";
  return oss.str();
}

std::string rl_sim::create_context_json(const std::string& cntxt, const std::string& action)
{
  std::ostringstream oss;
  oss << "{ " << cntxt << ", " << action << " }";
  return oss.str();
}

std::string rl_sim::create_context_json(const std::string& cntxt, const std::string& action, const std::string& slots)
{
  std::ostringstream oss;
  oss << "{ " << cntxt << ", " << action << ", " << slots << " }";
  return oss.str();
}

std::string rl_sim::create_event_id() { return boost::uuids::to_string(boost::uuids::random_generator()()); }

rl_sim::rl_sim(boost::program_options::variables_map vm) : _options(std::move(vm)), _loop_kind(CB)
{
  if (_options["ccb"].as<bool>())
    _loop_kind = CCB;
  else if (_options["slates"].as<bool>())
    _loop_kind = Slates;
  else if (_options["ca"].as<bool>())
    _loop_kind = CA;
  else if (_options["multistep"].as<bool>())
    _loop_kind = Multistep;
}

std::string get_dist_str(const reinforcement_learning::ranking_response& response)
{
  std::string ret;
  ret += "(";
  for (const auto& ap_pair : response)
  {
    ret += "[" + to_string(ap_pair.action_id) + ",";
    ret += to_string(ap_pair.probability) + "]";
    ret += " ,";
  }
  ret += ")";
  return ret;
}

std::string get_dist_str(const reinforcement_learning::slot_response& response)
{
  std::string ret;
  ret += "(";
  ret += "[" + std::string(response.get_slot_id()) + ",";
  ret += to_string(response.get_action_id()) + ",";
  ret += to_string(response.get_probability()) + "]";
  ret += " ,";
  ret += ")";
  return ret;
}

std::string get_dist_str(const reinforcement_learning::slot_entry& response)
{
  std::string ret;
  ret += "(";
  ret += "[" + string(response.get_id()) + ",";
  ret += to_string(response.get_action_id()) + ",";
  ret += to_string(response.get_probability()) + "]";
  ret += " ,";
  ret += ")";
  return ret;
}
std::string get_dist_str(const reinforcement_learning::decision_response& response)
{
  std::string ret;
  ret += "(";
  for (const auto& resp : response)
  {
    ret += get_dist_str(resp);
    ret += " ,";
  }
  ret += ")";
  return ret;
}
