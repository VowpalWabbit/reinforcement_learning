#include "test_common.h"
#include <boost/test/unit_test.hpp>
#include "rapidjson/document.h"
#include "io/logger.h"

using namespace rapidjson;

void should_match_expected_metrics(std::string infile_name,
  std::map<std::string, int> expected_metrics) {
  std::string infile = get_test_files_location() + infile_name;
  std::string outfile = get_test_files_location() + "metrics_spec.json";

  auto vw = VW::initialize("--cb_explore_adf -d " + infile +
                           " --binary_parser --quiet --extra_metrics " + outfile,
                           nullptr, false, nullptr, nullptr);

  v_array<example *> examples = v_init<example*>();
  examples.push_back(&VW::get_unused_example(vw));

  while (vw->example_parser->reader(vw, examples) > 0) {
    continue;
  }

  clear_examples(examples, vw);
  VW::finish(*vw);

  namespace rj = rapidjson;

  std::ostringstream metrics_stream;
  std::ifstream metrics_output(outfile);
  metrics_stream << metrics_output.rdbuf();

  try {
    rj::Document d;
    d.Parse(metrics_stream.str().c_str());

    for (auto const &metric : expected_metrics) {
      BOOST_CHECK(d.HasMember(metric.first.c_str()));
      BOOST_CHECK_EQUAL(d[metric.first.c_str()].GetInt(), metric.second);
    }
  }
  catch (const std::exception& e) {
    VW::io::logger::log_error(
      "parse metrics_spec.json failed: [{}].",
      e.what()
    );
  }
  remove(outfile.c_str());
}

BOOST_AUTO_TEST_CASE(metrics_increase_with_events_should_be_tracked) {
  std::string infile_name = "valid_joined_logs/average_reward_100_interactions.fb";
  std::map<std::string, int> expected_metrics = {
    {"number_of_learned_events", 100},
    {"number_of_skipped_events", 0}
  };
  should_match_expected_metrics(infile_name, expected_metrics);
}

BOOST_AUTO_TEST_CASE(metrics_not_increase_with_events_should_not_be_tracked) {
  std::string infile_name = "skip_learn/cb_deferred_action_without_activation.fb";
  std::map<std::string, int> expected_metrics = {
    {"number_of_learned_events", 0},
    {"number_of_skipped_events", 1}
  };
  should_match_expected_metrics(infile_name, expected_metrics);
}

