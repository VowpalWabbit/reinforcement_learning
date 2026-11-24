#include "person.h"

#include "rand48.h"

#include <sstream>
#include <utility>

person::person(std::string id, std::string major, std::string hobby, std::string fav_char, topic_prob& p)
    : _id(std::move(id))
    , _major{std::move(major)}
    , _hobby{std::move(hobby)}
    , _favorite_character{std::move(fav_char)}
    , _topic_click_probability{p}
{
}

person::~person() = default;

std::string person::get_features() const
{
  std::ostringstream oss;
  oss << R"("GUser":{)";
  oss << R"("id":")" << _id << R"(",)";
  oss << R"("major":")" << _major << R"(",)";
  oss << R"("hobby":")" << _hobby << R"(",)";
  oss << R"("favorite_character":")" << _favorite_character;
  oss << R"("})";
  return oss.str();
}

float person::get_outcome(const std::string& chosen_action, uint64_t& random_seed)
{
  float const norm_draw_val = rand48(random_seed);
  float const click_prob = _topic_click_probability[chosen_action];
  if (norm_draw_val <= click_prob) { return 1.0f; }
  return 0.0f;
}

std::string person::id() const { return _id; }
