#include "time_helper.h"

#include "date.h"

#include <chrono>
#include <ctime>

namespace reinforcement_learning
{

std::ostream& operator<<(std::ostream& os, const timestamp& dt)
{
  os << +dt.year << "-" << +dt.month << "-" << +dt.day << " " << +dt.hour << ":" << +dt.minute << ":" << +dt.second
     << "." << +dt.sub_second;
  return os;
}

timestamp timestamp_from_chrono(const std::chrono::system_clock::time_point& tp)
{
  timestamp ts;
  const auto dp = date::floor<date::days>(tp);
  const auto ymd = date::year_month_day(dp);
  const auto time = date::make_time(tp - dp);
  ts.year = int(ymd.year());
  ts.month = unsigned(ymd.month());
  ts.day = unsigned(ymd.day());
  ts.hour = time.hours().count();
  ts.minute = time.minutes().count();
  ts.second = static_cast<uint8_t>(time.seconds().count());
  ts.sub_second = static_cast<uint32_t>(time.subseconds().count());
  return ts;
}

std::chrono::system_clock::time_point chrono_from_timestamp(const timestamp& ts)
{
  return date::sys_days{date::year{ts.year} / ts.month / ts.day} + std::chrono::hours{ts.hour} +
      std::chrono::minutes{ts.minute} + std::chrono::seconds{ts.second} + std::chrono::microseconds{ts.sub_second};
}

timestamp clock_time_provider::gmt_now() { return timestamp_from_chrono(std::chrono::system_clock::now()); }
}  // namespace reinforcement_learning