#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "slot_ranking.h"
#include <boost/test/unit_test.hpp>

#include "api_status.h"

using namespace reinforcement_learning;
using namespace std;

using datacol = vector<pair<int, float>>;

datacol get_slot_ranking_test_data() { return {{2, 1.1f}, {6, 0.1f}, {1, 0.9f}, {4, 2.1f}, {3, 3.1f}}; }

BOOST_AUTO_TEST_CASE(slot_ranking_id)
{
  slot_ranking slot1;
  BOOST_CHECK_EQUAL(slot1.get_id(), "");
  slot_ranking slot2("id");
  BOOST_CHECK_EQUAL(slot2.get_id(), "id");
}

BOOST_AUTO_TEST_CASE(slot_ranking_empty_collection)
{
  slot_ranking slot;
  api_status s;
  size_t action_id;
  auto scode = slot.get_chosen_action_id(action_id, &s);
  BOOST_CHECK_GT(scode, 0);
  BOOST_CHECK_GT(s.get_error_code(), 0);

  action_id = 10;
  scode = slot.get_chosen_action_id(action_id);
  BOOST_CHECK_GT(scode, 0);
}

BOOST_AUTO_TEST_CASE(slot_ranking_write_read_iterator)
{
  auto test_data = get_slot_ranking_test_data();

  slot_ranking slot;

  for (auto& p : test_data) { slot.push_back(p.first, p.second); }

  int i = 0;
  for (const auto& d : slot)
  {
    auto& td = test_data[i++];
    BOOST_CHECK_EQUAL(d.action_id, td.first);
    BOOST_CHECK_EQUAL(d.probability, td.second);
  }

  i = 0;
  for (auto r = begin(slot); r != end(slot); ++r)
  {
    auto& td = test_data[i++];
    BOOST_CHECK_EQUAL((*r).action_id, td.first);
    BOOST_CHECK_EQUAL((*r).probability, td.second);
  }
}
