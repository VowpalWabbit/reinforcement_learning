#ifdef STAND_ALONE
#  define BOOST_TEST_MODULE Main
#endif

#include <boost/test/unit_test.hpp>

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
