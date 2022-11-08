#pragma once

#include <chrono>
#include <cstdint>
#include <tuple>
#include <iostream>

namespace reinforcement_learning
{
struct timestamp
{
  uint16_t year = 0;        // year
  uint8_t month = 0;        // month [1-12]
  uint8_t day = 0;          // day [1-31]
  uint8_t hour = 0;         // hour [0-23]
  uint8_t minute = 0;       // minute [0-60]
  uint8_t second = 0;       // second [0-60]
  uint32_t sub_second = 0;  // 0.1 u_second [0 - 9,999,999]
  friend std::ostream& operator<<(std::ostream& os, const timestamp& dt);
};

timestamp timestamp_from_chrono(const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>&);
std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> chrono_from_timestamp(const timestamp&);

inline bool operator==(const timestamp& lhs, const timestamp& rhs)
{
  return std::tie(lhs.year, lhs.month, lhs.day, lhs.hour, lhs.minute, lhs.second, lhs.sub_second) ==
      std::tie(rhs.year, rhs.month, rhs.day, rhs.hour, rhs.minute, rhs.second, rhs.sub_second);
}
inline bool operator!=(const timestamp& lhs, const timestamp& rhs) { return !(lhs == rhs); }

inline bool operator<(const timestamp& lhs, const timestamp& rhs)
{
  return std::tie(lhs.year, lhs.month, lhs.day, lhs.hour, lhs.minute, lhs.second, lhs.sub_second) <
      std::tie(rhs.year, rhs.month, rhs.day, rhs.hour, rhs.minute, rhs.second, rhs.sub_second);
}

inline bool operator<=(const timestamp& lhs, const timestamp& rhs) { return (lhs < rhs) || (lhs == rhs); }

inline bool operator>(const timestamp& lhs, const timestamp& rhs) { return !(lhs <= rhs); }

inline bool operator>=(const timestamp& lhs, const timestamp& rhs) { return !(lhs < rhs); }

class i_time_provider
{
public:
  virtual ~i_time_provider() = default;
  virtual timestamp gmt_now() = 0;
};

class clock_time_provider : public i_time_provider
{
public:
  timestamp gmt_now() override;
};
}  // namespace reinforcement_learning
