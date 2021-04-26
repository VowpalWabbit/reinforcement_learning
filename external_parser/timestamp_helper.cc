#include "timestamp_helper.h"

rl::timestamp to_timestamp(const v2::TimeStamp &ts) {
  rl::timestamp t;
  t.year = ts.year();
  t.month = ts.month();
  t.day = ts.day();
  t.hour = ts.hour();
  t.minute = ts.minute();
  t.second = ts.second();
  t.sub_second = ts.subsecond();
  return t;
}

rl::timestamp max_timestamp() {
  rl::timestamp t;
  t.year = std::numeric_limits<uint16_t>::max();
  t.month = std::numeric_limits<uint8_t>::max();
  t.day = std::numeric_limits<uint8_t>::max();
  t.hour = std::numeric_limits<uint8_t>::max();
  t.minute = std::numeric_limits<uint8_t>::max();
  t.second = std::numeric_limits<uint8_t>::max();
  t.sub_second = std::numeric_limits<uint32_t>::max();
  return t;
}

bool first_smaller_than_second(const rl::timestamp &a, const rl::timestamp &b) {
  // clang-format off
  return a.year < b.year ? true
    : (a.year > b.year ? false
      : (a.month < b.month ? true
        : (a.month > b.month ? false
          : (a.day < b.day ? true
            : (a.day > b.day ? false
              : (a.hour < b.hour ? true
                : (a.hour > b.hour ? false
                  : (a.minute < b.minute ? true
                    : (a.minute > b.minute ? false
                      : (a.second < b.second ? true
                        : (a.second > b.second ? false
                          : (a.sub_second < b.sub_second))))))))))));
  // clang-format on
}