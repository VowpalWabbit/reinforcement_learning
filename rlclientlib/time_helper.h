#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <tuple>

namespace reinforcement_learning
{
struct timestamp
{
  using one_hundred_nano = std::ratio<1, 10000000>;
  using one_hundred_nanoseconds = std::chrono::duration<uint64_t, one_hundred_nano>;

  uint16_t year = 0;        // year
  uint8_t month = 0;        // month [1-12]
  uint8_t day = 0;          // day [1-31]
  uint8_t hour = 0;         // hour [0-23]
  uint8_t minute = 0;       // minute [0-60]
  uint8_t second = 0;       // second [0-60]
  uint32_t sub_second = 0;  // 0.1 u_second [0 - 9,999,999]

  // Construct timestamp with all zero values
  timestamp() = default;

  // Construct timestamp from values for each time component
  timestamp(uint16_t yr, uint8_t mo, uint8_t dy, uint8_t h, uint8_t m, uint8_t s, uint32_t ss = 0);

  // Convert std::chrono::time_point to reinforcement_learning::timestamp
  explicit timestamp(const std::chrono::time_point<std::chrono::system_clock, timestamp::one_hundred_nanoseconds>&);

  // Overload that takes std::chrono::nanoseconds as returned by system_clock::now() and typecasts it to 100ns
  explicit timestamp(const std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>&);

  // Convert to a std::chrono::time_point with 100ns resolution
  std::chrono::time_point<std::chrono::system_clock, timestamp::one_hundred_nanoseconds> to_time_point() const;

  friend std::ostream& operator<<(std::ostream& os, const timestamp& dt);
};

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
