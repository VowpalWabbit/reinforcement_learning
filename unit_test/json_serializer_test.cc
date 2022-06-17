#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(json_serializer_ranking_event_single) {}

BOOST_AUTO_TEST_CASE(json_serializer_outcome_event_single_string) {}
BOOST_AUTO_TEST_CASE(json_serializer_outcome_event_single_numeric) {}
BOOST_AUTO_TEST_CASE(json_serializer_outcome_event_single_action_taken) {}

BOOST_AUTO_TEST_CASE(json_serializer_ranking_event_collection) {}
BOOST_AUTO_TEST_CASE(json_serializer_outcome_event_collection) {}

BOOST_AUTO_TEST_CASE(json_serializer_outcome_event_collection_mixed_types) {}