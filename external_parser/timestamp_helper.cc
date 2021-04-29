#include "timestamp_helper.h"

std::chrono::time_point<std::chrono::system_clock>
timestamp_to_chrono(const v2::TimeStamp &ts) {

  // --- date transformation ---
  // date::year(ts.year())/date::month(ts.month())/date::day(ts.day()) ->
  // date::year_month_day type used to initialize date::sys_days which is a
  // chrono::time_point expressed in days
  // date::sys_days can have hours/mins/seconds/nanosecond added to it and the
  // result will be a chrono::time_point expressed in nanoseconds
  return std::chrono::time_point<std::chrono::system_clock>(
      date::sys_days(date::year(ts.year()) / date::month(ts.month()) /
                     date::day(ts.day())) +
      std::chrono::hours(ts.hour()) + std::chrono::minutes(ts.minute()) +
      std::chrono::seconds(ts.second()) +
      std::chrono::nanoseconds(ts.subsecond()));
}