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

struct my_transformer: public l::i_payload_transformer {
    ~my_transformer() override = default;
    int transform(r::generic_event::payload_buffer_t &input, r::api_status* status) override
    {
        uint8_t* data = (uint8_t*)malloc(10);
        input = fb::DetachedBuffer(nullptr, false, data, 0, data, 10);
        return err::success;
    }
};

BOOST_AUTO_TEST_CASE(generic_event_logger_with_transformer) { 
    auto batcher = new my_async_batcher();
    l::generic_event_logger logger(nullptr, batcher, new my_transformer());
    BOOST_CHECK_EQUAL(err::success, logger.init(nullptr));

    l::outcome_serializer ser;
    
    auto payload = ser.numeric_event(1);
    BOOST_CHECK_NE(10, payload.size());
    BOOST_CHECK_EQUAL(err::success, logger.log("evt", std::move(payload), r::generic_event::payload_type_t::PayloadType_Outcome, nullptr));
    BOOST_CHECK_EQUAL(10, batcher->_last_append_size);
}
