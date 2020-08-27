#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include "action_flags.h"
#include "ranking_response.h"
#include "serialization/payload_serializer.h"

#include "generated/v2/OutcomeSingle_generated.h"
#include "generated/v2/CbEvent_generated.h"
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
}
