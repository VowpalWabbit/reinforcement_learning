#include <boost/test/unit_test.hpp>

#include "timestamp_helper.h"

#include <thread>

// gmt timestamp generation as used in rlclientlib/time_helper.cc
// copied from there (hasn't changed since it was checked in)
// since we need the internals to check that time transformations are done
// correctly
std::pair<v2::TimeStamp, std::chrono::time_point<std::chrono::system_clock>>
gmt_now_and_timestamp() {
  const auto tp = std::chrono::system_clock::now();
  const auto dp = date::floor<date::days>(tp);
  const auto ymd = date::year_month_day(dp);
  const auto time = date::make_time(tp - dp);

  return std::make_pair(
      v2::TimeStamp(int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day()),
                    time.hours().count(), time.minutes().count(),
                    time.seconds().count(), time.subseconds().count()),
      tp);
}

BOOST_AUTO_TEST_CASE(test_exact_timestamp) {
  auto times = gmt_now_and_timestamp();
  auto fb_ts = std::get<0>(times);
  auto initial_chrono_ts = std::get<1>(times);

  auto back_to_chrono = timestamp_to_chrono(fb_ts);

  BOOST_CHECK_EQUAL(back_to_chrono.time_since_epoch().count(),
                    initial_chrono_ts.time_since_epoch().count());

  // check time comparison operators
  BOOST_CHECK_EQUAL(back_to_chrono == initial_chrono_ts, true);
  BOOST_CHECK_EQUAL(back_to_chrono <= initial_chrono_ts, true);
  BOOST_CHECK_EQUAL(back_to_chrono >= initial_chrono_ts, true);
  BOOST_CHECK_EQUAL(back_to_chrono < initial_chrono_ts, false);
  BOOST_CHECK_EQUAL(back_to_chrono > initial_chrono_ts, false);
}

BOOST_AUTO_TEST_CASE(test_later_than_timestamp) {
  const auto earlier_timestamp = std::chrono::system_clock::now();
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  auto times = gmt_now_and_timestamp();
  auto later_fb_timestamp = std::get<0>(times);

  auto later_timestamp = timestamp_to_chrono(later_fb_timestamp);

  BOOST_CHECK_NE(earlier_timestamp.time_since_epoch().count(),
                 later_timestamp.time_since_epoch().count());

  // check time comparison operators
  BOOST_CHECK_EQUAL(earlier_timestamp < later_timestamp, true);
  BOOST_CHECK_EQUAL(earlier_timestamp <= later_timestamp, true);
  BOOST_CHECK_EQUAL(earlier_timestamp == later_timestamp, false);
  BOOST_CHECK_EQUAL(earlier_timestamp >= later_timestamp, false);
  BOOST_CHECK_EQUAL(earlier_timestamp > later_timestamp, false);
}

BOOST_AUTO_TEST_CASE(test_earlier_than_timestamp) {
  auto times = gmt_now_and_timestamp();
  std::this_thread::sleep_for(std::chrono::milliseconds(2));
  auto earlier_fb_timestamp = std::get<0>(times);
  auto earlier_timestamp = timestamp_to_chrono(earlier_fb_timestamp);

  const auto later_timestamp = std::chrono::system_clock::now();

  BOOST_CHECK_NE(earlier_timestamp.time_since_epoch().count(),
                 later_timestamp.time_since_epoch().count());

  // check time comparison operators
  BOOST_CHECK_EQUAL(earlier_timestamp < later_timestamp, true);
  BOOST_CHECK_EQUAL(earlier_timestamp <= later_timestamp, true);
  BOOST_CHECK_EQUAL(earlier_timestamp == later_timestamp, false);
  BOOST_CHECK_EQUAL(earlier_timestamp >= later_timestamp, false);
  BOOST_CHECK_EQUAL(earlier_timestamp > later_timestamp, false);
}