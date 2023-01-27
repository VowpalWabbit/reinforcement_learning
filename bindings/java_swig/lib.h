#include <memory>

namespace reinforcement_learning
{
    namespace utility
    {
        class configuration;
    }
}

namespace rlswig
{
struct configuration
{
  configuration();
  ~configuration();
  void set(const char* name, const char* value);
  const char* get(const char* name, const char* defval) const;
  int get_int(const char* name, int defval) const;
  bool get_bool(const char* name, bool defval) const;
  float get_float(const char* name, float defval) const;
  bool get_bool(const char* section, const char* name, bool defval) const;

private:
    std::unique_ptr<reinforcement_learning::utility::configuration> _config;
};
}