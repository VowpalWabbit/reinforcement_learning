#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "slot_detailed.h"
#include "multi_slot_response_detailed.h"
#include <boost/test/unit_test.hpp>
#include "api_status.h"

using namespace reinforcement_learning;
using namespace std;

using datacol = vector<pair<int, float>>;

datacol get_slot_detailed_test_data1() {
	return {
	  { 2,1.1f },
	  { 6,0.1f },
	  { 1,0.9f },
	  { 4,2.1f },
	  { 3,3.1f }
	};
}

datacol get_slot_detailed_test_data2() {
	return {
	  { 2,1.1f },
	  { 6,0.1f },
	  { 1,0.9f },
	  { 4,2.1f },
	  { 3,3.1f }
	};
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_event_id) {
	multi_slot_response_detailed multi1;
	BOOST_CHECK_EQUAL(multi1.get_event_id(), "");
	multi1.set_event_id("event_id");
	BOOST_CHECK_EQUAL(multi1.get_event_id(), "event_id");
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_model_id) {
	multi_slot_response_detailed multi1;
	BOOST_CHECK_EQUAL(multi1.get_model_id(), "");
	multi1.set_model_id("model_id");
	BOOST_CHECK_EQUAL(multi1.get_model_id(), "model_id");
}

BOOST_AUTO_TEST_CASE(multi_slot_response_detailed_write_read_iterator) {
	multi_slot_response_detailed multi;
	multi.resize(2);
	multi._decision[0] = get_slot_detailed_test_data1();
	multi[1] = get_slot_detailed_test_data2();
	slot_detailed slot1;

}

