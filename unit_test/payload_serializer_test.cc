#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "action_flags.h"
#include "ranking_response.h"
#include "serialization/payload_serializer.h"

#include "generated/v2/OutcomeEvent_generated.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/MultiSlotEvent_generated.h"

#include <boost/test/unit_test.hpp>

using namespace reinforcement_learning;
using namespace reinforcement_learning::logger;
using namespace std;
using namespace reinforcement_learning::messages::flatbuff;

const float tolerance = 0.00001;

BOOST_AUTO_TEST_CASE(cb_payload_serializer_test) {
  cb_serializer serializer;
  ranking_response rr("event_id");
  rr.set_model_id("model_id");
  rr.push_back(1, 0.2);
  rr.push_back(0, 0.8);

  const auto buffer = serializer.event("my_context", action_flags::DEFERRED, v2::LearningModeType_Apprentice, rr);

  const auto event = v2::GetCbEvent(buffer.data());

  std::string context;
  copy(event->context()->begin(), event->context()->end(), std::back_inserter(context));
  BOOST_CHECK_EQUAL("my_context", context.c_str());

  BOOST_CHECK_EQUAL("model_id", event->model_id()->c_str());

  const auto& actions = *event->action_ids();
  BOOST_CHECK_EQUAL(2, actions[0]);
  BOOST_CHECK_EQUAL(1, actions[1]);

  const auto& probabilities = *event->probabilities();
  BOOST_CHECK_CLOSE(0.2, probabilities[0], tolerance);
  BOOST_CHECK_CLOSE(0.8, probabilities[1], tolerance);

  BOOST_CHECK_EQUAL(true, event->deferred_action());
}

BOOST_AUTO_TEST_CASE(ccb_payload_serializer_test) {
  ccb_serializer serializer;

  vector<vector<uint32_t>> actions{ { 2, 1, 0 }, { 1, 0 }};
  vector<vector<float>> probs{ { 0.5, 0.3, 0.2 }, { 0.8, 0.2 }};

  const auto buffer = serializer.event("my_context", action_flags::DEFERRED, actions, probs, "model_id");

  const auto event = v2::GetMultiSlotEvent(buffer.data());

  std::string context;
  copy(event->context()->begin(), event->context()->end(), std::back_inserter(context));
  BOOST_CHECK_EQUAL("my_context", context.c_str());

  BOOST_CHECK_EQUAL("model_id", event->model_id()->c_str());

  const auto& slots = *event->slots();
  for (size_t i = 0; i < slots.size(); ++i) {
    BOOST_CHECK_EQUAL(actions[i].size(), slots[i]->action_ids()->size());
    BOOST_CHECK_EQUAL(probs[i].size(), slots[i]->probabilities()->size());
    for (size_t j = 0; j < actions[i].size(); ++j) {
      BOOST_CHECK_EQUAL(actions[i][j], (*slots[i]->action_ids())[j]);
    }
    for (size_t j = 0; j < probs[i].size(); ++j) {
      BOOST_CHECK_CLOSE(probs[i][j], (*slots[i]->probabilities())[j], tolerance);
    }
  }

  BOOST_CHECK_EQUAL(true, event->deferred_action());
}

BOOST_AUTO_TEST_CASE(slates_payload_serializer_test){
  slates_serializer serializer;

  vector<vector<uint32_t>> actions{ { 2, 1, 0 }, { 1, 0 }};
  vector<vector<float>> probs{ { 0.5, 0.3, 0.2 }, { 0.8, 0.2 }};
  const auto buffer = serializer.event("my_context", action_flags::DEFAULT, actions, probs, "model_id");

  const auto event = v2::GetMultiSlotEvent(buffer.data());

  std::string context;
  copy(event->context()->begin(), event->context()->end(), std::back_inserter(context));
  BOOST_CHECK_EQUAL("my_context", context.c_str());

  BOOST_CHECK_EQUAL("model_id", event->model_id()->c_str());

  const auto& slots = *event->slots();
  for (size_t i = 0; i < slots.size(); ++i) {
    BOOST_CHECK_EQUAL(actions[i].size(), slots[i]->action_ids()->size());
    BOOST_CHECK_EQUAL(probs[i].size(), slots[i]->probabilities()->size());
    for (size_t j = 0; j < actions[i].size(); ++j) {
        BOOST_CHECK_EQUAL(actions[i][j], (*slots[i]->action_ids())[j]);
    }
    for (size_t j = 0; j < probs[i].size(); ++j) {
        BOOST_CHECK_CLOSE(probs[i][j], (*slots[i]->probabilities())[j], tolerance);
    }
  }

  BOOST_CHECK_EQUAL(false, event->deferred_action());
}

BOOST_AUTO_TEST_CASE(outcome_string_single_payload_serializer_test) {
  outcome_serializer serializer;

  const auto buffer = serializer.string_event("my_outcome");

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_literal, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_NONE, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_EQUAL("my_outcome", event->value_as_literal()->c_str());
}

BOOST_AUTO_TEST_CASE(outcome_float_single_payload_serializer_test) {
  outcome_serializer serializer;

  const auto buffer = serializer.numeric_event(1.5);

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_numeric, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_NONE, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_CLOSE(1.5, event->value_as_numeric()->value(), tolerance);
}

BOOST_AUTO_TEST_CASE(outcome_string_indexed_payload_serializer_test) {
  outcome_serializer serializer;

  const auto buffer = serializer.string_event(2, "my_outcome");

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_literal, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_numeric, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_EQUAL("my_outcome", event->value_as_literal()->c_str());
  BOOST_CHECK_EQUAL(2, event->index_as_numeric()->index());
}

BOOST_AUTO_TEST_CASE(outcome_float_indexed_payload_serializer_test) {
  outcome_serializer serializer;

  const auto buffer = serializer.numeric_event(2, 1.5);

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_numeric, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_numeric, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_CLOSE(1.5, event->value_as_numeric()->value(), tolerance);
  BOOST_CHECK_EQUAL(2, event->index_as_numeric()->index());
}


BOOST_AUTO_TEST_CASE(outcome_float_string_indexed_payload_serializer_test) {
  outcome_serializer serializer;

  const auto buffer = serializer.numeric_event("index", 1.5);

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_numeric, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_literal, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_CLOSE(1.5, event->value_as_numeric()->value(), tolerance);
  BOOST_CHECK_EQUAL("index", event->index_as_literal()->c_str());
}

BOOST_AUTO_TEST_CASE(outcome_action_taken_payload_serializer_test) {
  outcome_serializer serializer;

  const auto buffer = serializer.report_action_taken();

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_NONE, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_NONE, event->index_type());
  BOOST_CHECK_EQUAL(true, event->action_taken());
}