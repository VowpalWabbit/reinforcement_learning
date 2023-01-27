#include "lib.h"

#include "configuration.h"

rlswig::configuration::configuration()
{
  _config = std::unique_ptr<reinforcement_learning::utility::configuration>(new reinforcement_learning::utility::configuration());
}
rlswig::configuration::~configuration() = default;
void rlswig::configuration::set(const char* name, const char* value)
{
  _config->set(name, value);
}
const char* rlswig::configuration::get(const char* name, const char* defval) const
{
  return _config->get(name, defval);
}
int rlswig::configuration::get_int(const char* name, int defval) const
{
  return _config->get_int(name, defval);
}
bool rlswig::configuration::get_bool(const char* name, bool defval) const
{
  return _config->get_bool(name, defval);
}
float rlswig::configuration::get_float(const char* name, float defval) const
{
  return _config->get_float(name, defval);
}
bool rlswig::configuration::get_bool(const char* section, const char* name, bool defval) const
{
  return _config->get_bool(section, name, defval);
}