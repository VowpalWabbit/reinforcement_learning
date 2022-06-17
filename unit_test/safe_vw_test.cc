#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "vw_model/safe_vw.h"

#include "data.h"
#include "model_mgmt.h"
#include "utility/versioned_object_pool.h"

#include <boost/test/unit_test.hpp>

using namespace reinforcement_learning;
using namespace reinforcement_learning::utility;

using pooled_vw = pooled_object_guard<safe_vw, safe_vw_factory>;

void get_model_data_from_raw(const char* data, unsigned int len, model_management::model_data* model_data)
{
  const auto buff = model_data->alloc(len);
  std::memcpy(buff, data, len);
}

BOOST_AUTO_TEST_CASE(safe_vw_1)
{
  safe_vw vw((const char*)cb_data_5_model, cb_data_5_model_len);
  const auto json = R"({"a":{"0":1,"5":2},"_multi":[{"b":{"0":1}},{"b":{"0":2}},{"b":{"0":3}}]})";

  std::vector<int> actions;
  std::vector<float> ranking;
  vw.rank(json, actions, ranking);

  std::vector<float> ranking_expected = {.8f, .1f, .1f};

  BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
}

BOOST_AUTO_TEST_CASE(factory_with_cb_model_and_ccb_arguments)
{
  const auto json =
      R"({ "GUser":{"id":"rnc", "major" : "engineering", "hobby" : "hiking", "favorite_character" : "spock"}, "_multi" : [{ "TAction":{"topic":"SkiConditions-VT"} }, { "TAction":{"topic":"HerbGarden"} }, { "TAction":{"topic":"BeyBlades"} }, { "TAction":{"topic":"NYCLiving"} }, { "TAction":{"topic":"MachineLearning"} }], "_slots" : [{ "_size":"large"}, { "_size":"medium" }, { "_size":"small" }]  })";
  const auto vw_commandLine = "--ccb_explore_adf --epsilon 0.2 --power_t 0 -l 0.001 --cb_type ips -q ::";

  // Start with loading cb model with ccb arguments
  model_management::model_data model_data;
  get_model_data_from_raw((const char*)cb_data_5_model, cb_data_5_model_len, &model_data);

  const auto factory = new safe_vw_factory(model_data, vw_commandLine);
  versioned_object_pool<safe_vw, safe_vw_factory> pool(factory);

  {
    pooled_vw guard(pool, pool.get_or_create());

    const char* event_id = "abcdef";
    uint32_t slot_count = 3;
    const char* features = json;

    std::vector<std::vector<uint32_t>> actions_ids;
    std::vector<std::vector<float>> action_pdfs;

    std::vector<std::string> slot_ids = {"0", "1"};

    guard->rank_multi_slot_decisions(event_id, slot_ids, features, actions_ids, action_pdfs);
    BOOST_CHECK_EQUAL(actions_ids.size(), slot_count);
    BOOST_CHECK_EQUAL(action_pdfs.size(), slot_count);
    // todo: add more accurate assertions once vw is updated with cb and ccb single slot equivalence changes
  }
}

BOOST_AUTO_TEST_CASE(factory_with_initial_model)
{
  const auto json = R"({"a":{"0":1,"5":2},"_multi":[{"b":{"0":1}},{"b":{"0":2}},{"b":{"0":3}}]})";
  std::vector<float> ranking_expected = {.8f, .1f, .1f};

  // Start with an initial model
  model_management::model_data model_data;
  get_model_data_from_raw((const char*)cb_data_5_model, cb_data_5_model_len, &model_data);

  const auto factory = new safe_vw_factory(model_data);
  versioned_object_pool<safe_vw, safe_vw_factory> pool(factory);

  {
    pooled_vw guard(pool, pool.get_or_create());

    // Update factory while an object is floating around
    model_management::model_data updated_model;
    get_model_data_from_raw((const char*)cb_data_5_model, cb_data_5_model_len, &updated_model);
    pool.update_factory(new safe_vw_factory(updated_model));

    std::vector<int> actions;
    std::vector<float> ranking;
    guard->rank(json, actions, ranking);

    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }

  {
    // Make sure we get a new object
    pooled_object_guard<safe_vw, safe_vw_factory> guard(pool, pool.get_or_create());

    std::vector<int> actions;
    std::vector<float> ranking;
    guard->rank(json, actions, ranking);

    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }
}

BOOST_AUTO_TEST_CASE(factory_with_empty_model)
{
  const auto json = R"({"a":{"0":1,"5":2},"_multi":[{"b":{"0":1}},{"b":{"0":2}},{"b":{"0":3}}]})";
  std::vector<float> ranking_expected = {.8f, .1f, .1f};

  // Start with empty model data
  model_management::model_data empty_data;
  const auto factory = new safe_vw_factory(empty_data);
  versioned_object_pool<safe_vw, safe_vw_factory> pool(factory);

  // Initial model & rank call
  {
    model_management::model_data new_model;
    get_model_data_from_raw((const char*)cb_data_5_model, cb_data_5_model_len, &new_model);
    pool.update_factory(new safe_vw_factory(new_model));
    pooled_vw vw(pool, pool.get_or_create());

    std::vector<int> actions;
    std::vector<float> ranking;
    vw->rank(json, actions, ranking);

    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }

  // Update model & rank call
  {
    model_management::model_data new_model;
    get_model_data_from_raw((const char*)cb_data_5_model, cb_data_5_model_len, &new_model);
    pool.update_factory(new safe_vw_factory(new_model));
    pooled_vw vw(pool, pool.get_or_create());

    std::vector<int> actions;
    std::vector<float> ranking;
    vw->rank(json, actions, ranking);

    BOOST_CHECK_EQUAL_COLLECTIONS(ranking.begin(), ranking.end(), ranking_expected.begin(), ranking_expected.end());
  }
}
