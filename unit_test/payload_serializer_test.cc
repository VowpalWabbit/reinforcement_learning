#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "action_flags.h"
#include "ranking_response.h"
#include "serialization/payload_serializer.h"

#include "generated/v2/OutcomeSingle_generated.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/CaEvent_generated.h"
#include "generated/v2/CcbEvent_generated.h"
#include "generated/v2/SlatesEvent_generated.h"

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

BOOST_AUTO_TEST_CASE(ca_payload_serializer_test)
{
  ca_serializer serializer;
  float action = 158.1;
  float pdf_value = 6.09909948e-05;
  continuous_action_response response;
  response.set_chosen_action(action);
  response.set_chosen_action_pdf_value(pdf_value);
  response.set_model_id("model_id");

  const auto buffer = serializer.event("my_context", action_flags::DEFERRED, response);

  const auto event = v2::GetCaEvent(buffer.data());

  std::string context;
  copy(event->context()->begin(), event->context()->end(), std::back_inserter(context));
  
  BOOST_CHECK_EQUAL(context.c_str(), "my_context");
  BOOST_CHECK_EQUAL(event->model_id()->c_str(), "model_id");
  BOOST_CHECK_EQUAL(event->action(), action);
  BOOST_CHECK_EQUAL(event->pdf_value(), pdf_value);
  BOOST_CHECK_EQUAL(event->deferred_action(), true);
}

BOOST_AUTO_TEST_CASE(ccb_payload_serializer_test) {
  ccb_serializer serializer;

  vector<vector<uint32_t>> actions{ { 2, 1, 0 }, { 1, 0 }};
  vector<vector<float>> probs{ { 0.5, 0.3, 0.2 }, { 0.8, 0.2 }};

  const auto buffer = serializer.event("my_context", action_flags::DEFERRED, actions, probs, "model_id");

  const auto event = v2::GetCcbEvent(buffer.data());

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

  const auto event = v2::GetCcbEvent(buffer.data());

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
  outcome_single_serializer serializer;

  const auto buffer = serializer.string_event("my_outcome");

  const auto event = v2::GetOutcomeSingleEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeSingleEventBody_StringEventSingle, event->body_type());
  BOOST_CHECK_EQUAL("my_outcome", event->body_as_StringEventSingle()->value()->c_str());
}

BOOST_AUTO_TEST_CASE(outcome_float_single_payload_serializer_test) {
  outcome_single_serializer serializer;

  const auto buffer = serializer.numeric_event(1.5);

  const auto event = v2::GetOutcomeSingleEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeSingleEventBody_NumericEventSingle, event->body_type());
  BOOST_CHECK_CLOSE(1.5, event->body_as_NumericEventSingle()->value(), tolerance);
}

BOOST_AUTO_TEST_CASE(outcome_string_indexed_payload_serializer_test) {
  outcome_single_serializer serializer;

  const auto buffer = serializer.string_event(2, "my_outcome");

  const auto event = v2::GetOutcomeSingleEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeSingleEventBody_StringEventIndexed, event->body_type());
  BOOST_CHECK_EQUAL("my_outcome", event->body_as_StringEventIndexed()->value()->c_str());
  BOOST_CHECK_EQUAL(2, event->body_as_StringEventIndexed()->index());
}

BOOST_AUTO_TEST_CASE(outcome_float_indexed_payload_serializer_test) {
  outcome_single_serializer serializer;

  const auto buffer = serializer.numeric_event(2, 1.5);

  const auto event = v2::GetOutcomeSingleEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeSingleEventBody_NumericEventIndexed, event->body_type());
  BOOST_CHECK_CLOSE(1.5, event->body_as_NumericEventIndexed()->value(), tolerance);
  BOOST_CHECK_EQUAL(2, event->body_as_NumericEventIndexed()->index());
}

BOOST_AUTO_TEST_CASE(outcome_action_taken_payload_serializer_test) {
  outcome_single_serializer serializer;

  const auto buffer = serializer.report_action_taken();

  const auto event = v2::GetOutcomeSingleEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeSingleEventBody_ActionTakenEvent, event->body_type());
  BOOST_CHECK_EQUAL(true, event->body_as_ActionTakenEvent()->value());
}