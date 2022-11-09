#define BOOST_TEST_DYN_LINK
#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

#include "date.h"
#include "time_helper.h"

#include <iostream>
namespace r = reinforcement_learning;

BOOST_AUTO_TEST_CASE(time_usage)
{
  r::clock_time_provider ctp;
  const uint16_t NUM_ITER = 1000;
  for (int i = 0; i < NUM_ITER; ++i)
  {
    const auto ts = ctp.gmt_now();
    BOOST_CHECK(ts.year > 0);
    BOOST_CHECK(ts.month <= 12);
    BOOST_CHECK(ts.day <= 31);
    BOOST_CHECK(ts.hour <= 23);
    BOOST_CHECK(ts.minute <= 59);
    BOOST_CHECK(ts.second <= 60);

    // subsecond is dependent on resolution of system clock
    // BOOST_CHECK(ts.sub_second <= 9999999);
  }
}

BOOST_AUTO_TEST_CASE(time_round_trip)
{
  r::clock_time_provider ctp;
  auto now = ctp.gmt_now();
  auto roundtripped = r::timestamp_from_chrono(r::chrono_from_timestamp(now));
  BOOST_CHECK_EQUAL(now, roundtripped);
}

BOOST_AUTO_TEST_CASE(time_ordering)
{
  r::clock_time_provider ctp;
  auto now = ctp.gmt_now();
  BOOST_CHECK(r::timestamp_from_chrono(std::chrono::system_clock::now() - std::chrono::seconds(5)) < now);
  BOOST_CHECK(r::timestamp_from_chrono(std::chrono::system_clock::now() - std::chrono::minutes(5)) < now);
  BOOST_CHECK(r::timestamp_from_chrono(std::chrono::system_clock::now() - std::chrono::hours(5)) < now);
  BOOST_CHECK(r::timestamp_from_chrono(std::chrono::system_clock::now() - std::chrono::hours(500)) < now);
  BOOST_CHECK(r::timestamp_from_chrono(std::chrono::system_clock::now() - date::days(1)) < now);
  BOOST_CHECK(r::timestamp_from_chrono(std::chrono::system_clock::now() - date::months(1)) < now);
  BOOST_CHECK(r::timestamp_from_chrono(std::chrono::system_clock::now() - date::years(1)) < now);
}

// BOOST_AUTO_TEST_CASE(time_loop) {
//	r::clock_time_provider ctp;
//	const uint16_t NUM_ITER = 1000;
//	std::vector<reinforcement_learning::timestamp> ts_coll;
//	for (int i = 0; i < NUM_ITER; ++i) {
//		ts_coll.emplace_back(ctp.gmt_now());
//	}
//	for (auto && ts: ts_coll) {
//		auto usec = ts.u_second % 1000;
//		auto msec = ((ts.u_second - usec) % 1000000)/1000;
//		auto total_sec = (ts.u_second - 1000 * msec - usec)/1000000;
//		auto sec = total_sec % 60;
//		auto min = (total_sec - sec)/60;
//		std::cout << ts.month << "/" << ts.day << "/" << ts.year+1900 << " " << ts.hour << ":" << min << ":" <<
// sec << "." << msec << "." << usec << std::endl;
//	}
// }
