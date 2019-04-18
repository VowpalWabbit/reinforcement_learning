#include "time_helper.h"
#include "date.h"
namespace reinforcement_learning
{
  timestamp clock_time_provider::gmt_now() {
    timestamp ts;
    const auto tp = std::chrono::system_clock::now();
    const auto dp = date::floor<date::days>(tp);
    const auto ymd = date::year_month_day(dp);
    const auto time = date::make_time(tp-dp);
    ts.year = int(ymd.year());
    ts.month = unsigned(ymd.month());
    ts.day = unsigned(ymd.day());
    ts.hour = time.hours().count();
	  ts.minute = time.minutes().count();
	  ts.second = time.seconds().count();
    ts.sub_second = time.subseconds().count();
    return ts;
  }
}