#include "config_helper.h"
#include "constants.h"

#include <cstring>
#include <sstream>

#ifndef _WIN32
#define _stricmp strcasecmp
#endif


namespace reinforcement_learning
{
  queue_mode_enum to_queue_mode_enum(const char *queue_mode) {
    if (_stricmp(queue_mode, "BLOCK") == 0) {
      return queue_mode_enum::BLOCK;
    } else {
      return queue_mode_enum::DROP;
    }
  }

namespace utility {

static int get_int(const configuration &config, const char *section, const char *property, int defval)
{

  std::stringstream ss;
  ss << section << "." << property;
  auto tmp = ss.str();
  const char *key = tmp.c_str();
  if(config.get(key, NULL) != nullptr) {
    return config.get_int(key, defval);
  }
  return config.get_int(property, defval);
}


static const char* get_str(const configuration &config, const char *section, const char *property, const char* defval)
{
  std::stringstream ss;
  ss << section << "." << property;
  auto tmp  = ss.str();
  const char *key = tmp.c_str();
  if(config.get(key, NULL) != nullptr) {
    return config.get(key, defval);
  }
  return config.get(property, defval);
}

async_batcher_config get_batcher_config(const configuration &config, const char *section)
{
  async_batcher_config res;
  res.send_high_water_mark = get_int(config, section, name::SEND_HIGH_WATER_MARK, 198 * 1024);
  res.send_batch_interval_ms = get_int(config, section, name::SEND_BATCH_INTERVAL_MS, 1000);
  res.send_queue_max_capacity = get_int(config, section, name::SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024;
  res.queue_mode = to_queue_mode_enum(get_str(config, section, name::QUEUE_MODE, value::QUEUE_MODE_DROP));
  res.batch_content_encoding = config.get_bool(section, name::USE_DEDUP, false) ? value::CONTENT_ENCODING_DEDUP : value::CONTENT_ENCODING_IDENTITY;
  return res;
}

async_batcher_config::async_batcher_config():
  send_high_water_mark(198 * 1024),
  send_batch_interval_ms(1000),
  send_queue_max_capacity(16 * 1024 * 1024),
  queue_mode(queue_mode_enum::DROP) {}

}}
