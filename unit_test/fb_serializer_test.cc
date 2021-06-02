#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "learning_mode.h"
#include "ranking_event.h"
#include "api_status.h"
#include "constants.h"
#include "serialization/fb_serializer.h"
#include "serialization/payload_serializer.h"

using namespace reinforcement_learning;
using namespace logger;
using namespace utility;
using namespace reinforcement_learning::logger;
using namespace std;
using namespace reinforcement_learning::messages::flatbuff;

BOOST_AUTO_TEST_CASE(fb_serializer_outcome_event) {
  data_buffer db;
  fb_collection_serializer<outcome_event> serializer(db, value::CONTENT_ENCODING_IDENTITY, "app_id");
  std::string event_id("an_event_id");
  const timestamp ts;
  auto ro = outcome_event::report_outcome(event_id.c_str(), 0.75f, ts, 0.54f);
  serializer.add(ro);
  std::string outcome_str("{stuff}");
  ro = outcome_event::report_outcome(event_id.c_str(), outcome_str.c_str(), ts, 0.54f);
  serializer.add(ro);
  ro = outcome_event::report_action_taken(event_id.c_str(), ts, 0.54f);
  serializer.add(ro);
  BOOST_CHECK_EQUAL(reinforcement_learning::error_code::success, serializer.finalize(nullptr));

  flatbuffers::Verifier v(db.body_begin(), db.body_filled_size());
  const OutcomeEventBatch* outcome_event_batch = GetOutcomeEventBatch(db.body_begin());
  BOOST_CHECK(outcome_event_batch->Verify(v));
  const auto& events = (*outcome_event_batch->events());
  BOOST_CHECK_EQUAL(events.size(), 3);

  auto event = events[0];
  BOOST_CHECK_EQUAL(event->the_event_type(), OutcomeEvent_NumericEvent);
  BOOST_CHECK_EQUAL(event->event_id()->str(), event_id);
  BOOST_CHECK_EQUAL(event->pass_probability(), 0.54f);
  const auto ne = event->the_event_as_NumericEvent();
  BOOST_CHECK_EQUAL(ne->value(), 0.75);

  event = events[1];
  BOOST_CHECK_EQUAL(event->the_event_type(), OutcomeEvent_StringEvent);
  BOOST_CHECK_EQUAL(event->event_id()->str(), event_id);
  BOOST_CHECK_EQUAL(event->pass_probability(), 0.54f);
  const auto se = event->the_event_as_StringEvent();
  BOOST_CHECK_EQUAL(se->value()->str(), outcome_str);

  event = events[2];
  BOOST_CHECK_EQUAL(event->the_event_type(), OutcomeEvent_ActionTakenEvent);
  BOOST_CHECK_EQUAL(event->event_id()->str(), event_id);
  BOOST_CHECK_EQUAL(event->pass_probability(), 0.54f);
  const auto ae = event->the_event_as_ActionTakenEvent();
  BOOST_CHECK_EQUAL(ae->value(), true);
}

BOOST_AUTO_TEST_CASE(fb_serializer_ranking_event) {
  data_buffer db;
  fb_collection_serializer<ranking_event> serializer(db, value::CONTENT_ENCODING_IDENTITY, "app_id");
  ranking_response resp;
  std::string model_id("a_model_id");
  resp.set_model_id(model_id.c_str());
  resp.push_back(2, .8f + .2f / 3);
  resp.push_back(0, .2f / 3);
  resp.push_back(1, .2f / 3);
  std::string event_id("an_event_id");
  std::string context("some_context");
  learning_mode mode = ONLINE;

  const timestamp ts;
  const size_t events_count = 10;
  const int learning_mode_count = 3;

  for (size_t i = 0; i < events_count; ++i) {
    mode = static_cast<learning_mode>(i % learning_mode_count);
    auto re = ranking_event::choose_rank(event_id.c_str(), context.c_str(), 0, resp, ts, 0.33f, mode);
    serializer.add(re);
  }
  BOOST_CHECK_EQUAL(reinforcement_learning::error_code::success, serializer.finalize(nullptr));

  flatbuffers::Verifier v(db.body_begin(), db.body_filled_size());
  const RankingEventBatch* ranking_event_batch = GetRankingEventBatch(db.body_begin());
  BOOST_CHECK(ranking_event_batch->Verify(v));
  const auto& events = *(ranking_event_batch->events());
  BOOST_CHECK_EQUAL(events.size(), events_count);
  for (size_t i = 0; i < events_count; ++i) {
    const auto& event = *events[(flatbuffers::uoffset_t)i];
    BOOST_CHECK_EQUAL(event.action_ids()->size(), 3);
    std::vector<int> expected_ids{ 3,1,2 };
    BOOST_CHECK_EQUAL_COLLECTIONS(event.action_ids()->begin(), event.action_ids()->end(), expected_ids.begin(), expected_ids.end());
    BOOST_CHECK_EQUAL_COLLECTIONS(event.context()->begin(), event.context()->end(), context.begin(), context.end());
    BOOST_CHECK_EQUAL(event.model_id()->str(), model_id);
    BOOST_CHECK_EQUAL(event.event_id()->str(), event_id);
    std::vector<float> expected_prob{ .8f + .2f / 3, .2f / 3, .2f / 3 };
    BOOST_CHECK_EQUAL_COLLECTIONS(event.probabilities()->begin(), event.probabilities()->end(), expected_prob.begin(), expected_prob.end());
    BOOST_CHECK_EQUAL(event.deferred_action(), false);
    BOOST_CHECK_EQUAL(event.pass_probability(), 0.33f);
    if (i % learning_mode_count == 0) {
      BOOST_CHECK_EQUAL(event.learning_mode(), LearningModeType_Online);
    }
    else if (i % learning_mode_count == 1) {
      BOOST_CHECK_EQUAL(event.learning_mode(), LearningModeType_Apprentice);
    }
    else {
      BOOST_CHECK_EQUAL(event.learning_mode(), LearningModeType_LoggingOnly);
    }
  }
}

BOOST_AUTO_TEST_CASE(fb_serializer_generic_event_content_encoding) {
  data_buffer db;
  fb_collection_serializer<generic_event> collection_serializer(db, value::CONTENT_ENCODING_DEDUP, "app_id");
  const char* event_id("event_id");
  const timestamp ts;
  cb_serializer serializer;
  ranking_response rr(event_id);
  rr.set_model_id("model_id");
  rr.push_back(1, 0.2);
  rr.push_back(0, 0.8);

  auto buffer = serializer.event("my_context", action_flags::DEFERRED, v2::LearningModeType_Apprentice, rr);

  generic_event ge(event_id, ts, v2::PayloadType_CB, std::move(buffer), event_content_type::IDENTITY);
  collection_serializer.add(ge);
  BOOST_CHECK_EQUAL(reinforcement_learning::error_code::success, collection_serializer.finalize(nullptr));

  flatbuffers::Verifier v(db.body_begin(), db.body_filled_size());
  const v2::EventBatch *event_batch = v2::GetEventBatch(db.body_begin());
  BOOST_CHECK(event_batch->Verify(v));
  const auto& batch_metadata = *(event_batch->metadata());
  BOOST_CHECK_EQUAL(batch_metadata.content_encoding()->c_str(), value::CONTENT_ENCODING_DEDUP);
}

BOOST_AUTO_TEST_CASE(fb_serializer_generic_event_metadata) {
  data_buffer db;
  fb_collection_serializer<generic_event> collection_serializer(db, value::CONTENT_ENCODING_DEDUP, "app_id");
  const char* event_id("event_id");
  const timestamp ts;
  cb_serializer serializer;
  ranking_response rr(event_id);
  rr.set_model_id("model_id");
  rr.push_back(1, 0.2);
  rr.push_back(0, 0.8);

  auto buffer = serializer.event("my_context", action_flags::DEFERRED, v2::LearningModeType_Apprentice, rr);

  generic_event ge(event_id, ts, v2::PayloadType_CB, std::move(buffer), event_content_type::IDENTITY);
  collection_serializer.add(ge);
  BOOST_CHECK_EQUAL(reinforcement_learning::error_code::success, collection_serializer.finalize(nullptr));

  flatbuffers::Verifier v(db.body_begin(), db.body_filled_size());
  const v2::EventBatch *event_batch = v2::GetEventBatch(db.body_begin());
  BOOST_CHECK(event_batch->Verify(v));

  const auto& events = *(event_batch->events());

  for (size_t i = 0; i < events.size(); ++i) {
    const auto& serialized_event = *events[(flatbuffers::uoffset_t)i];
    const auto& payload = serialized_event.payload();
    const v2::Event *event = flatbuffers::GetRoot<v2::Event>(payload);
    //const auto& metadata = *(event->meta());
	//BOOST_CHECK_EQUAL(metadata.app_id()->c_str(), "app_id");
  }
}