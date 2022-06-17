#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include "data_buffer.h"
#include <boost/test/unit_test.hpp>

#include "err_constants.h"
#include "utility/data_buffer_streambuf.h"

using namespace reinforcement_learning;
using namespace utility;
using namespace std;

BOOST_AUTO_TEST_CASE(new_data_buffer_is_empty)
{
  data_buffer buffer(1);
  BOOST_CHECK_EQUAL(buffer.body_capacity(), 1);
  BOOST_CHECK_EQUAL(buffer.body_filled_size(), 0);
}

BOOST_AUTO_TEST_CASE(single_output_to_data_buffer)
{
  data_buffer buffer;
  data_buffer_streambuf sbuff(&buffer);
  ostream out(&sbuff);
  out << "test";
  sbuff.finalize();
  const string body(reinterpret_cast<char*>(buffer.body_begin()));
  BOOST_CHECK_EQUAL(body, "test");
}

BOOST_AUTO_TEST_CASE(multiple_outputs_to_data_buffer)
{
  data_buffer buffer;
  data_buffer_streambuf sbuff(&buffer);
  ostream out(&sbuff);
  const string value_string = "test";
  const size_t value_size_t = 2;
  out << value_string << value_size_t << value_string.c_str();
  sbuff.finalize();
  const string body(reinterpret_cast<char*>(buffer.body_begin()));
  BOOST_CHECK_EQUAL(body, "test2test");
}

BOOST_AUTO_TEST_CASE(empty_data_buffer_reset)
{
  data_buffer buffer;
  data_buffer_streambuf sbuff(&buffer);
  ostream out(&sbuff);
  sbuff.finalize();
  buffer.reset();
  BOOST_CHECK_EQUAL(buffer.body_filled_size(), 0);
}

BOOST_AUTO_TEST_CASE(nonempty_data_buffer_reset)
{
  data_buffer buffer;
  data_buffer_streambuf sbuff(&buffer);
  ostream out(&sbuff);
  out << "test";
  sbuff.finalize();
  buffer.reset();
  BOOST_CHECK_EQUAL(buffer.body_filled_size(), 0);
}

BOOST_AUTO_TEST_CASE(data_buffer_reset_rewrite)
{
  data_buffer buffer;
  data_buffer_streambuf sbuff(&buffer);
  ostream out(&sbuff);
  out << "test";
  sbuff.finalize();
  string body(reinterpret_cast<char*>(buffer.body_begin()));
  BOOST_CHECK_EQUAL(body, "test");
  BOOST_CHECK_EQUAL(buffer.body_filled_size(), 4);
  BOOST_CHECK_EQUAL(buffer.buffer_filled_size(), 12);

  buffer.reset();
  BOOST_CHECK_EQUAL(buffer.body_filled_size(), 0);
  data_buffer_streambuf sbuff2(&buffer);
  ostream out2(&sbuff2);
  out2 << "tt";
  sbuff2.finalize();
  body = reinterpret_cast<char*>(buffer.body_begin());
  BOOST_CHECK_EQUAL(body, "tt");
  BOOST_CHECK_EQUAL(buffer.body_filled_size(), 2);
  BOOST_CHECK_EQUAL(buffer.buffer_filled_size(), 10);
}

BOOST_AUTO_TEST_CASE(data_buffer_init_tests)
{
  data_buffer db(1);
  BOOST_CHECK_EQUAL(db.preamble_size(), 8);
  BOOST_CHECK_EQUAL(db.body_capacity(), 1);
  BOOST_CHECK_EQUAL(db.body_filled_size(), 0);
  BOOST_CHECK_EQUAL(db.buffer_filled_size(), 8);
  BOOST_CHECK_EQUAL(db.get_body_beginoffset(), 8);
  BOOST_CHECK_EQUAL(db.get_body_endoffset(), 8);
  BOOST_CHECK_EQUAL(db.raw_begin(), db.preamble_begin());
  BOOST_CHECK_EQUAL(db.raw_begin() + 8, db.body_begin());
}

BOOST_AUTO_TEST_CASE(data_buffer_front_fill)
{
  data_buffer db(10);
  BOOST_CHECK_EQUAL(reinforcement_learning::error_code::success, db.set_body_endoffset(18));
  BOOST_CHECK_EQUAL(db.preamble_size(), 8);
  BOOST_CHECK_EQUAL(db.body_capacity(), 10);
  BOOST_CHECK_EQUAL(db.buffer_filled_size(), 18);
  BOOST_CHECK_EQUAL(db.get_body_beginoffset(), 8);
  BOOST_CHECK_EQUAL(db.get_body_endoffset(), 18);
  BOOST_CHECK_EQUAL(db.raw_begin(), db.preamble_begin());
  BOOST_CHECK_EQUAL(db.raw_begin() + 8, db.body_begin());
}

BOOST_AUTO_TEST_CASE(data_buffer_back_fill)
{
  data_buffer db(10);
  db.set_body_beginoffset(14);
  db.set_body_endoffset(18);
  BOOST_CHECK_EQUAL(db.preamble_size(), 8);
  BOOST_CHECK_EQUAL(db.body_capacity(), 10);
  BOOST_CHECK_EQUAL(db.buffer_filled_size(), 12);
  BOOST_CHECK_EQUAL(db.get_body_beginoffset(), 14);
  BOOST_CHECK_EQUAL(db.get_body_endoffset(), 18);
  BOOST_CHECK_EQUAL(db.raw_begin() + 6, db.preamble_begin());
  BOOST_CHECK_EQUAL(db.raw_begin() + 14, db.body_begin());
}