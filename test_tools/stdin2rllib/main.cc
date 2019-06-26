
#include <iostream>
#include <fstream>
#include <locale>
#include "config_utility.h"
#include "live_model.h"
#include "cpprest/json.h"

// Namespace manipulation for brevity
namespace r = reinforcement_learning;
namespace u = r::utility;
namespace cfg = u::config;
namespace err = r::error_code;

using namespace web;
using namespace utility::conversions; // string conversions utilities

//helper to load the config
int load_file(const std::string& file_name, std::string& file_data);
int load_config_from_json(const std::string& file_name, u::configuration& cc);

//helper to parse the json line
void parse_and_send(std::string&, r::live_model&, r::api_status&, bool);
void parse_and_send_outcome(json::value, r::live_model&, r::api_status&, bool);
void parse_and_send_cb_event(json::value, r::live_model&, r::api_status&, bool);
void parse_and_send_ccb_event(json::value, r::live_model&, r::api_status&, bool);

//the cmd line tool expects 2 arguments:
// * argv[1] is the path to rlclient config json file
// * if a 2nd argument is present, it activates the dry run mode (choose_rank and report_outome calls are skipped)
int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "missing arg: path to rlclientlib config" << std::endl;
		return -1;
	}
	std::string config_path = argv[1];

	//deactivate calls that send events (choose_rank and report_outcome)
	bool debug = false;
	if (argc == 3) {
		std::cout << "dry run mode is on" << std::endl;
		debug = true;
	}

	//load conf
	std::cout << "load rllib conf from " << config_path << std::endl;
	u::configuration config;
	if (load_config_from_json(config_path, config) != err::success) {
		std::cout << "unable to load rllib conf " << std::endl;
		return -1;
	}

	//init rllib
	std::cout << "init rllib" << std::endl;
	r::api_status status;
	r::live_model rl(config);
	if (rl.init(&status) != err::success) {
		std::cout << status.get_error_msg() << std::endl;
		return -1;
	}

	//process input events
	std::cout << "process stdin..." << std::endl;
	for (std::string line; std::getline(std::cin, line);) {
		parse_and_send(line, rl, status, debug);
	}
	std::cout << "all events processed" << std::endl;

	return 0;
}

int load_config_from_json(const std::string& file_name, u::configuration& config) {
	std::string config_str;
	// Load contents of config file into a string
	const auto scode = load_file(file_name, config_str);
	if (scode != 0) return scode;

	return cfg::create_from_json(config_str, config);
}

int load_file(const std::string& file_name, std::string& config_str) {
	std::ifstream fs;
	fs.open(file_name);
	if (!fs.good())
		return reinforcement_learning::error_code::invalid_argument;
	std::stringstream buffer;
	buffer << fs.rdbuf();
	config_str = buffer.str();
	return reinforcement_learning::error_code::success;
}

void parse_and_send(std::string& line, r::live_model& rl, r::api_status& status, bool debug) {
	try {
		json::value obj = json::value::parse(to_string_t(line));

		bool is_dangling = obj.has_field(U("RewardValue"));
		bool is_ccb = obj.has_field(U("_outcomes"));

		if (is_dangling) {
			parse_and_send_outcome(obj, rl, status, debug);
		}
		else if (!is_ccb) {
			parse_and_send_cb_event(obj, rl, status, debug);
		}
		else {
			parse_and_send_ccb_event(obj, rl, status, debug);
		}
	}
	catch (const web::json::json_exception& e) {
		std::cout << e.what();
		return;
	}
	catch (const std::exception& e) {
		std::cout << e.what();
		return;
	}
}

void parse_and_send_outcome(json::value obj, r::live_model& rl, r::api_status& status, bool debug) {
	//parse event id
	auto evt_id = U("EventId");
	bool has_event_id = obj.has_field(evt_id);
	if (!has_event_id) {
		std::cout << "missing 'EventId' field" << std::endl;
		return;
	}
	std::string event_id = to_utf8string(obj.at(evt_id).as_string());

	//parse outcome
	float outcome = obj[U("RewardValue")].as_double();
	std::cout << "report_outcome " << event_id << " " << outcome << std::endl;
	if (!debug && rl.report_outcome(event_id.c_str(), outcome, &status) != err::success) {
		std::cout << status.get_error_msg() << std::endl;
		return;
	}
}

void parse_and_send_cb_event(json::value obj, r::live_model& rl, r::api_status& status, bool debug) {
	//parse event id
	auto evt_id = U("EventId");
	bool has_event_id = obj.has_field(evt_id);
	if (!has_event_id) {
		std::cout << "missing 'EventId' field" << std::endl;
		return;
	}
	std::string event_id = to_utf8string(obj.at(evt_id).as_string());

	//parse the joined event
	auto context = U("c");
	bool has_context = obj.has_field(context);
	if (!has_context) {
		std::cout << "missing 'context' field" << std::endl;
		return;
	}
	//extract context string
	json::value v = obj.at(context);
	std::string c = to_utf8string(v.serialize());

	//send ranking
	std::cout << "choose_rank " << event_id << std::endl;
	r::ranking_response response;
	if (!debug && rl.choose_rank(event_id.c_str(), c.c_str(), response, &status) != err::success) {
		std::cout << status.get_error_msg() << std::endl;
		return;
	}

	//send outcome if cost exists and is non-zero
	auto cost = U("_label_cost");
	bool has_cost = obj.has_field(cost);
	if (!has_cost) return;

	float reward = -(obj[cost].as_double());
	if (reward != 0.0) {
		std::cout << "report_outcome " << event_id << " " << reward << std::endl;
		if (!debug && rl.report_outcome(event_id.c_str(), reward, &status) != err::success) {
			std::cout << status.get_error_msg() << std::endl;
			return;
		}
	}
}

void parse_and_send_ccb_event(json::value obj, r::live_model& rl, r::api_status& status, bool debug) {
	//extract context
	auto context = U("c");
	if (!obj.has_field(context)) {
		std::cout << "missing 'context' field" << std::endl;
		return;
	}

	//send decisions
	std::cout << "request_decision " << std::endl;
	//extract context
	json::value v = obj.at(context);
	std::string c = to_utf8string(v.serialize());
	r::decision_response response;
	if (!debug && rl.request_decision(c.c_str(), response, &status) != err::success) {
		std::cout << status.get_error_msg() << std::endl;
		return;
	}

	//iterate on _outcomes, check for positive rewards
	auto outcomes = U("_outcomes");
	if (!obj.has_field(outcomes)) {
		std::cout << "missing '_outcomes' field" << std::endl;
		return;
	}
	auto array = obj.at(outcomes).as_array();

	auto id = U("_id");
	auto cost = U("_label_cost");
	for (int i = 0; i < array.size(); i++) {
		//extract id
		if (!array[i].has_field(id)) {
			std::cout << "missing '_id' field" << std::endl;
			return;
		}
		std::string event_id = to_utf8string(array[i].at(id).as_string());

		//extract reward
		if (!array[i].has_field(cost)) {
			std::cout << "missing '_label_cost' field" << std::endl;
			return;
		}
		float reward = -(array[i].at(cost).as_double());

		if (reward != 0.0) {
			std::cout << "report_outcome " << event_id << " " << reward << std::endl;
			if (!debug && rl.report_outcome(event_id.c_str(), reward, &status) != err::success) {
				std::cout << status.get_error_msg() << std::endl;
				return;
			}
		}
	}
}