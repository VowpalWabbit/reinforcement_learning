
#include "config_utility.h"
#include "live_model.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>

#include <fstream>
#include <iostream>
#include <locale>

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;
namespace rj = rapidjson;

// helper to load the config
int load_file(const std::string& file_name, std::string& file_data);
int load_config_from_json(const std::string& file_name, u::configuration& cc);

// helper to parse the json line
void parse_and_send(std::string&, r::live_model&, r::api_status&, bool);
void parse_and_send_outcome(const rj::Document&, r::live_model&, r::api_status&, bool);
void parse_and_send_ca_event(const rj::Document&, r::live_model&, r::api_status&, bool);
void parse_and_send_cb_event(const rj::Document&, r::live_model&, r::api_status&, bool);
void parse_and_send_ccb_event(const rj::Document&, r::live_model&, r::api_status&, bool);

// the cmd line tool expects 2 arguments:
//  * argv[1] is the path to rlclient config json file
//  * if a 2nd argument is present, it activates the dry run mode (choose_rank and report_outome calls are skipped)
int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::cout << "missing arg: path to rlclientlib config" << std::endl;
    return -1;
  }
  std::string config_path = argv[1];

  // deactivate calls that send events (choose_rank and report_outcome)
  bool debug = false;
  if (argc == 3)
  {
    std::cout << "dry run mode is on" << std::endl;
    debug = true;
  }

  // load conf
  std::cout << "load rllib conf from " << config_path << std::endl;
  u::configuration config;
  if (load_config_from_json(config_path, config) != err::success)
  {
    std::cout << "unable to load rllib conf " << std::endl;
    return -1;
  }

  // init rllib
  std::cout << "init rllib" << std::endl;
  r::api_status status;
  r::live_model rl(config);
  if (rl.init(&status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return -1;
  }

  // process input events
  std::cout << "process stdin..." << std::endl;
  for (std::string line; std::getline(std::cin, line);) { parse_and_send(line, rl, status, debug); }
  std::cout << "all events processed" << std::endl;

  return 0;
}

int load_config_from_json(const std::string& file_name, u::configuration& config)
{
  std::string config_str;
  // Load contents of config file into a string
  const auto scode = load_file(file_name, config_str);
  if (scode != 0) return scode;

  return cfg::create_from_json(config_str, config);
}

int load_file(const std::string& file_name, std::string& config_str)
{
  std::ifstream fs;
  fs.open(file_name);
  if (!fs.good()) return reinforcement_learning::error_code::invalid_argument;
  std::stringstream buffer;
  buffer << fs.rdbuf();
  config_str = buffer.str();
  return reinforcement_learning::error_code::success;
}

void parse_and_send(std::string& line, r::live_model& rl, r::api_status& status, bool debug)
{
  try
  {
    rj::Document obj;
    obj.Parse(line.c_str());

    if (obj.HasParseError())
    {
      std::cout << "JSON parse error: " << rj::GetParseError_En(obj.GetParseError()) << " (" << obj.GetErrorOffset()
                << ")" << std::endl;
      return;
    }

    bool is_dangling = obj.HasMember("RewardValue");
    bool is_ccb = obj.HasMember("_outcomes");
    bool is_ca = obj.HasMember("_label_ca");

    if (is_dangling) { parse_and_send_outcome(obj, rl, status, debug); }
    else if (is_ca)
    {
      parse_and_send_ca_event(obj, rl, status, debug);
    }
    else if (!is_ccb)
    {
      parse_and_send_cb_event(obj, rl, status, debug);
    }
    else
    {
      parse_and_send_ccb_event(obj, rl, status, debug);
    }
  }
  catch (const std::exception& e)
  {
    std::cout << e.what();
    return;
  }
}

void parse_and_send_outcome(const rj::Document& obj, r::live_model& rl, r::api_status& status, bool debug)
{
  // parse event id
  auto evt_id = "EventId";
  bool has_event_id = obj.HasMember(evt_id);
  if (!has_event_id)
  {
    std::cout << "missing 'EventId' field" << std::endl;
    return;
  }
  std::string event_id = obj[evt_id].GetString();

  // parse outcome
  float outcome = obj["RewardValue"].GetDouble();
  std::cout << "report_outcome " << event_id << " " << outcome << std::endl;
  if (!debug && rl.report_outcome(event_id.c_str(), outcome, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return;
  }
}

void parse_and_send_ca_event(const rj::Document& obj, r::live_model& rl, r::api_status& status, bool debug)
{
  // parse event id
  auto evt_id = "EventId";
  bool has_event_id = obj.HasMember(evt_id);
  if (!has_event_id)
  {
    std::cout << "missing 'EventId' field" << std::endl;
    return;
  }
  auto event_id = obj[evt_id].GetString();

  // parse the joined event
  auto context = "c";
  bool has_context = obj.HasMember(context);
  if (!has_context)
  {
    std::cout << "missing 'context' field" << std::endl;
    return;
  }
  // extract context string
  auto& context_obj = obj[context];
  rj::StringBuffer sb;
  rj::Writer<rj::StringBuffer> writer(sb);
  context_obj.Accept(writer);
  auto c = sb.GetString();

  // request continuous action
  std::cout << "request_continuous_action " << event_id << std::endl;
  r::continuous_action_response response;
  if (!debug && rl.request_continuous_action(event_id, c, response, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return;
  }

  // send outcome if cost exists and is non-zero
  auto label_ca = "_label_ca";
  auto cost = "cost";
  bool has_label = obj.HasMember(label_ca);
  if (!has_label) return;

  float reward = -(obj[label_ca][cost].GetDouble());
  if (reward != 0.0)
  {
    std::cout << "report_outcome " << event_id << " " << reward << std::endl;
    if (!debug && rl.report_outcome(event_id, reward, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return;
    }
  }
}

void parse_and_send_cb_event(const rj::Document& obj, r::live_model& rl, r::api_status& status, bool debug)
{
  // parse event id
  auto evt_id = "EventId";
  bool has_event_id = obj.HasMember(evt_id);
  if (!has_event_id)
  {
    std::cout << "missing 'EventId' field" << std::endl;
    return;
  }
  auto event_id = obj[evt_id].GetString();

  // parse the joined event
  auto context = "c";
  bool has_context = obj.HasMember(context);
  if (!has_context)
  {
    std::cout << "missing 'context' field" << std::endl;
    return;
  }
  // extract context string
  auto& context_obj = obj[context];
  rj::StringBuffer sb;
  rj::Writer<rj::StringBuffer> writer(sb);
  context_obj.Accept(writer);
  auto c = sb.GetString();

  // send ranking
  std::cout << "choose_rank " << event_id << std::endl;
  r::ranking_response response;
  if (!debug && rl.choose_rank(event_id, c, response, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return;
  }

  // send outcome if cost exists and is non-zero
  auto cost = "_label_cost";
  bool has_cost = obj.HasMember(cost);
  if (!has_cost) return;

  float reward = -(obj[cost].GetDouble());
  if (reward != 0.0)
  {
    std::cout << "report_outcome " << event_id << " " << reward << std::endl;
    if (!debug && rl.report_outcome(event_id, reward, &status) != err::success)
    {
      std::cout << status.get_error_msg() << std::endl;
      return;
    }
  }
}

void parse_and_send_ccb_event(const rj::Document& obj, r::live_model& rl, r::api_status& status, bool debug)
{
  // extract context
  auto context = "c";
  if (!obj.HasMember(context))
  {
    std::cout << "missing 'context' field" << std::endl;
    return;
  }

  // send decisions
  std::cout << "request_decision " << std::endl;
  // extract context
  auto& context_obj = obj[context];
  rj::StringBuffer sb;
  rj::Writer<rj::StringBuffer> writer(sb);
  context_obj.Accept(writer);
  auto c = sb.GetString();

  r::decision_response response;
  if (!debug && rl.request_decision(c, response, &status) != err::success)
  {
    std::cout << status.get_error_msg() << std::endl;
    return;
  }

  // iterate on _outcomes, check for positive rewards
  auto outcomes = "_outcomes";
  if (!obj.HasMember(outcomes))
  {
    std::cout << "missing '_outcomes' field" << std::endl;
    return;
  }
  auto array = obj[outcomes].GetArray();

  auto id = "_id";
  auto cost = "_label_cost";
  for (int i = 0; i < array.Size(); i++)
  {
    // extract id
    if (!array[i].HasMember(id))
    {
      std::cout << "missing '_id' field" << std::endl;
      return;
    }
    std::string event_id = array[i][id].GetString();

    // extract reward
    if (!array[i].HasMember(cost))
    {
      std::cout << "missing '_label_cost' field" << std::endl;
      return;
    }
    float reward = -(array[i][cost]).GetDouble();

    if (reward != 0.0)
    {
      std::cout << "report_outcome " << event_id << " " << reward << std::endl;
      if (!debug && rl.report_outcome(event_id.c_str(), reward, &status) != err::success)
      {
        std::cout << status.get_error_msg() << std::endl;
        return;
      }
    }
  }
}