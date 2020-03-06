#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#   define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>
#include "data_buffer.h"
#include "logger/preamble_sender.h"
#include "logger/message_type.h"
#include "err_constants.h"
#include "logger/preamble.h"

using namespace reinforcement_learning::utility;
using namespace reinforcement_learning::logger;
using namespace reinforcement_learning;

struct dummy_sender : i_sender {
  int init(api_status* status) override {
    return error_code::success;
  };

  int v_send(const buffer& data, api_status* status = nullptr) override {
    v_data = data;
    return error_code::success;
  }

  buffer v_data;
};

BOOST_AUTO_TEST_CASE(simple_preamble_usage) {
  std::shared_ptr<data_buffer> db(new data_buffer());
  dummy_sender* raw_data = new dummy_sender();
  preamble_message_sender f_sender(raw_data);
  i_message_sender& sender = f_sender;
  db->set_body_endoffset(db->preamble_size()+db->body_capacity());
  const auto send_msg_sz = db->body_filled_size();
  const auto send_msg_type = message_type::fb_ranking_learning_mode_event_collection;

  sender.send(send_msg_type, db);
  preamble pre;
  pre.read_from_bytes(raw_data->v_data->preamble_begin(), raw_data->v_data->preamble_size());

  BOOST_CHECK_EQUAL(pre.msg_size, send_msg_sz);
  BOOST_CHECK_EQUAL(pre.msg_type, send_msg_type);
}
