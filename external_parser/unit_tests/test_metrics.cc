#include "vw/io/logger.h"
#include "rapidjson/document.h"
#include "test_common.h"
#include <boost/test/unit_test.hpp>

using namespace rapidjson;

void should_match_expected_metrics(
    const std::string &args, const std::string &infile_name,
    const std::string &outfile_name,
    const std::map<std::string, int> &expected_int_metrics,
    const std::map<std::string, float> &expected_float_metrics,
    const std::map<std::string, std::string> &expected_string_metrics) {

  std::string infile = get_test_files_location() + infile_name;
  std::string outfile =
      get_test_files_location() + "/test_outputs/" + outfile_name;

  remove(outfile.c_str());

  auto vw =
      VW::initialize(args + " -d " + infile +
                         " --binary_parser --quiet --extra_metrics " + outfile,
                     nullptr, false, nullptr, nullptr);

  VW::start_parser(*vw);
  VW::LEARNER::generic_driver(*vw);
  VW::end_parser(*vw);

  VW::finish(*vw);

  namespace rj = rapidjson;

  std::ostringstream metrics_stream;
  std::ifstream metrics_output(outfile);
  metrics_stream << metrics_output.rdbuf();

  try {
    rj::Document d;
    d.Parse(metrics_stream.str().c_str());

    for (auto const &metric : expected_int_metrics) {
      BOOST_CHECK(d.HasMember(metric.first.c_str()));
      BOOST_CHECK_EQUAL(d[metric.first.c_str()].GetInt(), metric.second);
    }

    for (auto const &metric : expected_float_metrics) {
      BOOST_CHECK(d.HasMember(metric.first.c_str()));
      BOOST_CHECK_CLOSE(d[metric.first.c_str()].GetFloat(), metric.second,
                        FLOAT_TOL);
    }

    for (auto const &metric : expected_string_metrics) {
      BOOST_CHECK(d.HasMember(metric.first.c_str()));
      BOOST_CHECK_EQUAL(d[metric.first.c_str()].GetString(), metric.second);
    }

  } catch (const std::exception &e) {
    std::cout << "parse metrics_spec.json failed: " << e.what() << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(metrics_increase_with_events_should_be_tracked) {
  std::string infile_name =
      "valid_joined_logs/average_reward_100_interactions.fb";

  std::string outfile_name =
      "averate_reward_100_interactions_metrics_spec.json";

  std::map<std::string, int> expected_int_metrics = {
      {"total_learn_calls", 100},
      {"total_learn_calls", 100},
      {"sfm_count_learn_example_with_shared", 100},
      {"cbea_labeled_ex", 100},
      {"cbea_predict_in_learn", 0},
      {"cbea_label_first_action", 44},
      {"cbea_label_not_first", 56},
      {"cbea_non_zero_cost", 100},
      {"cbea_min_actions", 2},
      {"cbea_max_actions", 2},
      {"number_skipped_events", 0},
      {"number_events_zero_actions", 0},
      {"line_parse_error", 0}};

  std::map<std::string, float> expected_float_metrics = {
      {"cbea_sum_cost", -270.29998779296877},
      {"cbea_sum_cost_baseline", -119.21666717529297},
      {"dsjson_sum_cost_original", -270.299988},
      {"dsjson_sum_cost_original_baseline", -119.21666717529297}
      // TODO uncomment and fix when some interaction work is pulled in from VW
      //   {"cbea_avg_feat_per_event", 8.0},
      //   {"cbea_avg_actions_per_event", 2.0},
      //   {"cbea_avg_ns_per_event", 6.0},
      //   {"cbea_avg_feat_per_action", 4.0},
      //   {"cbea_avg_ns_per_action", 3.0}
  };

  std::map<std::string, std::string> expected_string_metrics = {
      {"first_event_id", "91f71c8"},
      {"first_event_time", "2021-05-18T13:26:38.000000Z"},
      {"last_event_id", "1357e515"},
      {"last_event_time", "2021-05-18T13:26:39.000000Z"}};

  should_match_expected_metrics("--cb_explore_adf", infile_name, outfile_name,
                                expected_int_metrics, expected_float_metrics,
                                expected_string_metrics);
}

BOOST_AUTO_TEST_CASE(check_metrics_deferred_actions_without_activations) {
  std::string infile_name =
      "skip_learn/cb/deferred_action_without_activation.fb";
  std::string outfile_name =
      "cb_deferred_action_without_activation_metrics_spec.json";

  std::map<std::string, int> expected_int_metrics = {
      {"total_predict_calls", 0},
      {"total_learn_calls", 0},
      {"sfm_count_learn_example_with_shared", 0},
      {"cbea_labeled_ex", 0},
      {"cbea_predict_in_learn", 0},
      {"cbea_label_first_action", 0},
      {"cbea_label_not_first", 0},
      {"cbea_non_zero_cost", 0},
      {"number_skipped_events", 1},
      {"number_events_zero_actions", 0},
      {"line_parse_error", 0}};

  std::map<std::string, float> expected_float_metrics = {
      {"cbea_sum_cost", 0},
      {"cbea_sum_cost_baseline", 0},
      {"dsjson_sum_cost_original", 0},
      {"dsjson_sum_cost_original_baseline", 0}};

  std::map<std::string, std::string> expected_string_metrics = {
      {"first_event_id", ""},
      {"first_event_time", ""},
      {"last_event_id", ""},
      {"last_event_time", ""}};

  should_match_expected_metrics("--cb_explore_adf", infile_name, outfile_name,
                                expected_int_metrics, expected_float_metrics,
                                expected_string_metrics);
}

BOOST_AUTO_TEST_CASE(
    check_metrics_deferred_actions_with_activations_and_apprentice) {
  std::string infile_name =
      "valid_joined_logs/"
      "cb_deferred_actions_w_activations_and_apprentice_10.fb";
  std::string outfile_name =
      "cb_deferred_actions_w_activations_and_apprentice_10_metrics_spec.json";

  std::map<std::string, int> expected_int_metrics = {
      {"total_predict_calls", 7},
      {"total_learn_calls", 7},
      {"sfm_count_learn_example_with_shared", 7},
      {"cbea_labeled_ex", 7},
      {"cbea_predict_in_learn", 0},
      {"cbea_label_first_action", 3},
      {"cbea_label_not_first", 4},
      // 3 instead of expected 7 since it's apprentice mode plus skip learn
      {"cbea_non_zero_cost", 3},
      {"number_skipped_events", 3},
      {"number_events_zero_actions", 0},
      {"line_parse_error", 0},
      {"cbea_min_actions", 2},
      {"cbea_max_actions", 2}};

  std::map<std::string, float> expected_float_metrics = {
      {"cbea_sum_cost", -3.0},
      {"cbea_sum_cost_baseline", -3.0},
      {"dsjson_sum_cost_original", -19.0},
      {"dsjson_sum_cost_original_baseline", -9.0}
      // TODO uncomment and fix when some interaction work is pulled in from VW
      //   {"cbea_avg_feat_per_event", 8.0},
      //   {"cbea_avg_actions_per_event", 2.0},
      //   {"cbea_avg_ns_per_event", 6.0},
      //   {"cbea_avg_feat_per_action", 4.0},
      //   {"cbea_avg_ns_per_action", 3.0}
  };

  std::map<std::string, std::string> expected_string_metrics = {
      {"first_event_id", "e28a9ae6"},
      {"first_event_time", "2021-07-23T13:35:30.000000Z"},
      {"last_event_id", "db81aacf"},
      {"last_event_time", "2021-07-23T13:35:30.000000Z"}};

  should_match_expected_metrics("--cb_explore_adf", infile_name, outfile_name,
                                expected_int_metrics, expected_float_metrics,
                                expected_string_metrics);
}

BOOST_AUTO_TEST_CASE(
    check_metrics_ccb_deferred_actions_with_activations_and_apprentice) {
  std::string infile_name =
      "valid_joined_logs/"
      "ccb_deferred_actions_w_activations_and_apprentice_20.fb";
  std::string outfile_name =
      "ccb_deferred_actions_w_activations_and_apprentice_20_metrics_spec.json";

  std::map<std::string, int> expected_int_metrics = {
      {"total_predict_calls", 0},
      {"total_learn_calls", 17},
      {"sfm_count_learn_example_with_shared", 34},
      {"cbea_labeled_ex", 34},
      {"cbea_predict_in_learn", 0},
      {"cbea_label_first_action", 29},
      {"cbea_label_not_first", 5},
      // 5 instead of expected 20 since it's apprentice mode plus skip learn
      {"cbea_non_zero_cost", 10},
      {"number_skipped_events", 3},
      {"number_events_zero_actions", 0},
      {"line_parse_error", 0},
      {"cbea_min_actions", 1},
      {"cbea_max_actions", 2}};

  std::map<std::string, float> expected_float_metrics = {
      {"cbea_sum_cost", -10.0},
      {"cbea_sum_cost_baseline", -5.0},
      {"dsjson_sum_cost_original", -57.0},
      {"dsjson_sum_cost_original_first_slot", -27.0},
      {"dsjson_number_label_equal_baseline_first_slot", 5},
      {"dsjson_number_label_not_equal_baseline_first_slot", 12},
      {"dsjson_sum_cost_original_label_equal_baseline_first_slot", -11.0}
      // TODO uncomment when some interaction work is pulled in from VW
      //   {"cbea_avg_feat_per_event", 18.0},
      //   {"cbea_avg_actions_per_event", 1.0},
      //   {"cbea_avg_ns_per_event", 7.0},
      //   {"cbea_avg_feat_per_action", 12.0},
      //   {"cbea_avg_ns_per_action", 5.0}
  };

  std::map<std::string, std::string> expected_string_metrics = {
      {"first_event_id", "75d50657"},
      {"first_event_time", "2021-07-27T17:51:23.000000Z"},
      {"last_event_id", "1a997865"},
      {"last_event_time", "2021-07-27T17:51:23.000000Z"}};

  should_match_expected_metrics("--ccb_explore_adf", infile_name, outfile_name,
                                expected_int_metrics, expected_float_metrics,
                                expected_string_metrics);
}

BOOST_AUTO_TEST_CASE(check_metrics_ca_mixed_deferred_action_events) {
  std::string infile_name =
      "valid_joined_logs/ca_mixed_deferred_action_events_20.log";
  std::string outfile_name = "ca_mixed_deferred_action_events.json";

  std::map<std::string, int> expected_int_metrics = {
      {"total_predict_calls", 0},
      {"total_learn_calls", 10},
      {"number_skipped_events", 10},
      {"number_events_zero_actions", 0},
      {"line_parse_error", 0}};

  std::map<std::string, float> expected_float_metrics = {
      {"dsjson_sum_cost_original", -15.0},
  };

  std::map<std::string, std::string> expected_string_metrics = {
      {"first_event_id", "48373f5e"},
      {"first_event_time", "2021-08-10T01:29:55.000000Z"},
      {"last_event_id", "1a997865"},
      {"last_event_time", "2021-08-10T01:29:55.000000Z"}};

  should_match_expected_metrics(
      "--cats 4 --min_value 1 --max_value 100 --bandwidth 1", infile_name,
      outfile_name, expected_int_metrics, expected_float_metrics,
      expected_string_metrics);
}
