#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "serialization/payload_serializer.h"
#include <boost/test/unit_test.hpp>

#include "action_flags.h"
#include "generated/v2/CaEvent_generated.h"
#include "generated/v2/CbEvent_generated.h"
#include "generated/v2/MultiSlotEvent_generated.h"
#include "generated/v2/OutcomeEvent_generated.h"
#include "ranking_response.h"

using namespace reinforcement_learning;
using namespace reinforcement_learning::logger;
using namespace std;
using namespace reinforcement_learning::messages::flatbuff;

namespace r = reinforcement_learning;

const float tolerance = 0.00001f;

BOOST_AUTO_TEST_CASE(cb_payload_serializer_test)
{
  cb_serializer serializer;
  ranking_response rr("event_id");
  rr.set_model_id("model_id");
  rr.push_back(1, 0.2f);
  rr.push_back(0, 0.8f);

  std::vector<uint64_t> action_ids;
  std::vector<float> probs;
  std::string model_id(rr.get_model_id());
  for (auto const& r : rr)
  {
    action_ids.push_back(r.action_id + 1);
    probs.push_back(r.probability);
  }

  const auto buffer = serializer.event(
      "my_context", action_flags::DEFERRED, v2::LearningModeType_Apprentice, action_ids, probs, model_id);

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
  float action = 158.1f;
  float pdf_value = 6.09909948e-05f;
  continuous_action_response response;
  response.set_chosen_action(action);
  response.set_chosen_action_pdf_value(pdf_value);
  response.set_model_id("model_id");

  const auto buffer = serializer.event("my_context", action_flags::DEFERRED, response.get_chosen_action(),
      response.get_chosen_action_pdf_value(), response.get_model_id());

  const auto event = v2::GetCaEvent(buffer.data());

  std::string context;
  copy(event->context()->begin(), event->context()->end(), std::back_inserter(context));

  BOOST_CHECK_EQUAL(context.c_str(), "my_context");
  BOOST_CHECK_EQUAL(event->model_id()->c_str(), "model_id");
  BOOST_CHECK_EQUAL(event->action(), action);
  BOOST_CHECK_EQUAL(event->pdf_value(), pdf_value);
  BOOST_CHECK_EQUAL(event->deferred_action(), true);
}

BOOST_AUTO_TEST_CASE(multi_slot_payload_serializer_test)
{
  multi_slot_serializer serializer;

  vector<vector<uint32_t>> actions{{2, 1, 0}, {1, 0}};
  vector<vector<float>> probs{{0.5f, 0.3f, 0.2f}, {0.8f, 0.2f}};
  vector<std::string> slot_ids = {"0", "1"};
  vector<int> baseline_actions = {1, 0};
  const auto buffer = serializer.event("my_context", action_flags::DEFAULT, actions, probs, "model_id", slot_ids,
      baseline_actions, v2::LearningModeType_Apprentice);

  const auto event = v2::GetMultiSlotEvent(buffer.data());

  std::string context;
  copy(event->context()->begin(), event->context()->end(), std::back_inserter(context));
  BOOST_CHECK_EQUAL("my_context", context.c_str());

  BOOST_CHECK_EQUAL("model_id", event->model_id()->c_str());

  const auto& slots = *event->slots();
  for (flatbuffers::uoffset_t i = 0; i < slots.size(); ++i)
  {
    BOOST_CHECK_EQUAL(slot_ids[i], slots[i]->id()->str());
    BOOST_CHECK_EQUAL(actions[i].size(), slots[i]->action_ids()->size());
    BOOST_CHECK_EQUAL(probs[i].size(), slots[i]->probabilities()->size());
    for (flatbuffers::uoffset_t j = 0; j < actions[i].size(); ++j)
    {
      BOOST_CHECK_EQUAL(actions[i][j], (*slots[i]->action_ids())[j]);
    }
    for (flatbuffers::uoffset_t j = 0; j < probs[i].size(); ++j)
    {
      BOOST_CHECK_CLOSE(probs[i][j], (*slots[i]->probabilities())[j], tolerance);
    }
  }

  const auto& baseline = *event->baseline_actions();
  for (flatbuffers::uoffset_t i = 0; i < baseline_actions.size(); ++i)
  {
    BOOST_CHECK_EQUAL(baseline_actions[i], baseline[i]);
  }

  BOOST_CHECK_EQUAL(false, event->deferred_action());
}

BOOST_AUTO_TEST_CASE(outcome_string_single_payload_serializer_test)
{
  outcome_serializer serializer;

  const auto buffer = serializer.string_event("my_outcome");

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_literal, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_NONE, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_EQUAL("my_outcome", event->value_as_literal()->c_str());
}

BOOST_AUTO_TEST_CASE(outcome_float_single_payload_serializer_test)
{
  outcome_serializer serializer;

  const auto buffer = serializer.numeric_event(1.5);

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_numeric, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_NONE, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_CLOSE(1.5, event->value_as_numeric()->value(), tolerance);
}

BOOST_AUTO_TEST_CASE(outcome_string_indexed_payload_serializer_test)
{
  outcome_serializer serializer;

  const auto buffer = serializer.string_event(2, "my_outcome");

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_literal, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_numeric, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_EQUAL("my_outcome", event->value_as_literal()->c_str());
  BOOST_CHECK_EQUAL(2, event->index_as_numeric()->index());
}

BOOST_AUTO_TEST_CASE(outcome_float_indexed_payload_serializer_test)
{
  outcome_serializer serializer;

  const auto buffer = serializer.numeric_event(2, 1.5);

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_numeric, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_numeric, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_CLOSE(1.5, event->value_as_numeric()->value(), tolerance);
  BOOST_CHECK_EQUAL(2, event->index_as_numeric()->index());
}

BOOST_AUTO_TEST_CASE(outcome_float_string_indexed_payload_serializer_test)
{
  outcome_serializer serializer;

  const auto buffer = serializer.numeric_event("index", 1.5);

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_numeric, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_literal, event->index_type());
  BOOST_CHECK_EQUAL(false, event->action_taken());

  BOOST_CHECK_CLOSE(1.5, event->value_as_numeric()->value(), tolerance);
  BOOST_CHECK_EQUAL("index", event->index_as_literal()->c_str());
}

BOOST_AUTO_TEST_CASE(outcome_action_taken_payload_serializer_test)
{
  outcome_serializer serializer;

  const auto buffer = serializer.report_action_taken();

  const auto event = v2::GetOutcomeEvent(buffer.data());
  BOOST_CHECK_EQUAL(v2::OutcomeValue_NONE, event->value_type());
  BOOST_CHECK_EQUAL(v2::IndexValue_NONE, event->index_type());
  BOOST_CHECK_EQUAL(true, event->action_taken());
}

BOOST_AUTO_TEST_CASE(dedup_info_serialization_test)
{
  dedup_info_serializer serializer;

  std::vector<std::string> object_vals;
  generic_event::object_list_t object_ids;
  std::vector<r::string_view> object_views;

  object_ids.push_back(1020);
  object_vals.push_back("hello");
  object_views.push_back(object_vals[0]);

  const auto buffer = serializer.event(object_ids, object_views);

  const auto event = v2::GetDedupInfo(buffer.data());
  BOOST_CHECK_EQUAL(1, event->ids()->size());
  BOOST_CHECK_EQUAL(1, event->values()->size());

  const auto& objects = *event->ids();
  BOOST_CHECK_EQUAL(1020, objects[0]);
  const auto& values = *event->values();
  BOOST_CHECK_EQUAL("hello", values.GetAsString(0)->c_str());
}
