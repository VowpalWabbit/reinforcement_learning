#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "multi_slot_response_detailed.h"
#include <boost/test/unit_test.hpp>

#include "api_status.h"
#include "slot_ranking.h"

#include <string>

using namespace reinforcement_learning;
using namespace std;

using datacol = vector<pair<int, float>>;

datacol get_slot_ranking_test_data1() { return {{2, 1.1f}, {6, 0.1f}, {1, 0.9f}, {4, 2.1f}, {3, 3.1f}}; }

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_event_id)
{
  multi_slot_response_detailed multi1;
  BOOST_CHECK_EQUAL(multi1.get_event_id(), "");
  multi1.set_event_id("event_id");
  BOOST_CHECK_EQUAL(multi1.get_event_id(), "event_id");
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_model_id)
{
  multi_slot_response_detailed multi1;
  BOOST_CHECK_EQUAL(multi1.get_model_id(), "");
  multi1.set_model_id("model_id");
  BOOST_CHECK_EQUAL(multi1.get_model_id(), "model_id");
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_write_read_iterator)
{
  multi_slot_response_detailed multi;
  multi.resize(2);

  slot_ranking slot1;
  slot_ranking slot2;
  auto test_data = get_slot_ranking_test_data1();
  for (auto& p : test_data)
  {
    slot1.push_back(p.first, p.second);
    slot2.push_back(p.first, p.second);
  }

  multi.set_slot_at_index(0, std::move(slot1));
  multi.set_slot_at_index(0, std::move(slot2));

  int i = 0;
  for (const auto& s : multi)
  {
    int j = 0;
    for (const auto& d : s)
    {
      auto& td = test_data[j++];
      BOOST_CHECK_EQUAL(d.action_id, td.first);
      BOOST_CHECK_EQUAL(d.probability, td.second);
    }
  }

  i = 0;
  for (auto r = begin(multi); r != end(multi); ++r)
  {
    int j = 0;
    for (const auto& d : (*r))
    {
      auto& td = test_data[j++];
      BOOST_CHECK_EQUAL(d.action_id, td.first);
      BOOST_CHECK_EQUAL(d.probability, td.second);
    }
  }
}
