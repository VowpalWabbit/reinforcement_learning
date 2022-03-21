#include "timestamp_helper.h"
#include "io/logger.h"

TimePoint timestamp_to_chrono(const v2::TimeStamp &ts) {

  // --- date transformation ---
  // date::year(ts.year())/date::month(ts.month())/date::day(ts.day()) ->
  // date::year_month_day type used to initialize date::sys_days which is a
  // chrono::time_point expressed in days
  // date::sys_days can have hours/mins/seconds added to it and the
  // result will be a chrono::time_point expressed in seconds
  // Explicitly truncate subseconds (= 100 ns)
  // expecting UTC, no leap second support
  return date::sys_days(date::year(ts.year()) / date::month(ts.month()) /
                        date::day(ts.day())) +
         std::chrono::hours(ts.hour()) + std::chrono::minutes(ts.minute()) +
         std::chrono::seconds(ts.second());
}

bool is_empty_timestamp(const v2::TimeStamp &ts) {
  return (ts.year() == 0 && ts.month() == 0 && ts.day() == 0 &&
          ts.hour() == 0 && ts.minute() == 0 && ts.second() == 0);
}

TimePoint get_enqueued_time(const v2::TimeStamp *enqueued_time_utc,
                            const v2::TimeStamp *client_time_utc,
                            bool use_client_time, VW::io::logger& logger) {
  if (use_client_time) {
    if (!client_time_utc || is_empty_timestamp(*client_time_utc)) {
      logger.out_warn(
          "binary parser is configured to use client-provided EnqueuedTimeUTC, "
          "but input metadata does not contain a client timestamp.");
      return timestamp_to_chrono(*enqueued_time_utc);
    }
    return timestamp_to_chrono(*client_time_utc);
  }
  return timestamp_to_chrono(*enqueued_time_utc);
}