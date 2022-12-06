#include "time_helper.h"

#include "date.h"

#include <chrono>
#include <ctime>
#include <ratio>

namespace reinforcement_learning
{
std::ostream& operator<<(std::ostream& os, const timestamp& dt)
{
  os << +dt.year << "-" << +dt.month << "-" << +dt.day << " " << +dt.hour << ":" << +dt.minute << ":" << +dt.second
     << "." << +dt.sub_second;
  return os;
}

timestamp::timestamp(uint16_t yr, uint8_t mo, uint8_t dy, uint8_t h, uint8_t m, uint8_t s, uint32_t ss)
    : year(yr), month(mo), day(dy), hour(h), minute(m), second(s), sub_second(ss)
{
}

timestamp::timestamp(const std::chrono::time_point<std::chrono::system_clock, timestamp::one_hundred_nanoseconds>& tp)
{
  const auto dp = date::floor<date::days>(tp);
  const auto ymd = date::year_month_day(dp);
  const auto duration_since_start_of_day = tp - dp;
  const auto time = date::make_time(duration_since_start_of_day);
  auto usec_since_start_of_day =
      std::chrono::duration_cast<timestamp::one_hundred_nanoseconds>(duration_since_start_of_day);

  year = int(ymd.year());
  month = unsigned(ymd.month());
  day = unsigned(ymd.day());
  hour = time.hours().count();
  minute = time.minutes().count();
  second = static_cast<uint8_t>(time.seconds().count());
  sub_second = static_cast<uint32_t>(usec_since_start_of_day.count() % timestamp::one_hundred_nano::den);
}

timestamp::timestamp(const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>& tp)
    : timestamp(std::chrono::time_point_cast<timestamp::one_hundred_nanoseconds>(tp))
{
}

std::chrono::time_point<std::chrono::system_clock, timestamp::one_hundred_nanoseconds> timestamp::to_time_point() const
{
  return date::sys_days{date::year{year} / month / day} + std::chrono::hours{hour} + std::chrono::minutes{minute} +
      std::chrono::seconds{second} + timestamp::one_hundred_nanoseconds{sub_second};
}

timestamp clock_time_provider::gmt_now() { return timestamp(std::chrono::system_clock::now()); }
}  // namespace reinforcement_learning
