#include "time_helper.h"

#include "date.h"

#include <chrono>
#include <ctime>
#include <ratio>

namespace
{
constexpr uint64_t ONE_HUNDRED_NANO_DENOMINATOR = 10000000;
using one_hundred_nano = std::ratio<1, ONE_HUNDRED_NANO_DENOMINATOR>;
using one_hundred_nanoseconds = std::chrono::duration<uint64_t, one_hundred_nano>;
}  // namespace

namespace reinforcement_learning
{
std::ostream& operator<<(std::ostream& os, const timestamp& dt)
{
  os << +dt.year << "-" << +dt.month << "-" << +dt.day << " " << +dt.hour << ":" << +dt.minute << ":" << +dt.second
     << "." << +dt.sub_second;
  return os;
}

timestamp timestamp_from_chrono(const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& tp)
{
  timestamp ts;
  const auto dp = date::floor<date::days>(tp);
  const auto ymd = date::year_month_day(dp);
  const auto duration_since_start_of_day = tp - dp;
  const auto time = date::make_time(duration_since_start_of_day);
  ts.year = int(ymd.year());
  ts.month = unsigned(ymd.month());
  ts.day = unsigned(ymd.day());
  ts.hour = time.hours().count();
  ts.minute = time.minutes().count();
  ts.second = static_cast<uint8_t>(time.seconds().count());
  std::chrono::duration<uint64_t, one_hundred_nano> usec_since_start_of_day =
      std::chrono::duration_cast<one_hundred_nanoseconds>(duration_since_start_of_day);
  ts.sub_second = static_cast<uint32_t>(usec_since_start_of_day.count() % ONE_HUNDRED_NANO_DENOMINATOR);
  return ts;
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> chrono_from_timestamp(const timestamp& ts)
{
  return date::sys_days{date::year{ts.year} / ts.month / ts.day} + std::chrono::hours{ts.hour} +
      std::chrono::minutes{ts.minute} + std::chrono::seconds{ts.second} + std::chrono::nanoseconds{ts.sub_second * 100};
}

timestamp clock_time_provider::gmt_now() { return timestamp_from_chrono(std::chrono::system_clock::now()); }
}  // namespace reinforcement_learning
