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

static const char* get_default_sender_implmentation(const char* section)
{
  if (section == OBSERVATION_SECTION) {
      return value::get_default_observation_sender();
  }
  return value::get_default_interaction_sender();
}

static float get_float(const configuration &config, const char *section, const char *property, float defval)
{

  std::stringstream ss;
  ss << section << "." << property;
  auto tmp = ss.str();
  const char *key = tmp.c_str();
  if(config.get(key, NULL) != nullptr) {
    return config.get_float(key, defval);
  }
  return config.get_float(property, defval);
}

events_counter_status get_counter_status(const utility::configuration& config, const char* section)
{
  int version = config.get_int(name::PROTOCOL_VERSION, value::DEFAULT_PROTOCOL_VERSION);
  const char* sender_implmentation = get_str(config, section, name::SENDER_IMPLEMENTATION, get_default_sender_implmentation(section));
  //In future validation can be extended based on requirement
  if (version == 2 && _stricmp(sender_implmentation, value::INTERACTION_HTTP_API_SENDER) == 0) {
    //only when protocol version is 2 and sender implmentation is INTERACTION_HTTP_API_HOST
    return events_counter_status::ENABLE;
  }
  else {
    return events_counter_status::DISABLE;
  }
}

async_batcher_config get_batcher_config(const configuration &config, const char *section)
{
  async_batcher_config res;
  res.send_high_water_mark = get_int(config, section, name::SEND_HIGH_WATER_MARK, 198 * 1024);
  res.send_batch_interval_ms = get_int(config, section, name::SEND_BATCH_INTERVAL_MS, 1000);
  res.send_queue_max_capacity = get_int(config, section, name::SEND_QUEUE_MAX_CAPACITY_KB, 16 * 1024) * 1024;
  res.queue_mode = to_queue_mode_enum(get_str(config, section, name::QUEUE_MODE, value::QUEUE_MODE_DROP));
  res.batch_content_encoding = config.get_bool(section, name::USE_DEDUP, false) ? value::CONTENT_ENCODING_DEDUP : value::CONTENT_ENCODING_IDENTITY;
  res.subsample_rate = get_float(config, section, name::SUBSAMPLE_RATE, 1.f);
  res.events_counter_status = get_counter_status(config,section);
  return res;
}

async_batcher_config::async_batcher_config():
  send_high_water_mark(198 * 1024),
  send_batch_interval_ms(1000),
  send_queue_max_capacity(16 * 1024 * 1024),
  queue_mode(queue_mode_enum::DROP),
  events_counter_status(events_counter_status::DISABLE){}

}}
