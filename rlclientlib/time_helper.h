#pragma once
#include <cstdint>
namespace reinforcement_learning {

  struct timestamp {
    uint16_t year = 0;    // year 
    uint8_t month = 0;    // month [1-12]
    uint8_t day = 0;      // day [1-31]
    uint8_t hour = 0;     // hour [0-23]
	  uint8_t minute = 0;   // minute [0-60]
	  uint8_t second = 0;   // second [0-60]
	  uint32_t sub_second = 0; // 0.1 u_second [0 - 9,999,999]
  };

  class i_time_provider {
  public:
    virtual ~i_time_provider() = default;
    virtual timestamp gmt_now() = 0;
  };

  class clock_time_provider : public i_time_provider {
  public:
    timestamp gmt_now() override;
  };
}
