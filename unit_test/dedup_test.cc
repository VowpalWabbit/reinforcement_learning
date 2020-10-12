#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "dedup.h"



namespace r = reinforcement_learning;
namespace err = reinforcement_learning::error_code;

BOOST_AUTO_TEST_CASE(dedup_remove_unknown_action) {
    r::dedup_dict dict;

    BOOST_CHECK_EQUAL(false, dict.remove_action(1));
}

BOOST_AUTO_TEST_CASE(dedup_bad_json) {
    r::dedup_dict dict;
    const char *payload = "{ invalid }";
    std::string p_out;
    r::dedup_dict::action_list_t a_out;

    BOOST_CHECK_EQUAL(err::json_parse_error, dict.transform_payload(payload, p_out, a_out, nullptr));
}



