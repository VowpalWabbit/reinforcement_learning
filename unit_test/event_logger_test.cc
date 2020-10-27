#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "generic_event.h"
#include "logger/async_batcher.h"
#include "logger/event_logger.h"
#include "constants.h"
#include "serialization/payload_serializer.h"

namespace r = reinforcement_learning;
namespace err = reinforcement_learning::error_code;
namespace v = reinforcement_learning::value;
namespace l = reinforcement_learning::logger;
namespace fb = flatbuffers;

struct my_async_batcher : public l::i_async_batcher<r::generic_event>\
{
    size_t _last_append_size;

    ~my_async_batcher() override {}

    int init(r::api_status* status) override { return err::success; }
    int append(r::generic_event&& evt, r::api_status* status = nullptr) override {
        return err::success;
    }
    int append(r::generic_event& evt, r::api_status* status = nullptr) override {
        _last_append_size = evt.get_payload().size();
        return err::success;
    }
    int run_iteration(r::api_status* status) override {
        return err::success;
    }
};

